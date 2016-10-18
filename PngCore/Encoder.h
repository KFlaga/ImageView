#pragma once

namespace ImgOps
{
	class FileStream;
	class Image;

	__interface ImageEncoder
	{
	public:
		virtual bool SaveImageToFile(const char* filePath, Image* image) = 0;
		virtual bool SaveImageToFile(FileStream* file, Image* image) = 0;
	};

	ImageEncoder* CreatePNGEncoder();
}