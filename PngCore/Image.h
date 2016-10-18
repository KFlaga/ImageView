#pragma once

#include "TypeDefs.h"

namespace ImgOps
{
	class Image
	{
	protected:
		byte* _dataPtr; // Row-major storage for image matrix
		PixelFormat _format;
		int _width;
		int _height;
		int _pixelSize; // Number of channels * size of channel
		int _channelSize;
		int _stride; // Size of single row (pixel size * width)
		// gamma / color-space
		byte* _palettes; // Storage for palettes (3 bytes per palette) if they are used -> 
		                 // pixels contains indices for this array then
		int _palettesCount;

	public:
		Image(int width, int height, PixelFormat format)
		{
			_width = width;
			_height = height;
			_pixelSize = PixelFormats::GetPixelSize(format);
			_channelSize = _pixelSize / PixelFormats::GetChannels(format);
			_stride = _width * _pixelSize;
			_format = format;

			_dataPtr = (byte*)malloc(_width * _height * _pixelSize);
			_palettes = NULL;
			_palettesCount = 0;
		}
		
		Image(int width, int height, PixelFormat format, int palettes)
		{
			_width = width;
			_height = height;
			_pixelSize = PixelFormats::GetPixelSize(format);
			_channelSize = _pixelSize / PixelFormats::GetChannels(format);
			_stride = _width * _pixelSize;
			_format = format;

			_dataPtr = (byte*)malloc(_width * _height * _pixelSize);

			_palettesCount = palettes;
			_palettes = (byte*)malloc(palettes * 3);
		}

		~Image()
		{
			free(_dataPtr);
			if(_palettes != NULL) free(_palettes);
		}

		int Width() const { return _width; }
		int Height() const { return _height; }
		int PixelSize() const { return _pixelSize; }
		int ChannelSize() const { return _channelSize; }
		int Stride() const { return _stride; }
		int GetPalettesCount() const { return _palettesCount; }

		byte* Data() { return _dataPtr; }

		PixelFormat PixFormat() const { return _format; }

		void SetPalettesCount(int count)
		{
			if(_palettes != NULL) 
				free(_palettes);
			_palettesCount = count;
			if(count > 0)
			{
				_palettes = (byte*)malloc(count * 3);
			}
		}

		// Returns pointer to i-th pixel for storage
		byte* Pixel(int index)
		{
			if(index >= _width * _height)
				return NULL;

			return _dataPtr + index * _pixelSize;
		}
		
		// Returns pointer to pixel on position (y,x)
		byte* Pixel(int y, int x)
		{
			return Pixel(y * _width + x);
		}
		
		// Returns pointer to byte in pixel (y,x) corresponding to given channel
		byte* Pixel(int y, int x, int channel)
		{
			return Pixel(y * _width + x) + channel * _channelSize;
		}

		const byte* Pixel(int index) const
		{
			return _dataPtr + index * _pixelSize;
		}

		const byte* Pixel(int y, int x) const
		{
			return Pixel(y * _width + x);
		}

		const byte* Pixel(int y, int x, int channel) const
		{
			return Pixel(y * _width + x) + channel * _channelSize;
		}

		// Sets value for a pixel, assumes 'value' is pointer to array containing 'pixelSize' bytes
		void SetPixel(int index, byte* value)
		{
			byte* pix = Pixel(index);
			switch (_pixelSize)
			{
			case 8:
				pix[7] = value[7];
				pix[6] = value[6];
			case 6:
				pix[5] = value[5];
				pix[4] = value[4];
			case 4:
				pix[3] = value[3];
			case 3:
				pix[2] = value[2];
			case 2:
				pix[1] = value[1];
			case 1:
				pix[0] = value[0];
				break;
			default:
				break;
			}
		}
		
		// Sets value for a pixel, assumes 'value' is pointer to array containing 'pixelSize' bytes
		void SetPixel(int y, int x, byte* value)
		{
			SetPixel(y * _stride + x, value);
		}
		
		// Sets value for specific channel in pixel, assumes 'value' is pointer to array containing 'channelSize' bytes
		void SetPixel(int y, int x, int channel, byte* value)
		{
			byte* pix = Pixel(y, x, channel);
			switch (_channelSize)
			{
			case 4:
				pix[3] = value[3];
			case 3:
				pix[2] = value[2];
			case 2:
				pix[1] = value[1];
			case 1:
				pix[0] = value[0];
				break;
			default:
				break;
			}
		}
		
		// Returns pointer to i-th palette
		byte* Palette(int index)
		{
			return _palettes + 3 * index;
		}
		
		const byte* Palette(int index) const
		{
			return _palettes + 3 * index;
		}
		
		void SetPalette(int index, byte* color)
		{
			_palettes[3 * index] = *color;
			_palettes[3 * index + 1] = *(color + 1);
			_palettes[3 * index + 2] = *(color + 2);
		}

		// Returns pointer to color for pixel (y,x)
		// Differs from Pixel() only if palettes are used -> it returns color from palette then
		byte* GetColor(int y, int x)
		{
			if(_format & PixelFormats::Indexed)
			{
				return Palette(*Pixel(y,x));
			}
			else
			{
				return Pixel(y, x);
			}
		}
	};
}