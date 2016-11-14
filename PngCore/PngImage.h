#pragma once

#include "Image.h"
#include "Decoder.h"
#include "Encoder.h"
#include <map>

namespace ImgOps
{
	enum ChunkType : uint32
	{
		None = 0,
		Critical = 0x1, // If 1st bit is not set its ancillary
		Private = 0x2, // If 2nd bit is not set its private
		ReservedSet = 0x4, // Should always be set (chunk is unknown if its not set)
		SafeToCopy = 0x8, // If 4rd bit is not set its unsafe
		ChunkProprtyMask = 0xF,

		StoreChunkInImage = 0x10000, // If it is set by chunk reader, chunk will be stored in image for later copy on saving
		// If it is not set then chunk data will be freed after read (as data is stored in Image in own format)

		// Set of available common chunks
		IHDR_Header = 0x10,
		IDAT_Data = 0x20,
		IEND_End = 0x30,
		PLTE_Palettes = 0x40,
		tRNS_SimpleTransparency = 0x50,
		gAMA_ImageGamma = 0x60,
		cHRM_PrimaryChromaticities = 0x70,
		sRGB_StandardRGB = 0x80,
		iCCP_EmbeddedICCProfile = 0x90,
		tEXt_Text = 0xA0,
		zTXt_CompressedText = 0xB0,
		iTXt_InternationalText = 0xC0,
		bKGD_BackgroundColor = 0xD0,
		pHYs_PhysicalPixelDimensions = 0xE0,
		sBIT_OryginalSignificantBits = 0xF0,
		sPLT_SuggestedPalette = 0x100,
		hIST_PaletteHistogram = 0x110,
		tIME_ModificationTime = 0x120,
		deCf_DecryptedFormat= 0x130,

		ChunkTypeMask = 0xFFF0,
	};

	inline bool CheckIsCritical(uint32 chunkType)
	{
		return ((chunkType >> 24) & 32) == 0;
	}

	inline bool CheckIsAncillary(uint32 chunkType)
	{
		return ((chunkType >> 24) & 32) != 0;
	}

	inline bool CheckIsPublic(uint32 chunkType)
	{
		return ((chunkType >> 16) & 32) == 0;
	}

	inline bool CheckIsPrivate(uint32 chunkType)
	{
		return ((chunkType >> 16) & 32) != 0;
	}

	inline bool CheckIsReservedLow(uint32 chunkType)
	{
		return ((chunkType >> 8) & 32) == 0;
	}

	inline bool CheckIsUnsafeToCopy(uint32 chunkType)
	{
		return ((chunkType) & 32) == 0;
	}

	inline bool CheckIsSafeToCopy(uint32 chunkType)
	{
		return ((chunkType) & 32) != 0;
	}

	enum ChunkTypeBytes : uint32
	{
		// Bytes set associated with those chunks
		IHDR_Bytes = (uint32)'R' | ((uint32)'D' << 8) | 
		((uint32)'H' << 16) | ((uint32)'I' << 24),
		IDAT_Bytes = (uint32)'T' | ((uint32)'A' << 8) | 
		((uint32)'D' << 16) | ((uint32)'I' << 24),
		IEND_Bytes = (uint32)'D' | ((uint32)'N' << 8) | 
		((uint32)'E' << 16) | ((uint32)'I' << 24),
		PLTE_Bytes = (uint32)'E' | ((uint32)'T' << 8) | 
		((uint32)'L' << 16) | ((uint32)'P' << 24),
		tRNS_Bytes = (uint32)'S' | ((uint32)'N' << 8) | 
		((uint32)'R' << 16) | ((uint32)'tS' << 24),
		gAMA_Bytes = (uint32)'A' | ((uint32)'M' << 8) | 
		((uint32)'A' << 16) | ((uint32)'g' << 24),
		cHRM_Bytes = (uint32)'M' | ((uint32)'R' << 8) | 
		((uint32)'H' << 16) | ((uint32)'c' << 24),
		sRGB_Bytes = (uint32)'B' | ((uint32)'G' << 8) | 
		((uint32)'R' << 16) | ((uint32)'s' << 24),
		iCCP_Bytes = (uint32)'P' | ((uint32)'C' << 8) | 
		((uint32)'C' << 16) | ((uint32)'i' << 24),
		tEXt_Bytes = (uint32)'t' | ((uint32)'X' << 8) | 
		((uint32)'E' << 16) | ((uint32)'t' << 24),
		zTXt_Bytes = (uint32)'t' | ((uint32)'X' << 8) | 
		((uint32)'T' << 16) | ((uint32)'z' << 24),
		iTXt_Bytes = (uint32)'t' | ((uint32)'X' << 8) | 
		((uint32)'T' << 16) | ((uint32)'i' << 24),
		bKGD_Bytes = (uint32)'D' | ((uint32)'G' << 8) | 
		((uint32)'K' << 16) | ((uint32)'b' << 24),
		pHYs_Bytes = (uint32)'s' | ((uint32)'Y' << 8) | 
		((uint32)'H' << 16) | ((uint32)'p' << 24),
		sBIT_Bytes = (uint32)'T' | ((uint32)'I' << 8) | 
		((uint32)'B' << 16) | ((uint32)'s' << 24),
		sPLT_Bytes = (uint32)'T' | ((uint32)'L' << 8) | 
		((uint32)'P' << 16) | ((uint32)'s' << 24),
		hIST_Bytes = (uint32)'T' | ((uint32)'S' << 8) | 
		((uint32)'I' << 16) | ((uint32)'h' << 24),
		tIME_Bytes = (uint32)'E' | ((uint32)'M' << 8) | 
		((uint32)'I' << 16) | ((uint32)'t' << 24),
		deCf_Bytes = (uint32)'f' | ((uint32)'C' << 8) | 
		((uint32)'e' << 16) | ((uint32)'d' << 24),
	};

	namespace ColorModes
	{
		enum ColorModeType : byte
		{
			GrayScale = 0,
			RGB = 2,
			Palette = 3,
			GrayAlpha = 4,
			RGBAlpha  = 6
		};
	}
	typedef ColorModes::ColorModeType ColorMode;

	namespace InterlaceMethods
	{
		enum InterlaceMethodType : byte
		{
			None = 0,
			Adam7 = 1
		};
	}
	typedef InterlaceMethods::InterlaceMethodType InterlaceMethod;

	struct ChunkInfo
	{
		uint32 Lenght;
		uint32 TypeBytes;
		uint32 Type;
		uint32 CRCExpected;
		int Position;
		byte* ChunkData;
	};

	class PNGImageDecoder;
	struct ChunkReader
	{
		PNGImageDecoder* _decoder;
	public:
		ChunkReader(PNGImageDecoder* decoder)
		{
			_decoder = decoder;
		}

		virtual void operator()(ChunkInfo* cinfo, byte* chunkData) = 0;
	};

	namespace PositionFlags
	{
		enum PositionFlagType : byte
		{
			JustStarted = 0,
			IHDR_Read = 0x1,
			PLTE_Read = 0x2,
			IDAT_Started = 0x4,
			IDAT_Finished = 0x8,
			IEND_Read = 0x10,
		};
	}
	typedef PositionFlags::PositionFlagType PositionFlag;

	// Reset CRC to all 1s (also make fast-crc table on 1st call)
	void CRC_Init();
	// Updates CRC with some magic formula
	void CRC_Add(byte* data, uint32 length);
	// Finishes CRC computation (XOR with 1s)
	uint32 CRC_Finish();

	class PNGImageDecoder : public ImageDecoder
	{
	private:
		std::map<uint32, ChunkReader*> _chunkReaders; // Contains all available chunk readers with chunk type byte-code as keys

		Image* _image; // Decoded image
		byte _chunkInfoBuf[16]; // For storing info abount chunk (max 8 bytes)

		bool _imageInterlaced;

		int _decodingPosition; // Info on already read chunks (to check validity of chunk ordering)

	public:
		PNGImageDecoder();
		~PNGImageDecoder();

		void SetImageInfo(int width, int height, PixelFormat pixFormat);
		Image* GetImage() { return _image; }

		PositionFlag GetPositionFlags() const { return (PositionFlag)_decodingPosition; }
		void AddPositionFlags(PositionFlag flag) { _decodingPosition |= flag; }
		bool CheckPositionFlag(PositionFlag flag) const { return (_decodingPosition & flag) != 0; }

		void SetImageInterlaced(bool val) { _imageInterlaced = val; }
		bool IsImageInterlaced() const { return _imageInterlaced; }

		void FreeMemory(bool removeImage);
		void ReportError(const char* error);

		Image* ReadImageFromFile(const char* filePath);
		Image* ReadImageFromFile(FileStream* file);

	private:
		void ReadNextChunk(FileStream* file);
		void ReadImageFromFile_Internal(FileStream* file);
	};

	class PNGImageEncoder : public ImageEncoder
	{
	public:
		static const int ChunkBufferSize = 65536;
		static const int ImageBufferSize = 65536;

	private:
		Image* _image;
		byte _chunkBuf[ChunkBufferSize];
		byte _filteredImageBuf[ImageBufferSize];
		bool _saveInterlaced;

	public:
		PNGImageEncoder();
		~PNGImageEncoder();

		void SetImage(Image* image);
		Image* GetImage() { return _image; }

		void SetImageInterlaced(bool val) { _saveInterlaced = val; }
		bool IsImageInterlaced() const { return _saveInterlaced; }

		void FreeMemory();
		void ReportError(const char* error);

		bool SaveImageToFile(const char* filePath, Image* image);
		bool SaveImageToFile(FileStream* file, Image* image);

	private:
		void SaveImageToFile_Internal(FileStream* file);

		void StoreChunk_IHDR(FileStream* file);
		void StoreChunk_PLTE(FileStream* file);
		void StoreChunk_IDAT(FileStream* file);
		void StoreChunk_IEND(FileStream* file);
		void StoreChunk_deCf(FileStream* file);

		// Stores filtered rows in '_filteredImageBuf', uses fixed 'filter' method or adaptative if 'filter' = -1
		uint32 FilterRows(uint32 startRow, uint32 rowsCount, int filter = -1);
	};
}