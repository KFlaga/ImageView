#pragma once 

#include <Image.h>

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

namespace ImgOps
{
	public class ImageConverter
	{
	public:
		System::Drawing::Bitmap^ ConvertImage(Image* image)
		{
			int width = image->Width();
			int height = image->Height();
			System::Drawing::Bitmap^ frameworkImage = gcnew System::Drawing::Bitmap(
				width, height, System::Drawing::Imaging::PixelFormat::Format32bppArgb);

			GetColorFunc getColor;
			switch (image->PixFormat())
			{
			case PixelFormats::Gray16:
			case PixelFormats::Gray8: getColor = &ImageConverter::GetColor_Gray8; break;
			case PixelFormats::GrayAlpha32:
			case PixelFormats::GrayAlpha16: getColor = &ImageConverter::GetColor_GrayAlpha16; break;
			case PixelFormats::Indexed: getColor = &ImageConverter::GetColor_Indexed; break;
			case PixelFormats::Rgb48:
			case PixelFormats::Rgb24: getColor = &ImageConverter::GetColor_Rgb24; break;
			case PixelFormats::Rgba64:
			case PixelFormats::Rgba32: getColor = &ImageConverter::GetColor_Rgba32; break;

			default:
				getColor = &ImageConverter::GetColor_Gray8; break;
			}

			for(int y = 0; y < height; ++y)
			{
				for(int x = 0; x < width; ++x)
				{
					frameworkImage->SetPixel(x, y, CALL_MEMBER_FN(*this, getColor)(image, y, x));
				}
			}

			return frameworkImage;
		}

	private:
		typedef System::Drawing::Color (ImageConverter::*GetColorFunc)(Image*, int, int);
		System::Drawing::Color GetColor_Indexed(Image* image, int row, int col)
		{
			byte* imgColor = image->GetColor(row, col);
			return System::Drawing::Color::FromArgb(255, imgColor[0], imgColor[1], imgColor[2]);
		}

		System::Drawing::Color GetColor_Gray8(Image* image, int row, int col)
		{
			// Gray8 or Gray16 (truncate 16bit to 8bit)
			byte* pixelGray = image->Pixel(row, col);
			return System::Drawing::Color::FromArgb(255, pixelGray[0], pixelGray[0], pixelGray[0]);
		}

		System::Drawing::Color GetColor_GrayAlpha16(Image* image, int row, int col)
		{
			// GrayAlpha16 or GrayAlpha32 (truncate 16bit to 8bit for 32bpp)
			byte* pixelGray = image->Pixel(row, col, ColorChannels::Gray); 
			byte* pixelAlpha = image->Pixel(row, col, ColorChannels::AlphaGray); 
			return System::Drawing::Color::FromArgb(*pixelAlpha, pixelGray[0], pixelGray[0], pixelGray[0]);
		}

		System::Drawing::Color GetColor_Rgb24(Image* image, int row, int col)
		{
			// Rgb24 or Rgb48 (truncate 16bit to 8bit for 48bpp)
			byte* pixelRed = image->Pixel(row, col, ColorChannels::Red); 
			byte* pixelGreen = image->Pixel(row, col, ColorChannels::Green); 
			byte* pixelBlue = image->Pixel(row, col, ColorChannels::Blue);
			return System::Drawing::Color::FromArgb(255, pixelRed[0], pixelGreen[0], pixelBlue[0]);
		}

		System::Drawing::Color GetColor_Rgba32(Image* image, int row, int col)
		{
			// Rgba32 or Rgba64 (truncate 16bit to 8bit for 64bpp)
			byte* pixelRed = image->Pixel(row, col, ColorChannels::Red); 
			byte* pixelGreen = image->Pixel(row, col, ColorChannels::Green); 
			byte* pixelBlue = image->Pixel(row, col, ColorChannels::Blue);
			byte* pixelAlpha = image->Pixel(row, col, ColorChannels::AlphaRGB); 
			return System::Drawing::Color::FromArgb(*pixelAlpha, pixelRed[0], pixelGreen[0], pixelBlue[0]);
		}
	};
}