#pragma once

#include <exception>

namespace ImgOps
{
	class Exception : public std::exception
	{
	public:
		Exception() : exception()
		{ }
		
		Exception(const char* txt) : exception(txt)
		{ }
	};

	class DecoderException : public Exception
	{
	public:
		DecoderException() : Exception()
		{ }
		
		DecoderException(const char* txt) : Exception(txt)
		{ }
	};

	class EncoderException : public Exception
	{
	public:
		EncoderException() : Exception()
		{ }
		
		EncoderException(const char* txt) : Exception(txt)
		{ }
	};

}