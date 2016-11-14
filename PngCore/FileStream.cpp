#include "FileStream.h"
#include <stdio.h>
//#include <io.h>

namespace ImgOps
{
#define File reinterpret_cast<FILE*>(_file)

	FileStream::FileStream(const char* filePath, OpenMode openMode, uint32 buffSize)
	{
		_filePath = filePath;

		string stdOpenMode = "";
		switch (openMode)
		{
		case OpenMode::Read: stdOpenMode = "rb"; break;
		case OpenMode::ReadWrite: stdOpenMode = "r+b"; break;
		case OpenMode::ReadWriteAppend: stdOpenMode = "a+b"; break;
		case OpenMode::ReadWriteTrunc: stdOpenMode = "w+b"; break;
		case OpenMode::Append: 
		case OpenMode::WriteAppend: stdOpenMode = "ab"; break;
		case OpenMode::Write: 
		case OpenMode::Trunc: 
		case OpenMode::WriteTrunc: stdOpenMode = "wb"; break;
		default:
			return;
		}

		fopen_s((FILE**)(&_file), filePath, stdOpenMode.c_str());
		if(_file == NULL)
			return;

		setvbuf(File, NULL, _IOFBF, buffSize);

		//int res = _fseeki64((FILE*)_file, 0, SEEK_END);
		//if(res == -1)
		//	Close();

		//_length = _ftelli64(File);
		rewind(File);
		_openMode = openMode;
	}

	FileStream::~FileStream()
	{
		Close();
	}

	void FileStream::Close()
	{
		if(_file != NULL)
		{
			fclose(File);
			_file = NULL;
		}
	}

	bool FileStream::NextByte()
	{
		return _fseeki64(File, 1, SEEK_CUR) != -1;
	}

	int64 FileStream::Position()
	{
		return _ftelli64(File);
	}

	bool FileStream::SetPosition(int64 pos)
	{
		return _fseeki64(File, pos, SEEK_SET) != -1;
	}

	bool FileStream::MovePosition(int64 pos)
	{
		return _fseeki64(File, pos, SEEK_CUR) != -1;
	}

	bool FileStream::ReadByte(byte* data)
	{
		int c = fgetc(File);
		*data = (byte)c;
		if(c == EOF)
			return false;
		return true;
	}

	bool FileStream::WriteByte(byte data)
	{
		int c = fputc(data, File);
		if(c == EOF)
			return false;
		return true;
	}

	int64 FileStream::ReadSome(int64 bytesToRead, byte* buffer)
	{
		return fread(buffer, 1, bytesToRead, File);
	}

	int64 FileStream::ReadLine(int64 bytesToRead, byte* buffer)
	{
		if(fgets((char*)buffer, bytesToRead, File) != NULL)
		{
			for(int p = 0; p < bytesToRead; ++p)
			{
				byte c = buffer[p];
				if(c == '\r' || c == '\n' || c == EOF) // New line / EOF
				{
					buffer[p] = '\0';
					return p;
				}
			}
		}
		return -1;
	}

	int64 FileStream::WriteSome(int64 bytesToWrite, byte* buffer)
	{
		return fwrite(buffer, 1, bytesToWrite, File);
	}
}