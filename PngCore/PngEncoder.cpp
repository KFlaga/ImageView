
#include "PngImage.h"
#include "FileStream.h"
#include "zlib\zlib.h"
#include "Exceptions.h"
#include "PngFilter.h"

namespace ImgOps
{
	const byte PNGHeaderBytes[8] = {
		137, 80, 78, 71, 13, 10, 26, 10
	};

	ImageEncoder* CreatePNGEncoder()
	{
		return new PNGImageEncoder();
	}

	PNGImageEncoder::PNGImageEncoder()
	{
		_image = NULL;
		_saveInterlaced = false;
	}

	PNGImageEncoder::~PNGImageEncoder()
	{
		FreeMemory();
	}

	void PNGImageEncoder::SetImage(Image* image)
	{
		_image = image;
	}

	void PNGImageEncoder::FreeMemory()
	{

	}

	bool PNGImageEncoder::SaveImageToFile(const char* filePath, Image* image)
	{
		FileStream file(filePath, OpenModes::WriteTrunc);
		if(file.IsOpen())
		{
			return SaveImageToFile(&file, image);
		}
		else
		{
			return false;
		}
	}

	bool PNGImageEncoder::SaveImageToFile(FileStream* file, Image* image)
	{
		try
		{
			SetImage(image);
			SaveImageToFile_Internal(file);
		}
		catch(Exception e)
		{
			FreeMemory();
			return false;
		}
		return true;
	}

	void PNGImageEncoder::SaveImageToFile_Internal(FileStream* file)
	{
		// First store png header
		int headerSize = 8;
		memcpy(_chunkBuf, PNGHeaderBytes, headerSize);
		int64 writeBytes = file->WriteSome(headerSize, _chunkBuf);

		if(writeBytes != headerSize)
		{
			ReportError("Failed to store image header");
		}

		StoreChunk_IHDR(file);
		if(_image->GetPalettesCount() > 0)
			StoreChunk_PLTE(file);
		if(_image->GetDecryptedFormat() != PixelFormats::Unknown)
			StoreChunk_deCf(file);
		StoreChunk_IDAT(file);
		StoreChunk_IEND(file);
	}

	void PNGImageEncoder::ReportError(const char* error)
	{
		throw EncoderException(error);
	}

#pragma region STORE_CHUNK

	void PNGImageEncoder::StoreChunk_IHDR(FileStream* file)
	{
		// 1) Store chunk length and type in buffer
		uint32 length = 13;
		uint32 ihdrBytes = IHDR_Bytes;
		Uint32ToByte4(length, _chunkBuf);
		Uint32ToByte4(ihdrBytes, _chunkBuf + 4);

		uint32 bufOffset = 8;

		// Contents:
		// 4bytes[0] : width 
		// 4bytes[4] : height
		// 1byte[8]  : bit depth
		// 1byte[9]  : color type
		// 1byte[10] : compression method
		// 1byte[11] : filter method
		// 1byte[12] : interlace method

		// Check validity
		if(_image->Width() == 0 || _image->Height() == 0)
			ReportError("Zero width/height not supported");

		Uint32ToByte4(_image->Width(), _chunkBuf + bufOffset);
		bufOffset += 4;

		Uint32ToByte4(_image->Height(), _chunkBuf + bufOffset);
		bufOffset += 4;

		byte bitdepth = _image->ChannelSize() * 8;
		_chunkBuf[bufOffset] = bitdepth;
		bufOffset += 1;

		byte colorType = 0;
		if(_image->PixFormat() == PixelFormats::Indexed)
		{
			colorType = ColorModes::Palette;
		}
		else
		{
			if((_image->PixFormat() & PixelFormats::TrueColor) != 0)
			{
				colorType = ColorModes::RGB;
			}
			if((_image->PixFormat() & PixelFormats::HaveAlphaChannel) != 0)
			{
				colorType |= 4;
			}
		}
		_chunkBuf[bufOffset] = colorType;
		bufOffset += 1;

		_chunkBuf[bufOffset] = 0; // Compression method
		_chunkBuf[bufOffset + 1] = 0; // Filter method
		_chunkBuf[bufOffset + 2] = _saveInterlaced ? InterlaceMethods::Adam7 : InterlaceMethods::None;
		bufOffset += 3;

		// Compute CRC from buffer
		CRC_Init();
		CRC_Add(_chunkBuf + 4, bufOffset - 4);
		uint32 crc = CRC_Finish();

		Uint32ToByte4(crc, _chunkBuf + bufOffset);
		bufOffset += 4;

		int64 writeBytes = file->WriteSome(bufOffset, _chunkBuf);
		if(writeBytes != bufOffset)
		{
			ReportError("Failed to store IHDR");
		}
	}

	void PNGImageEncoder::StoreChunk_IEND(FileStream* file)
	{
		// 1) Store chunk length and type in buffer
		uint32 length = 0;
		uint32 iendBytes = IEND_Bytes;
		Uint32ToByte4(length, _chunkBuf);
		Uint32ToByte4(iendBytes, _chunkBuf + 4);

		// Compute CRC from buffer
		CRC_Init();
		CRC_Add(_chunkBuf + 4, 4);
		uint32 crc = CRC_Finish();

		Uint32ToByte4(crc, _chunkBuf + 8);

		int64 writeBytes = file->WriteSome(12, _chunkBuf);
		if(writeBytes != 12)
		{
			ReportError("Failed to store IEND");
		}
	}

	void PNGImageEncoder::StoreChunk_IDAT(FileStream* file)
	{
		// Init zlib
		z_stream zlib = z_stream();
		zlib.zalloc = Z_NULL;
		zlib.zfree = Z_NULL;
		zlib.opaque = Z_NULL;
		// TODO: if image byte size is less than 2^15 set smaller window

		int retVal = deflateInit2(&zlib, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15, 8, Z_RLE); // TODO: add compression level dialog
		if(retVal != Z_OK)
		{
			ReportError("Zlib failed to initialize");
		}

		// Check how much rows will fit to image buffer (+ one byte each row for filter type)
		uint32 rowsInChunk = ImageBufferSize / (1 + _image->Stride());
		uint32 totalRows = 0; // Count of processed rows
		uint32 imageBufSize;  // Current amount of available image data
		while(retVal != Z_STREAM_END) // Process all image data
		{
			// Store chunk type in buffer and save space for chunk length
			uint32 length = 0;
			uint32 idatBytes = IDAT_Bytes;
			Uint32ToByte4(idatBytes, _chunkBuf + 4);

			zlib.next_out = _chunkBuf + 8; // Start after chunk type bytes
			zlib.avail_out = ChunkBufferSize - 12; // Save 4 bytes for CRC

			while(zlib.avail_out > 0 && totalRows < _image->Height()) // Until some more data can be deflated and we got more input to provide
			{
				if(zlib.avail_in == 0) // We got no mor data to compress, so filter new rows
				{
					// Store filtered rows in buffer
					uint32 rowsStored = FilterRows(totalRows, rowsInChunk, -1);
					imageBufSize = rowsStored * (1 + _image->Stride());
					totalRows += rowsStored;

					zlib.next_in = _filteredImageBuf;
					zlib.avail_in = imageBufSize;

					if(totalRows == _image->Height())
					{
						// Processed all input
						break;
					}
					if(totalRows > _image->Height())
						ReportError("Processed rows count exceeds image height");
				}
				deflate(&zlib, Z_NO_FLUSH); // We dont need to check retur value assuming 
				// avail_in/avail_out > 0 && next_in/next_out > 0, which is ensured
				// With NO_FLUSH deflate should produce some output after call or only after 
				// enough input to fill whole out buffer is provided -> which is what we want
			}

			// Here we know deflate used all possible output space or there is no more input
			if(totalRows == _image->Height())
			{
				// End stream : we got no more input
				// Deflate will finalize compressing and return Z_STREAM_END - next file-store will be a lat one
				// or it will return Z_OK if not enough space is provided - in next iteration loop will be omitted
				// and deflate called with Z_FINISH until whole compressed stream is saved
				retVal = deflate(&zlib, Z_FINISH);
			}

			length = ChunkBufferSize - 12 - zlib.avail_out; // compressed data length = avail_out_init - avail_out_finish
			// avail_out_finish may be > 0 for last chunk
			Uint32ToByte4(length, _chunkBuf);

			// Compute CRC from type/compressed data
			CRC_Init();
			CRC_Add(_chunkBuf + 4, length + 4);
			uint32 crc = CRC_Finish();
			Uint32ToByte4(crc, _chunkBuf + 8 + length);

			int64 writeBytes = file->WriteSome(length + 12, _chunkBuf);
			if(writeBytes != length + 12)
			{
				ReportError("Failed to store IDAT");
			}
		} // while(retVal != Z_STREAM_END)

		// Here we have processed whole image, so deinit zlib
		retVal = deflateEnd(&zlib);
		switch (retVal) 
		{
		case Z_DATA_ERROR:
			ReportError("Zlib failed to finzalize compression : stream was freed prematurely");
			return;
		case Z_STREAM_ERROR :
			ReportError("Zlib failed to finzalize compression : stream state was inconsistent");
			return;
		}
	}


	uint32 PNGImageEncoder::FilterRows(uint32 startRow, uint32 rowsCount, int filter)
	{
		// - If the image type is Palette, or the bit depth is smaller than 8, 
		// then do not filter the image (i.e. use fixed filtering, with the filter None).
		// - If the image type is Grayscale or RGB (with or without Alpha), 
		// and the bit depth is not smaller than 8, then use adaptive filtering as follows: 
		// independently for each row, apply all five filters and select the filter 
		// that produces the smallest sum of absolute values per row.

		uint32 currentRow = startRow;
		uint32 currentColumn = 0;	
		int currentByte;
		int bufOffset = 0;

		if(filter == -1 && _image->PixFormat() == PixelFormats::Indexed)
		{
			filter = 0;
		}
		byte filterMethod = (byte)filter;
		byte curVal[8], leftVal[8], topVal[8], topLeftVal[8];
		memset(curVal, 0, 8);
		memset(topVal, 0, 8);

		while(currentRow < _image->Height() && currentRow < startRow + rowsCount)
		{
			if(filter == -1)
			{
				// Choose best filter for current row: 
				// apply all five filters and select the filter 
				// that produces the smallest sum of absolute values per row

				int64 filterSums[5]; // Sum of values for each filter
				memset(filterSums, 0, 5 * 8);
				currentColumn = 0;

				memset(leftVal, 0, _image->PixelSize()); // Zero left/top-left for each row -> it will be column -1
				memset(topLeftVal, 0, _image->PixelSize());

				while (currentColumn < _image->Width())
				{
					if(currentRow > 0)
						memcpy(topVal, _image->Pixel(currentRow - 1, currentColumn), _image->PixelSize());
					memcpy(curVal, _image->Pixel(currentRow, currentColumn), _image->PixelSize());

					currentByte = 0;
					while(currentByte < _image->PixelSize())
					{
						// For each channel check value with each filter
						for(int fm = 0; fm < 5; ++fm)
						{
							filterSums[fm] += PngFilter::FilterByte(fm, 
								curVal[currentByte], leftVal[currentByte], 
								topVal[currentByte], topLeftVal[currentByte]);
						}
						++currentByte;
					}

					// We will move right in next iteration, so left/topLeft values will be same as current/top values
					memcpy(leftVal, curVal, _image->PixelSize());
					memcpy(topLeftVal, topVal, _image->PixelSize());
					++currentColumn;
				}

				// We have sums, so choose filter with smallest one
				filterMethod = 0;
				int64 bestSum = filterSums[0];
				for(int i = 1; i < 5; ++i)
				{
					if(bestSum > filterSums[i])
					{
						filterMethod = i;
						bestSum = filterSums[i];
					}
				}
			}

			// We are sure we have proper 'filterMethod' set, so just filter and store image row
			_filteredImageBuf[bufOffset] = filterMethod;
			++bufOffset;
			currentColumn = 0;

			memset(leftVal, 0, _image->PixelSize()); // Zero left/top-left for each row -> it will be column -1
			memset(topLeftVal, 0, _image->PixelSize());

			while (currentColumn < _image->Width())
			{
				if(currentRow > 0)
					memcpy(topVal, _image->Pixel(currentRow - 1, currentColumn), _image->PixelSize());
				memcpy(curVal, _image->Pixel(currentRow, currentColumn), _image->PixelSize());

				currentByte = 0;
				while(currentByte < _image->PixelSize())
				{
					_filteredImageBuf[bufOffset] = PngFilter::FilterByte(filterMethod, 
						curVal[currentByte], leftVal[currentByte], 
						topVal[currentByte], topLeftVal[currentByte]);
					++bufOffset;
					++currentByte;
				}

				// We will move right in next iteration, so left/topLeft values will be same as current/top values
				memcpy(leftVal, curVal, _image->PixelSize());
				memcpy(topLeftVal, topVal, _image->PixelSize());
				++currentColumn;
			}
			++currentRow;
		}
		return currentRow - startRow;
	}

	void PNGImageEncoder::StoreChunk_PLTE(FileStream* file)
	{
		// 1) Store chunk length and type in buffer
		uint32 length = _image->GetPalettesCount() * 3;
		uint32 plteBytes = PLTE_Bytes;
		Uint32ToByte4(length, _chunkBuf);
		Uint32ToByte4(plteBytes, _chunkBuf + 4);
		uint32 bufOffset = 8;

		// Each 3bytes of data contains [r,g,b] colors for palette
		uint32 pltes = _image->GetPalettesCount();
		for(uint32 i = 0; i < pltes; ++i)
		{
			_chunkBuf[bufOffset] = _image->Palette(i)[0];
			_chunkBuf[bufOffset + 1] = _image->Palette(i)[1];
			_chunkBuf[bufOffset + 2] = _image->Palette(i)[2];
			bufOffset += 3;
		}

		// Compute CRC from buffer
		CRC_Init();
		CRC_Add(_chunkBuf + 4, bufOffset - 4);
		uint32 crc = CRC_Finish();

		Uint32ToByte4(crc, _chunkBuf + bufOffset);
		bufOffset += 4;

		int64 writeBytes = file->WriteSome(bufOffset, _chunkBuf);
		if(writeBytes != bufOffset)
		{
			ReportError("Failed to store PLTE");
		}
	}

	void PNGImageEncoder::StoreChunk_deCf(FileStream* file)
	{
		// 1) Store chunk length and type in buffer
		uint32 length = 8;
		uint32 ihdrBytes = deCf_Bytes;
		Uint32ToByte4(length, _chunkBuf);
		Uint32ToByte4(ihdrBytes, _chunkBuf + 4);

		uint32 bufOffset = 8;

		// Contents:
		// 4bytes[0] : pix format
		// 4bytes[4] : decrypted image last chunk size
		
		Uint32ToByte4(_image->GetDecryptedFormat(), _chunkBuf + bufOffset);
		bufOffset += 4;
		Uint32ToByte4(_image->GetDecryptedLastChunkSize(), _chunkBuf + bufOffset);
		bufOffset += 4;

		// Compute CRC from buffer
		CRC_Init();
		CRC_Add(_chunkBuf + 4, bufOffset - 4);
		uint32 crc = CRC_Finish();

		Uint32ToByte4(crc, _chunkBuf + bufOffset);
		bufOffset += 4;

		int64 writeBytes = file->WriteSome(bufOffset, _chunkBuf);
		if(writeBytes != bufOffset)
		{
			ReportError("Failed to store deCf");
		}
	}

#pragma endregion

}