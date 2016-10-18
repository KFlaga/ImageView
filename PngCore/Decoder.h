#pragma once

namespace ImgOps
{
	class FileStream;
	class Image;

	__interface ImageDecoder
	{
	public:
		virtual Image* ReadImageFromFile(const char* filePath) = 0;
		virtual Image* ReadImageFromFile(FileStream* file) = 0;
	};

	ImageDecoder* CreatePNGDecoder();
}