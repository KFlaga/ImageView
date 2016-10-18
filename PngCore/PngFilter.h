#pragma once

#include "TypeDefs.h"
#include "Exceptions.h"
#include "PngImage.h"

namespace ImgOps
{
	struct PngFilter
	{
	private:
		PngFilter() { }

	public:
		enum FilterMethod : byte
		{
			None = 0, 
			Sub = 1,
			Up = 2,
			Average = 3,
			Paeth = 4
		};

		static byte FilterByte(byte method, byte currValue, Image* image, uint32 row, uint32 column, byte currentByte)
		{
			byte leftValue = column > 0 ? 
				*(image->Pixel(row, column - 1) + currentByte) : 0;
			byte topValue = row > 0 ? 
				*(image->Pixel(row - 1, column) + currentByte) : 0;
			byte topLeftValue = row > 0 && column > 0 ? 
				*(image->Pixel(row - 1, column - 1) + currentByte) : 0;

			switch (method)
			{
			case None:
				return FilterByte_None(currValue);
			case Sub:
				return FilterByte_Sub(currValue, leftValue);
			case Up:
				return FilterByte_Up(currValue, topValue);
			case Average:
				return FilterByte_Average(currValue, leftValue, topValue);
			case Paeth:
				return FilterByte_Paeth(currValue, leftValue, topValue, topLeftValue);
			default:
				throw Exception("Unsupported filter type");
			};
		}
		
		static byte FilterByte(byte method, byte currValue, byte leftValue, byte topValue, byte topLeftValue)
		{
			switch (method)
			{
			case None:
				return FilterByte_None(currValue);
			case Sub:
				return FilterByte_Sub(currValue, leftValue);
			case Up:
				return FilterByte_Up(currValue, topValue);
			case Average:
				return FilterByte_Average(currValue, leftValue, topValue);
			case Paeth:
				return FilterByte_Paeth(currValue, leftValue, topValue, topLeftValue);
			default:
				throw Exception("Unsupported filter type");
			};
		}

		static byte UnfilterByte(byte method, byte currValue, Image* image, uint32 row, uint32 column, byte currentByte)
		{
			byte leftValue = column > 0 ? 
				*(image->Pixel(row, column - 1) + currentByte) : 0;
			byte topValue = row > 0 ? 
				*(image->Pixel(row - 1, column) + currentByte) : 0;
			byte topLeftValue = row > 0 && column > 0 ? 
				*(image->Pixel(row - 1, column - 1) + currentByte) : 0;

			switch (method)
			{
			case None:
				return UnfilterByte_None(currValue);
			case Sub:
				return UnfilterByte_Sub(currValue, leftValue);
			case Up:
				return UnfilterByte_Up(currValue, topValue);
			case Average:
				return UnfilterByte_Average(currValue, leftValue, topValue);
			case Paeth:
				return UnfilterByte_Paeth(currValue, leftValue, topValue, topLeftValue);
			default:
				throw Exception("Unsupported filter type");
			};
		}
		
		static byte UnfilterByte(byte method, byte currValue, byte leftValue, byte topValue, byte topLeftValue)
		{
			switch (method)
			{
			case None:
				return UnfilterByte_None(currValue);
			case Sub:
				return UnfilterByte_Sub(currValue, leftValue);
			case Up:
				return UnfilterByte_Up(currValue, topValue);
			case Average:
				return UnfilterByte_Average(currValue, leftValue, topValue);
			case Paeth:
				return UnfilterByte_Paeth(currValue, leftValue, topValue, topLeftValue);
			default:
				throw Exception("Unsupported filter type");
			};
		}

		static byte FilterByte_None(byte actualValue)
		{
			return actualValue;
		}

		static byte FilterByte_Sub(byte actualValue, byte left)
		{
			return actualValue - left;
		}

		static byte FilterByte_Up(byte actualValue, byte top)
		{
			return actualValue - top;
		}

		static byte FilterByte_Average(byte actualValue, byte left, byte top)
		{
			return actualValue - (byte)(((uint32)left + (uint32)top) / 2);
		}

		static byte FilterByte_Paeth(byte actualValue, byte left, byte top, byte leftTop)
		{
			return actualValue - PaethPredictor(left, top, leftTop);
		}
		
		static byte UnfilterByte_None(byte actualValue)
		{
			return actualValue;
		}

		static byte UnfilterByte_Sub(byte actualValue, byte left)
		{
			return actualValue + left;
		}

		static byte UnfilterByte_Up(byte actualValue, byte top)
		{
			return actualValue + top;
		}

		static byte UnfilterByte_Average(byte actualValue, byte left, byte top)
		{
			return actualValue + (byte)(((uint32)left + (uint32)top) / 2);
		}

		static byte UnfilterByte_Paeth(byte actualValue, byte left, byte top, byte leftTop)
		{
			return actualValue + PaethPredictor(left, top, leftTop);
		}

		static byte PaethPredictor(byte left, byte top, byte leftTop)
		{
			int p = (int)left + (int)top - (int)leftTop;
			uint32 pL = abs(p - left);
			uint32 pT = abs(p - top);
			uint32 pLT = abs(p - leftTop);
			if(pL <= pT && pL <= pLT)
				return left;
			if(pT <= pLT)
				return top;
			return leftTop;
		}
	};
}