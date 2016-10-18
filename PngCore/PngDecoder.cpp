
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

	ImageDecoder* CreatePNGDecoder()
	{
		return new PNGImageDecoder();
	}

	void PNGImageDecoder::SetImageInfo(int width, int height, PixelFormat pixFormat)
	{
		_image = new Image(width, height, pixFormat);
	}

	// TODO: store current chunk in class and free it here
	void PNGImageDecoder::FreeMemory(bool removeImage)
	{
		if(removeImage && _image!= NULL)
		{
			delete _image;
			_image = NULL;
		}

		for(auto it = _chunkReaders.begin(); it != _chunkReaders.end(); ++it)
		{
			delete it->second;
		}
		_chunkReaders.clear();
	}

	Image* PNGImageDecoder::ReadImageFromFile(const char* filePath)
	{
		FileStream file(filePath, OpenModes::Read);
		if(file.IsOpen())
		{
			return ReadImageFromFile(&file);
		}
		else
		{
			return NULL;
		}
	}

	Image* PNGImageDecoder::ReadImageFromFile(FileStream* file)
	{
		try
		{
			ReadImageFromFile_Internal(file);
		}
		catch(Exception e)
		{
			FreeMemory(true);
			return NULL;
		}
		return _image;
	}

	void PNGImageDecoder::ReadImageFromFile_Internal(FileStream* file)
	{
		// Read png header : 8 bytes
		int headerSize = 8;
		int64 readBytes = file->ReadSome(headerSize, _chunkInfoBuf);
		if(readBytes != headerSize)
		{
			ReportError("Failed to read image header");
		}

		if(CompareBytes(_chunkInfoBuf, PNGHeaderBytes, headerSize) == false)
		{
			ReportError("Invalid PNG image header");
		}

		_decodingPosition = PositionFlags::JustStarted;

		// We got valid header -> start reading chunks untill END chunk
		while(!CheckPositionFlag(PositionFlags::IEND_Read) && file->Position() != EOF)
		{
			ReadNextChunk(file);
		}

		if(CheckPositionFlag(PositionFlags::IEND_Read) == false)
		{
			ReportError("File ended before IEND chunk");
		}
	}

	void PNGImageDecoder::ReadNextChunk(FileStream* file)
	{
		ChunkInfo* info = new ChunkInfo();
		info->Type = None;
		// Read first 8 bytes
		int64 readCount = file->ReadSome(8, _chunkInfoBuf);
		if(readCount != 8) ReportError("Failed to read chunk length");

		// Convert first 4 bytes to uint32 length, next 4 bytes to chunk type
		info->Lenght = Byte4ToUint32(_chunkInfoBuf);
		info->TypeBytes = Byte4ToUint32(_chunkInfoBuf + 4);

		// Read chunk properties:
		if(CheckIsCritical(info->TypeBytes)) info->Type |= Critical;
		if(CheckIsPrivate(info->TypeBytes)) info->Type |= Private;
		if(CheckIsReservedLow(info->TypeBytes)) info->Type |= ReservedSet;
		if(CheckIsSafeToCopy(info->TypeBytes)) info->Type |= SafeToCopy;

		// Read chunk type data (length) + CRC (4)
		byte* chunkData = (byte*)malloc(info->Lenght + 4);
		readCount = file->ReadSome(info->Lenght + 4, chunkData);
		if(readCount != info->Lenght + 4) ReportError("Failed to read chunk data");
		info->ChunkData = chunkData;

		// Read expected CRC and init its computation
		info->CRCExpected = Byte4ToUint32(chunkData + info->Lenght);
		CRC_Init();
		// Add 4 bytes from ChunkType to CRC, then add chunk data
		CRC_Add(_chunkInfoBuf + 4, 4);
		CRC_Add(chunkData, info->Lenght);
		uint32 crc = CRC_Finish();
		bool crcGood = crc == info->CRCExpected;
		if(!crcGood) 
		{
			if(info->Type & Critical)
				ReportError("Check-sum is invalid : corrupted file");
		}
		else
		{
			// Save current position ( used when storing image )
			info->Position = _decodingPosition;

			// Now determine if chunk is recognized
			auto readerIt = _chunkReaders.find(info->TypeBytes);
			if(readerIt == _chunkReaders.end())
			{
				// Not recognized chunk :
				// if its critical we have unreadable image
				if(info->Type & Critical)
					ReportError("Unrecognized critical chunk");
				//else
				//	info->Type |= StoreChunkInImage; // If its not critical, store it so it will be copied when saving
			}
			else
			{
				// We got recognized chunk -> so process it
				(*readerIt->second)(info, chunkData);
			}
		}

		//if(info->Type & StoreChunkInImage)
		//{
			// Chunk is marked as to be stored in image
			//_image->SavedChunks().push_back(info);
		//}
		//else
		//{
			// Chunk data may be freed as it no longer necessary
			free(chunkData);
			delete info;
		//}
	}

	void PNGImageDecoder::ReportError(const char* error)
	{
		throw DecoderException(error);
	}

#pragma region CHUNK_READERS

	//====================================================//
	//================== CHUNK READERS ===================//

	struct ChunkReader_IHDR : public ChunkReader
	{
		ChunkReader_IHDR(PNGImageDecoder* decoder) : ChunkReader(decoder) { }
		void operator()(ChunkInfo* info, byte* data)
		{
			// Contents:
			// 4bytes[0] : width 
			// 4bytes[4] : height
			// 1byte[8]  : bit depth
			// 1byte[9]  : color type
			// 1byte[10] : compression method
			// 1byte[11] : filter method
			// 1byte[12] : interlace method
			uint32 width =  Byte4ToUint32(data);
			uint32 height = Byte4ToUint32(data+4);
			byte bitDepth = data[8];
			byte colorType = data[9];
			byte compreesionMethod = data[10];
			byte filterMethod = data[11];
			byte interlaceMethod = data[12];
			byte sampleSize = bitDepth;

			// Check validity
			if(width == 0 || height == 0)
				_decoder->ReportError("Zero width/height not supported");

			PixelFormat pixFormat = PixelFormat::Unknown;
			switch(colorType)
			{
			case ColorModes::GrayScale:
				if(bitDepth == 1 || bitDepth == 2 || bitDepth == 4 )
					_decoder->ReportError("BitDepth < 8 not yet supported");

				if(bitDepth == 8)
					pixFormat = PixelFormats::Gray8;
				else if(bitDepth == 16)
					pixFormat = PixelFormats::Gray16;
				else 
					_decoder->ReportError("Unsupported bitdepth encountered");
				break;
			case ColorModes::RGB:
				if(bitDepth == 8)
					pixFormat = PixelFormats::Rgb24;
				else if(bitDepth == 16)
					pixFormat = PixelFormats::Rgb48;
				else 
					_decoder->ReportError("Unsupported bitdepth encountered");
				break;
			case ColorModes::RGBAlpha:
				if(bitDepth == 8)
					pixFormat = PixelFormats::Rgba32;
				else if(bitDepth == 16)
					pixFormat = PixelFormats::Rgba64;
				else 
					_decoder->ReportError("Unsupported bitdepth encountered");
				break;
			case ColorModes::GrayAlpha:
				if(bitDepth == 8)
					pixFormat = PixelFormats::GrayAlpha16;
				else if(bitDepth == 16)
					pixFormat = PixelFormats::GrayAlpha32;
				else 
					_decoder->ReportError("Unsupported bitdepth encountered");
				break;
			case ColorModes::Palette:
				if(!(bitDepth == 1 || bitDepth == 2 || bitDepth == 4 || bitDepth == 8))		
					_decoder->ReportError("Unsupported bitdepth encountered");
				pixFormat = PixelFormats::Indexed;
				sampleSize = 8;
				break;
			default:
				_decoder->ReportError("Unsupported color mode encountered");
			}

			if(compreesionMethod != 0)
				_decoder->ReportError("Unsupported compression method encountered");

			if(filterMethod != 0)
				_decoder->ReportError("Unsupported filter method encountered");

			if(interlaceMethod > 1)
				_decoder->ReportError("Unsupported interlace method encountered");

			_decoder->SetImageInfo(width, height, pixFormat);
			_decoder->SetImageInterlaced(interlaceMethod == 1);
			//_decoder->SampleBitSize = sampleSize;
			_decoder->AddPositionFlags(PositionFlags::IHDR_Read);
		}
	};

	struct ChunkReader_IEND : public ChunkReader
	{
		ChunkReader_IEND(PNGImageDecoder* decoder) : ChunkReader(decoder) { }
		void operator()(ChunkInfo* info, byte* data)
		{
			if(_decoder->CheckPositionFlag(PositionFlags::IHDR_Read) == false)
				_decoder->ReportError("IEND appered before IHDR");

			if(_decoder->CheckPositionFlag(PositionFlags::IDAT_Finished) == false)
				_decoder->ReportError("IEND appered before IDAT finished");

			_decoder->AddPositionFlags(PositionFlags::IEND_Read);
		}
	};

	struct ChunkReader_IDAT : public ChunkReader
	{
		static const int ChunkBufferSize = 65536u;
		uint32 CurrentRow;
		uint32 CurrentColumn;
		bool IsInterlaced;
		z_stream Zlib;
		byte OutBuf[ChunkBufferSize];
		byte CurrentRowFilter;
		byte RemBytesInPixel;

		ChunkReader_IDAT(PNGImageDecoder* decoder) : ChunkReader(decoder)
		{
			CurrentRow = 0;
			CurrentColumn = 0;
			IsInterlaced = decoder->IsImageInterlaced();
			CurrentRowFilter = 255;
		}

		void operator()(ChunkInfo* info, byte* data)
		{
			if(_decoder->CheckPositionFlag(PositionFlags::IHDR_Read) == false)
				_decoder->ReportError("IDAT appered before IHDR");

			if(_decoder->CheckPositionFlag(PositionFlags::IDAT_Finished))
				_decoder->ReportError("Image data in incontinous IDATs");

			if((_decoder->GetImage()->PixFormat() == PixelFormats::Indexed) &&
				(_decoder->CheckPositionFlag(PositionFlags::PLTE_Read) == false))
			{
				_decoder->ReportError("IDAT appeared before PLTE in indexed image");
			}

			if(_decoder->CheckPositionFlag(PositionFlags::IDAT_Started) == false)
			{
				// First chunk with image data -> init read
				_decoder->AddPositionFlags(PositionFlags::IDAT_Started);
				InitIDATRead();
			}

			// Compressed format :
			// 1byte : compression method/flags (must be 8 for method and LZ77 window max 32kB)
			// 1byte : additional flags
			// nbytes : data
			// 4bytes : check values
			byte compressionMethod = *data;
			byte additionalFlags = *(data+1);

			Zlib.next_out = OutBuf;
			Zlib.avail_out = ChunkBufferSize;
			Zlib.avail_in = info->Lenght;
			Zlib.next_in = (data);
			int retVal;

			do
			{
				retVal = inflate(&Zlib, Z_PARTIAL_FLUSH);

				switch (retVal) 
				{
				case Z_NEED_DICT:
					inflateEnd(&Zlib);
					_decoder->ReportError("Zlib failed to decompress image data : need dict");
					return;
				case Z_DATA_ERROR:
					inflateEnd(&Zlib);
					_decoder->ReportError("Zlib failed to decompress image data : data error");
					return;
				case Z_MEM_ERROR:
					inflateEnd(&Zlib);
					_decoder->ReportError("Zlib failed to decompress image data : memory error");
					return;
				}

				// As image is saved as continous zlib stream, end of stream means end of image
				if (retVal == Z_STREAM_END)
				{
					SaveImageData(info, OutBuf, ChunkBufferSize - Zlib.avail_out);

					if (Zlib.avail_in != 0)
						_decoder->ReportError("Extra compressed data");

					_decoder->AddPositionFlags(PositionFlags::IDAT_Finished);
					inflateEnd(&Zlib);
					break;
				}

				if(Zlib.avail_out == 0 || Zlib.avail_in == 0)
				{
					// Save uncompressed data 
					SaveImageData(info, OutBuf, ChunkBufferSize - Zlib.avail_out);

					if(Zlib.avail_in == 0) // Finished chunk
						break;

					// Some data in chunk remains, but output buffer is too small so refresh it
					Zlib.avail_out = ChunkBufferSize;
					Zlib.next_out = OutBuf;
				}
			} 
			while (Zlib.avail_out != 0);
		}

		void InitIDATRead()
		{
			Zlib = z_stream();
			CurrentColumn = 0;
			CurrentRow = 0;
			RemBytesInPixel = _decoder->GetImage()->PixelSize();

			Zlib.zalloc = Z_NULL;
			Zlib.zfree = Z_NULL;
			Zlib.opaque = Z_NULL;
			int retVal = inflateInit(&Zlib);
			if(retVal != Z_OK)
			{
				_decoder->ReportError("Zlib failed to initialize");
			}
		}

		void SaveImageData(ChunkInfo* cinfo, byte* imgData, uint32 dataLength)
		{
			// We have uncompressed data here
			int dataRemaining = (int)dataLength;
			byte* currentDataPtr = imgData;

			while(CurrentRow < _decoder->GetImage()->Height())
			{
				if(dataRemaining == 0) // Chunk ended before CurrentRow
					return;

				if(CurrentColumn == 0 && CurrentRowFilter == 255)
				{
					// Read filter type (1st byte for each row)
					// If CurrentRowFilter wasn't -1 then this row is continued from previous chunk
					CurrentRowFilter = *currentDataPtr;
					currentDataPtr += 1;
					dataRemaining -= 1;
				}

				while(CurrentColumn < _decoder->GetImage()->Width())
				{
					if(dataRemaining == 0) // Chunk ended before CurrentColumn
						return;

					while(RemBytesInPixel > 0)
					{
						if(dataRemaining == 0) // Chunk ended between pixel samples -> continue on next chunk
							return;

						// For each byte in pixel unfilter it and store unfiltered
						// TODO: add 1/2/4 bit depth unfilter support
						int currByte = _decoder->GetImage()->PixelSize() - RemBytesInPixel;
						byte filtered = PngFilter::UnfilterByte(CurrentRowFilter, *currentDataPtr,
							_decoder->GetImage(), CurrentRow, CurrentColumn, currByte);
						*(_decoder->GetImage()->Pixel(CurrentRow, CurrentColumn) + currByte) = filtered;

						--RemBytesInPixel;
						--dataRemaining;
						++currentDataPtr;
					}
					// Set next pixel to be read
					RemBytesInPixel = _decoder->GetImage()->PixelSize(); 
					++CurrentColumn;
				}
				CurrentColumn = 0;
				CurrentRowFilter = 255;
				++CurrentRow;
			}

			if(dataRemaining != 0 || CurrentRow != _decoder->GetImage()->Height())
			{
				_decoder->ReportError("Incorrect amount of image data");
			}
		}
	};

	struct ChunkReader_PLTE : public ChunkReader
	{
		ChunkReader_PLTE(PNGImageDecoder* decoder) : ChunkReader(decoder)
		{

		}

		void operator()(ChunkInfo* info, byte* data)
		{
			if(_decoder->CheckPositionFlag(PositionFlags::IHDR_Read) == false)
				_decoder->ReportError("PLTE appered before IHDR");

			if(_decoder->CheckPositionFlag(PositionFlags::IDAT_Started))
				_decoder->ReportError("PLTE appeared after IDAT");

			_decoder->AddPositionFlags(PositionFlags::PLTE_Read);

			// Each 3bytes of data contains [r,g,b] colors for palette
			// Up to 256 palettes, count determined from length (count = length/3)
			// If length is not divisable by 3, its an error
			if(info->Lenght % 3 != 0)
				_decoder->ReportError("PLTE length not divisable by 3");

			int count = info->Lenght / 3;
			_decoder->GetImage()->SetPalettesCount(count);
			for(int p = 0; p < count; ++p)
			{
				byte* palette = _decoder->GetImage()->Palette(p);
				palette[0] = data[3 * p];
				palette[1] = data[3 * p + 1];
				palette[2] = data[3 * p + 2];
			}
		}
	};

#pragma endregion

	PNGImageDecoder::PNGImageDecoder()
	{
		_image = NULL;
		_chunkReaders[IHDR_Bytes] = new ChunkReader_IHDR(this);
		_chunkReaders[IEND_Bytes] = new ChunkReader_IEND(this);
		_chunkReaders[IDAT_Bytes] = new ChunkReader_IDAT(this);
		_chunkReaders[PLTE_Bytes] = new ChunkReader_PLTE(this);
	}

	PNGImageDecoder::~PNGImageDecoder()
	{
		FreeMemory(false);
	}
}