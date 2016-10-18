#include "PngImage.h"

namespace ImgOps
{
	uint32 _crc; // Current chunk CRC
	uint32 _crcTable[256]; // Fast CRC table with some magic numbers
	bool _crcTableDone = false;

	void MakeCRCTable()
	{
		uint32 c;
		int n, k;
		for (n = 0; n < 256; n++)
		{
			c = (uint32)n;
			for (k = 0; k < 8; k++)
			{
				if (c & 1)
					c = 0xedb88320L ^ (c >> 1);
				else
					c = c >> 1;
			}
			_crcTable[n] = c;
		}
		_crcTableDone = true;
	}

	// Reset CRC to all 1s (also make fast-crc table on 1st call)
	void CRC_Init()
	{
		//_crc = crc32(0L, Z_NULL, 0);
		_crc = 0xFFFFFFFFL;
		if(_crcTableDone == false)
			MakeCRCTable();
	}

	// Updates CRC with some magic formula
	void CRC_Add(byte* data, uint32 length)
	{
		for (int n = 0; n < length; ++n) 
		{
			_crc = _crcTable[(_crc ^ data[n]) & 0xff] ^ (_crc >> 8);
		}
		//_crc = crc32(_crc, data, length);
	}

	// Finishes CRC computation (XOR with 1s)
	uint32 CRC_Finish()
	{
		return _crc ^ 0xFFFFFFFFL;
		//return _crc;
	}

}