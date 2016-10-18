#pragma once

#include "TypeDefs.h"

namespace ImgOps
{
	class Image;
	struct Complex
	{
	public:
		double Real;
		double Imag;
	};

	struct FourierData
	{
	public:
		uint32 Width;
		uint32 Height;
		Complex* Data;

		FourierData(uint32 width, uint32 height)
		{
			Width = width;
			Height = height;
			uint32 size = width * height;
			Data = (Complex*)malloc(size * sizeof(Complex));

			// Zero memory:
			for(int i = 0; i < size; ++i)
			{
				Data[i].Imag = 0.0;
				Data[i].Real = 0.0;
			}
		}

		~FourierData()
		{
			free(Data);
		}

		Complex* GetCell(uint32 row, uint32 col)
		{
			return &Data[row * Width + col];
		}
	};

	namespace Fourier // (DFT/FFT based on Paul Bourke implementation) 
	{
		// Returns array with 2d-fourier transform of given image (based on pixel intensity (channels average) or given channel if set >= 0 )
		// Array is row-major matrix of complex values of size image.width * image.height
		// Indexed images are not supported
		FourierData* FourierTransform(Image* image, int channel = -1);

		// Returns image with magnitude of fourier transfrom, in format Gray8
		Image* GetMagnitudeImage(FourierData* fourier, bool scaleLog = false);

		// Returns image with phase of fourier transfrom, in format Gray8
		Image* GetPhaseImage(FourierData* fourier);

		// Performs inverse fourier transform and stores results in Image in format Gray8
		Image* RetrieveImage(FourierData* fourier);

		enum TransfromDirection : int
		{
			Forward = 1,
			Inverse = -1
		};
		
		/*
		1D Direct fourier transform 
		*/
		bool DFT(int dir, int N, double* real, double* imag);

		/*
		Computes an in-place complex-to-complex FFT 
		x and y are the real and imaginary arrays of 2^m points.
		*/
		bool FFT(short int dir, long N, double* real, double* imag);

		/*-------------------------------------------------------------------------
		Perform a 2D FFT inplace given a complex 2D array
		The direction dir, 1 for forward, -1 for reverse
		The size of the array (nx,ny)
		Return false if there are memory problems or
		the dimensions are not powers of 2
		*/
		bool FFT2D(Complex* c, int nx, int ny, int dir);

		/*-------------------------------------------------------------------------
		Calculate the closest but lower power of two of a number
		twopm = 2**m <= n
		Return TRUE if 2**m == n
		*/
		bool Powerof2(int n, int* m, int* twopm);
	}
}