#pragma once

#include "TypeDefs.h"

namespace ImgOps
{
	__interface IDataStream
	{
	public:
		virtual int64 Length() = 0;
		virtual bool IsOpen() = 0;

		virtual bool NextByte() = 0;
		virtual int64 Position() = 0;
		virtual bool SetPosition(int64 pos) = 0;
		virtual bool MovePosition(int64 posChange) = 0;

		virtual bool ReadByte(byte* data) = 0;
		virtual bool WriteByte(byte data) = 0;

		virtual int64 ReadSome(int64 bytesToRead, byte* buffer) = 0;
		virtual int64 WriteSome(int64 bytesToWrite, byte* buffer) = 0;
	};
}