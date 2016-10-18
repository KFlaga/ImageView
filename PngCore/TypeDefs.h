#pragma once

#include <string>

namespace ImgOps
{
	typedef std::string string;
	typedef unsigned __int8 byte;
	typedef __int8 sbyte;
	typedef __int32 int32;
	typedef unsigned __int32 uint32;
	typedef __int64 int64;
	typedef unsigned __int64 uint64;
	
	namespace ColorChannels
	{
		enum ChannelsType : int
		{
			Gray = 0,
			Red = 0,
			Green = 1,
			Blue = 2,
			AlphaRGB = 3,
			AlphaGray = 1
		};
	}
	typedef ColorChannels::ChannelsType ColorChannel;
	
	namespace PixelFormats
	{
		enum PixelFormatType : int
		{
			Unknown = 0,

			PixelSize_8 = 0x1,
			PixelSize_16 = 0x2,
			PixelSize_24 = 0x3,
			PixelSize_32 = 0x4,
			PixelSize_48 = 0x6,
			PixelSize_64 = 0x8,
			PixelSize_128 = 0xF,
			PixelSize_Mask = 0xFF,
			
			Channels_1 = 0x10000,
			Channels_2 = 0x20000,
			Channels_3 = 0x30000,
			Channels_4 = 0x40000,
			Channels_Mask = 0xF0000,
			
			HaveAlphaChannel = 0x100000,
			GrayScale = 0x200000,
			TrueColor = 0x400000,
			Misc_Mask = 0xFF00000,

			Gray8 = 0x100 | PixelSize_8 | Channels_1 | GrayScale,
			Rgb24 = 0x200 | PixelSize_24 | Channels_3 | TrueColor,
			Rgb48 = 0x300 | PixelSize_48 | Channels_3 | TrueColor,
			Rgba32 = 0x400 | PixelSize_32 | Channels_4 | HaveAlphaChannel | TrueColor,
			GrayAlpha16 = 0x500 | PixelSize_16 | Channels_2 | HaveAlphaChannel | GrayScale,
			Gray16 = 0x600 | PixelSize_16 | Channels_1 | GrayScale,
			Rgba64 = 0x700 | PixelSize_64 | Channels_4 | HaveAlphaChannel | TrueColor,
			GrayAlpha32 = 0x800 | PixelSize_32 | Channels_2 | HaveAlphaChannel | GrayScale,
			Indexed = 0x900 | PixelSize_8 | Channels_1,

			// GrayFloat32 = 0xA00 | Channels_1 | PixelSize_32,

			Format_Mask = 0xFF00
		};

		static int GetChannels(PixelFormatType format)
		{
			return (format & Channels_Mask) >> 16;
		}

		static int GetFormat(PixelFormatType format)
		{
			return (format & Format_Mask);
		}
		
		static int GetPixelSize(PixelFormatType format)
		{
			return (format & PixelSize_Mask);
		}
	}
	typedef PixelFormats::PixelFormatType PixelFormat;

	static bool CompareBytes(const byte* arr1, const byte* arr2, const int count)
	{
		for(int i = 0; i < count; ++i)
		{
			if(*(arr1 + i) != *(arr2 + i))
				return false;
		}
		return true;
	}

	inline uint32 Byte4ToUint32(byte* bytes)
	{
		// Cannot do casting conversion due to byte ordering
		// uint32* ptr = reinterpret_cast<uint32*>(bytes);
		// uint32 ptrInt = *ptr;
		return bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];
	}

	inline void Uint32ToByte4(uint32 val, byte* bytes)
	{
		bytes[0] = (byte)(val >> 24);
		bytes[1] = (byte)(val >> 16);
		bytes[2] = (byte)(val >> 8);
		bytes[3] = (byte)(val);
	}
	
	inline int32 Byte4ToInt32(byte* bytes)
	{
		return bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];
	}
	
	inline void Int32ToByte4(int32 val, byte* bytes)
	{
		bytes[0] = (byte)(val >> 24);
		bytes[1] = (byte)(val >> 16);
		bytes[2] = (byte)(val >> 8);
		bytes[3] = (byte)(val);
	}

	static const double Pi  = 3.141592654;
	static const double TwoPi  = 2 * 3.141592654;
	static const double HalfPi  = 0.5 * 3.141592654;
	static const double Const_E = 2.7182818284590;
	static const double Const_EInv = 0.36787944117;
}