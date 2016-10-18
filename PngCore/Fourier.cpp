#include "Fourier.h"
#include "Image.h"

namespace ImgOps
{
	namespace Fourier
	{
		double PixelValue_Gray(Image* image, int row, int col)
		{
			return ((double)*image->Pixel(row, col)) / 255.0;
		}

		double PixelValue_Color(Image* image, int row, int col)
		{
			byte* pixel = image->Pixel(row, col);
			return ((double)((int)pixel[0] + (int)pixel[1] + (int)pixel[2])) / 765.0;
		}

		int _channel;
		double PixelValue_Channel(Image* image, int row, int col)
		{
			return ((double)*image->Pixel(row, col, _channel)) / 2555.0;
		}

		FourierData* FourierTransform(Image* image, int channel)
		{
			// 1) Create fourier array : dimensions of image must be powers of 2, so if they are not, we have somes options:
			// a) down-scale image to nearest power of 2
			// b) up-scale image to nearest power of 2
			// c) crop image preserving center
			// d) place image on center of bigger black image
			// For now lets do option d)

			// 1.1) Check if height is power of 2
			// Starting from MSB save first set bit (high_bit) and check if there is more than one
			// If there is more, its not power of 2
			// Next bigger pow2 is number with bit (high_bit+1) set
			uint32 height = image->Height();
			int highBit = -1;
			int bitsSet = 0;
			for(int bit = 0; bit < 32; ++bit)
			{
				if( ((height >> bit) & 1) == 1 )
				{
					// Height have 'bit' bit set and it higher than previous highest
					highBit = bit;
					++bitsSet;
				}
			}

			uint32 newHeight = height;
			if(bitsSet > 1)
			{
				// Height is not power of 2
				newHeight = 1 << (highBit + 1);
			}

			// 1.2) Same with width
			uint32 width = image->Width();
			highBit = -1;
			bitsSet = 0;
			for(int bit = 0; bit < 32; ++bit)
			{
				if( ((width >> bit) & 1) == 1 )
				{
					// Height have 'bit' bit set and it higher than previous highest
					highBit = bit;
					++bitsSet;
				}
			}

			uint32 newWidth = width;
			if(bitsSet > 1)
				newWidth = 1 << (highBit + 1);

			// Compute top-left of original image
			uint32 left = (newWidth - width) / 2;
			uint32 top = (newHeight - height) / 2;

			uint32 imgSize = newHeight * newWidth;
			FourierData* fourier = new FourierData(newWidth, newHeight);

			// Set correct pixel-value getter based on pixformat and desired channel
			_channel = channel;
			auto pixelValue = channel < 0 ? 
				((image->PixFormat() & PixelFormats::TrueColor) != 0 ? PixelValue_Color : PixelValue_Gray)
				: PixelValue_Channel;

			// Fill array with image info
			for(int row = 0; row < image->Height(); ++row)
			{
				int frow = row + top;
				for(int col = 0; col < image->Width(); ++col)
				{
					int fcol = col + left;
					fourier->GetCell(frow, fcol)->Real = pixelValue(image, row, col);
				}
			}

			if( FFT2D(fourier->Data, newWidth, newHeight, TransfromDirection::Forward) == false )
			{
				delete fourier;
				return NULL;
			}

			return fourier;
		}

		inline byte GetMagnitudeByte(FourierData* fourier, 
			int row, int col, double magScale, double logScale, bool scaleLog)
		{
			Complex* f = fourier->GetCell(row, col);
			double mag = sqrt(f->Real * f->Real + f->Imag * f->Imag) * magScale;
			mag = scaleLog ? (log(mag + 1.0)) * logScale : mag;
			return (byte)((int)(mag * 255.0));
		}

		Image* GetMagnitudeImage(FourierData* fourier, bool scaleLog)
		{
			// Magintude : M(y,x) = sqrt(F(y,x).R^2 + F(y,x).I^2)
			Image* image = new Image(fourier->Width, fourier->Height, PixelFormats::Gray8);

			int fsize = fourier->Height * fourier->Width;
			Complex* f;
			double maxMag = 0.0;
			double mag;
			for(int i = 0; i < fsize; ++i)
			{
				f = &fourier->Data[i];
				mag = f->Real * f->Real + f->Imag * f->Imag;
				maxMag = maxMag < mag ? mag : maxMag;
			}

			double scale = 1.0 / sqrt(maxMag);
			double logScale = 1.0 / log(2.0);
			// We need to translate fourier image so that bias (fourier(0,0)) is on image center
			// So lets divide image to four parts :
			// TopLeft will be shifted by (+width/2, +height/2)
			// TopRight will be shifted by (-width/2, +height/2)
			// BotLeft will be shifted by (+width/2, -height/2)
			// BotRight will be shifted by (-width/2, -height/2)
			int h2 = image->Height() / 2;
			int w2 = image->Width() / 2;

			// TOP LEFT
			for(int row = 0; row < h2; ++row)
				for(int col = 0; col < w2; ++col)
					image->Pixel(row + h2, col + w2)[0] = 
					GetMagnitudeByte(fourier, row, col, scale, logScale, scaleLog);
			// Top Right
			for(int row = 0; row < h2; ++row)
				for(int col = w2; col < image->Width(); ++col)
					image->Pixel(row + h2, col - w2)[0] = 
					GetMagnitudeByte(fourier, row, col, scale, logScale, scaleLog);
			// Bot Left
			for(int row = h2; row < image->Height(); ++row)
				for(int col = 0; col < w2; ++col)
					image->Pixel(row - h2, col + w2)[0] = 
					GetMagnitudeByte(fourier, row, col, scale, logScale, scaleLog);
			// Bot Right
			for(int row = h2; row < image->Height(); ++row)
				for(int col = w2; col < image->Width(); ++col)
					image->Pixel(row - h2, col - w2)[0] = 
					GetMagnitudeByte(fourier, row, col, scale, logScale, scaleLog);

			return image;
		}


		inline byte GetPhaseByte(FourierData* fourier, int row, int col)
		{
			Complex* f = fourier->GetCell(row, col);
			double arg = atan2(f->Imag, f->Real); // Arg is  in range [-pi,pi]
			return (byte)((uint32)(((arg + Pi) / TwoPi ) * 255.0));
		}

		Image* GetPhaseImage(FourierData* fourier)
		{
			// Phase : A(y,x) = atan2(F(y,x).I, F(y,x).R)
			Image* image = new Image(fourier->Width, fourier->Height, PixelFormats::Gray8);

			int h2 = image->Height() / 2;
			int w2 = image->Width() / 2;
			
			// TOP LEFT
			for(int row = 0; row < h2; ++row)
				for(int col = 0; col < w2; ++col)
					image->Pixel(row + h2, col + w2)[0] = GetPhaseByte(fourier, row, col);
			// Top Right
			for(int row = 0; row < h2; ++row)
				for(int col = w2; col < image->Width(); ++col)
					image->Pixel(row + h2, col - w2)[0] = GetPhaseByte(fourier, row, col);
			// Bot Left
			for(int row = h2; row < image->Height(); ++row)
				for(int col = 0; col < w2; ++col)
					image->Pixel(row - h2, col + w2)[0] = GetPhaseByte(fourier, row, col);
			// Bot Right
			for(int row = h2; row < image->Height(); ++row)
				for(int col = w2; col < image->Width(); ++col)
					image->Pixel(row - h2, col - w2)[0] = GetPhaseByte(fourier, row, col);

			return image;
		}


		bool DFT(int dir, int N, double* real, double* imag)
		{
			long i,k;
			double arg;
			double cosarg, sinarg;
			double *x2 = NULL, *y2 = NULL;

			x2 = (double*)malloc(N * sizeof(double));
			y2 = (double*)malloc(N * sizeof(double));
			if (x2 == NULL || y2 == NULL)
				return false;

			// F(i) = 1/N sum{k=[0,N)}(f(k)exp(-j*k*2Pi*i/N))
			// f(i) =     sum{k=[0,N)}(F(k)exp(j*k*2Pi*i/N))
			double nInv = 1.0 / (double)N;
			for(i = 0; i < N; ++i) 
			{
				x2[i] = 0;
				y2[i] = 0;
				arg = - dir * TwoPi * (double)i * nInv; // 2Pi*i/N
				for (k = 0; k < N; ++k) 
				{
					// exp(jx) = cos(x) + jsin(x)
					cosarg = cos(k * arg);
					sinarg = sin(k * arg);
					x2[i] += (real[k] * cosarg - imag[k] * sinarg); // f(k).real * exp(jx)
					y2[i] += (real[k] * sinarg + imag[k] * cosarg); // f(k).imag * exp(jx)
				}
			}

			/* Copy the data back */
			nInv = dir == 1 ? nInv : 1.0; // Scale forward transfrom
			for (i = 0; i < N; ++i) 
			{
				real[i] = x2[i] * nInv;
				imag[i] = y2[i] * nInv;
			}

			free(x2);
			free(y2);
			return true;
		}

		bool FFT(short int dir, long m, double* x, double* y)
		{
			long n,i,i1,j,k,i2,l,l1,l2;
			double c1,c2,tx,ty,t1,t2,u1,u2,z;

			/* Calculate the number of points */
			n = 1;
			for (i = 0; i < m; i++) 
				n *= 2;

			/* Do the bit reversal */
			i2 = n >> 1;
			j = 0;
			for (i=0;i<n-1;i++) 
			{
				if (i < j) 
				{
					tx = x[i];
					ty = y[i];
					x[i] = x[j];
					y[i] = y[j];
					x[j] = tx;
					y[j] = ty;
				}
				k = i2;
				while (k <= j) 
				{
					j -= k;
					k >>= 1;
				}
				j += k;
			}

			/* Compute the FFT */
			c1 = -1.0; 
			c2 = 0.0;
			l2 = 1;
			for (l=0;l<m;l++) 
			{
				l1 = l2;
				l2 <<= 1;
				u1 = 1.0; 
				u2 = 0.0;
				for (j=0;j<l1;j++) 
				{
					for (i=j;i<n;i+=l2) 
					{
						i1 = i + l1;
						t1 = u1 * x[i1] - u2 * y[i1];
						t2 = u1 * y[i1] + u2 * x[i1];
						x[i1] = x[i] - t1; 
						y[i1] = y[i] - t2;
						x[i] += t1;
						y[i] += t2;
					}
					z =  u1 * c1 - u2 * c2;
					u2 = u1 * c2 + u2 * c1;
					u1 = z;
				}
				c2 = sqrt((1.0 - c1) / 2.0);
				if (dir == 1) 
					c2 = -c2;
				c1 = sqrt((1.0 + c1) / 2.0);
			}

			/* Scaling for forward transform */
			if (dir == 1) 
			{
				for (i=0;i<n;i++) 
				{
					x[i] /= n;
					y[i] /= n;
				}
			}

			return true;
		}

		/*-------------------------------------------------------------------------
		Perform a 2D FFT inplace given a complex 2D array
		The direction dir, 1 for forward, -1 for reverse
		The size of the array (nx,ny)
		Return false if there are memory problems or
		the dimensions are not powers of 2
		*/

		bool FFT2D(Complex* c, int nx, int ny, int dir)
		{
			int x, y;
			int m,twopm;
			double *real,*imag;

			/* Transform the rows */
			real = (double *)malloc(nx * sizeof(double));
			imag = (double *)malloc(nx * sizeof(double));

			if (real == NULL || imag == NULL)
				return false;
			if (!Powerof2(nx, &m, &twopm) || twopm != nx)
				return false;

			for (y = 0; y < ny; ++y) 
			{
				for (x = 0; x < nx; ++x) 
				{
					real[x] = c[y * nx + x].Real;
					imag[x] = c[y * nx + x].Imag;
				}
				FFT(dir, m, real, imag);
				for (x = 0; x < nx; ++x) 
				{
					c[y * nx + x].Real = real[x];
					c[y * nx + x].Imag = imag[x];
				}
			}
			free(real);
			free(imag);

			/* Transform the columns */
			real = (double *)malloc(ny * sizeof(double));
			imag = (double *)malloc(ny * sizeof(double));

			if (real == NULL || imag == NULL)
				return false;
			if (!Powerof2(ny, &m, &twopm) || twopm != ny)
				return false;

			for (x = 0; x < nx; ++x) 
			{
				for (y = 0; y < ny; ++y) 
				{
					real[y] = c[y * nx + x].Real;
					imag[y] = c[y * nx + x].Imag;
				}
				FFT(dir, m, real, imag);
				for (y = 0; y < ny; ++y) 
				{
					c[y * nx + x].Real = real[y];
					c[y * nx + x].Imag = imag[y];
				}
			}
			free(real);
			free(imag);

			return true;
		}

		/*-------------------------------------------------------------------------
		Calculate the closest but lower power of two of a number
		twopm = 2**m <= n
		Return TRUE if 2**m == n
		*/
		bool Powerof2(int n, int *m, int *twopm)
		{
			if (n <= 1) 
			{
				*m = 0;
				*twopm = 1;
				return false;
			}

			*m = 1;
			*twopm = 2;
			do 
			{
				(*m)++;
				(*twopm) *= 2;
			} 
			while (2*(*twopm) <= n);

			if (*twopm != n)
				return false;
			else
				return true;
		}
	}
}