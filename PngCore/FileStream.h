#pragma once

#include "TypeDefs.h"
//#include "PngDataStream.h"

namespace ImgOps
{
	namespace OpenModes
	{
		enum OpenModeType : int
		{
			Unknown = 0,
			Read = 1,
			Write = 2,
			Append = 4,
			Trunc = 8,
			WriteAppend = Write | Append,
			WriteTrunc = Write | Trunc,
			ReadWrite = Read | Write,
			ReadWriteAppend = ReadWrite | Append,
			ReadWriteTrunc = ReadWrite | Trunc
		};
	}
	typedef OpenModes::OpenModeType OpenMode;
	
	class FileStream // : IDataStream
	{
	protected:
		void* _file;
		string _filePath;
		//int64 _length;
		OpenMode _openMode;

	public:
		FileStream(const char* filePath, OpenMode openMode, uint32 bufferSize = 512u);
		~FileStream();

		string FilePath() { return _filePath; }
		//int64 Length() { return _length; }
		bool IsOpen() { return _file != NULL; }

		bool NextByte();
		int64 Position();
		bool SetPosition(int64 pos);
		bool MovePosition(int64 posChange);

		bool ReadByte(byte* data);
		bool WriteByte(byte data);

		int64 ReadSome(int64 bytesToRead, byte* buffer);
		int64 WriteSome(int64 bytesToWrite, byte* buffer);

	protected:
		void Close();
		
	private:
		FileStream(const FileStream&);
		FileStream& operator=(const FileStream&);
	};
}