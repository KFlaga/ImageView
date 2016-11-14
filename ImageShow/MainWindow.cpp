#include "MainWindow.h"

#include <Decoder.h>
#include <Encoder.h>
#include <msclr/marshal.h>
#include "ImageConverter.h"
#include <FileStream.h>

namespace ImgOps
{
	MainWindow::MainWindow(void)
	{
		InitializeComponent();
		_image = 0;
		_magnitudeImage = 0;
		_phaseImage = 0;
		_fourier = 0;

		_publicKey = new RSA::RSAKey();
		_privateKey = new RSA::RSAKey();

		_tabPhase->Enabled = false;
	}

	MainWindow::~MainWindow()
	{
		if (components)
		{
			delete components;
		}
		if(_image != 0)
			delete _image;
		if(_magnitudeImage != 0)
			delete _magnitudeImage;
		if(_phaseImage != 0)
			delete _phaseImage;
		if(_fourier != 0)
			delete _fourier;

		if(_publicKey != 0)
			delete _publicKey;	
		if(_privateKey != 0)
			delete _privateKey;
	}

	void MainWindow::SetNewImage(Image* image)
	{
		if(image != 0)
		{	
			if(_image != 0)
			{
				delete _image;
			}
			_image = image;

			// We have valid image, so copy it into picture box
			ImageConverter converter;
			_frameworkImage = converter.ConvertImage(_image);
			_picBoxImage->Image = _frameworkImage;

			ResetFourier();
			_tabsImages->SelectedIndex = 0;
			_butFourier->Enabled = true;
		}
	}

	System::Void MainWindow::_butLoad_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		System::Windows::Forms::OpenFileDialog fileDialog;
		fileDialog.DefaultExt = "png";
		fileDialog.Filter = "Png Images|*.png";
		auto res = fileDialog.ShowDialog();
		if(res == System::Windows::Forms::DialogResult::OK)
		{
			if(fileDialog.FileName->EndsWith(".png"))
			{
				ImageDecoder* decoder = CreatePNGDecoder();
				msclr::interop::marshal_context ctx;
				const char* filePath = ctx.marshal_as<const char*>(fileDialog.FileName);
				Image* img = decoder->ReadImageFromFile(filePath);
				if(img != 0)
				{	
					SetNewImage(img);
				}
				else
					System::Windows::Forms::MessageBox::Show("Blad przy dekodowaniu obrazu");

				delete decoder;
			}
			else
				System::Windows::Forms::MessageBox::Show("Nieobslugiwany format pliku");
		}
	}

	System::Void MainWindow::_butSave_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		if(_image != 0)
		{	
			System::Windows::Forms::SaveFileDialog fileDialog;
			fileDialog.DefaultExt = "png";
			fileDialog.AddExtension = true;
			fileDialog.Filter = "Png Images|*.png";
			auto res = fileDialog.ShowDialog();
			if(res == System::Windows::Forms::DialogResult::OK)
			{
				if(fileDialog.FileName->EndsWith(".png"))
				{
					ImageEncoder* encoder = CreatePNGEncoder();
					msclr::interop::marshal_context ctx;
					const char* filePath = ctx.marshal_as<const char*>(fileDialog.FileName);
					bool res = encoder->SaveImageToFile(filePath, _image);
					if(res == true)
					{ }
					else
						System::Windows::Forms::MessageBox::Show("Blad przy kodowaniu obrazu");

					delete encoder;
				}
				else
					System::Windows::Forms::MessageBox::Show("Nieobslugiwany format pliku");
			}
		}
	}

	System::Void MainWindow::_butFourier_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		if(_image != 0)
		{
			FourierTransform(false);
		}
	}

	System::Void MainWindow::_butFourierLog_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		if(_image != 0)
		{
			FourierTransform(true);
		}
	}

	void MainWindow::FourierTransform(bool scaleLog)
	{
		ResetFourier();
		_fourier = Fourier::FourierTransform(_image);
		if(_fourier != 0)
		{
			_magnitudeImage = Fourier::GetMagnitudeImage(_fourier, scaleLog);
			_phaseImage = Fourier::GetPhaseImage(_fourier);

			ImageConverter converter;
			_frameworkMagnitude = converter.ConvertImage(_magnitudeImage);
			_picBoxMagnitude->Image = _frameworkMagnitude;
			_frameworkPhase = converter.ConvertImage(_phaseImage);
			_picBoxPhase->Image = _frameworkPhase;
		}
	}

	void MainWindow::ResetFourier()
	{
		if(_fourier != 0) delete _fourier;
		if(_magnitudeImage != 0) delete _magnitudeImage;
		if(_phaseImage != 0) delete _phaseImage;

		_fourier = 0;
		_magnitudeImage = 0;
		_phaseImage = 0;

		_picBoxMagnitude->Image = nullptr;
		_picBoxPhase->Image = nullptr;
	}

	System::Void MainWindow::_butKeyGen_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		RSA::GenerateRSAKeys(_publicKey, _privateKey);
	}

	bool MainWindow::ReadKeyFromFile(const char* filePath, RSA::RSAKey* key)
	{
		// File should consist of 2 times WordByteCount bytes
		// First is key->n, second is key->e
		FileStream fileStream(filePath, OpenModes::Read);
		if(fileStream.IsOpen() == false)
			return false;

		byte buf[RSA::WordByteCount * 2];

		int64 read = fileStream.ReadSome(RSA::WordByteCount, buf);
		if(read != RSA::WordByteCount)
			return false;
		RSA::ByteArrayToWord(buf, read, &key->n);

		read = fileStream.ReadSome(RSA::WordByteCount, buf);
		if(read != RSA::WordByteCount)
			return false;
		RSA::ByteArrayToWord(buf, read, &key->e);

		return true;
	}

	bool MainWindow::WriteKeyToFile(const char* filePath, RSA::RSAKey* key)
	{
		// File should consist of 2 times WordByteCount bytes
		// First is key->n, second is key->e
		FileStream fileStream(filePath, OpenModes::WriteTrunc);
		if(fileStream.IsOpen() == false)
			return false;

		byte buf[RSA::WordByteCount];
		RSA::WordToByteArray_FullWord(buf, &key->n);
		int64 writen = fileStream.WriteSome(RSA::WordByteCount, buf);
		if(writen != RSA::WordByteCount)
			return false;

		RSA::WordToByteArray_FullWord(buf, &key->e);
		writen = fileStream.WriteSome(RSA::WordByteCount, buf);
		if(writen != RSA::WordByteCount)
			return false;

		return true;
	}

	System::Void MainWindow::_butKeyLoad_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		System::Windows::Forms::OpenFileDialog fileDialog;
		fileDialog.Title = "Wczytaj Klucz Publiczny";
		fileDialog.DefaultExt = "key";
		fileDialog.Filter = "RSA Key Files|*.key";
		auto res = fileDialog.ShowDialog();
		if(res == System::Windows::Forms::DialogResult::OK)
		{
			if(fileDialog.FileName->EndsWith(".key"))
			{
				msclr::interop::marshal_context ctx;
				const char* filePath = ctx.marshal_as<const char*>(fileDialog.FileName);
				if(ReadKeyFromFile(filePath, _publicKey) == false)
					System::Windows::Forms::MessageBox::Show("Blad przy wczytywaniu klucza publicznego");
			}
			else
				System::Windows::Forms::MessageBox::Show("Nieobslugiwany format pliku");
		}
		else
			return;

		fileDialog.Title = "Wczytaj Klucz Prywatny";
		res = fileDialog.ShowDialog();
		if(res == System::Windows::Forms::DialogResult::OK)
		{
			if(fileDialog.FileName->EndsWith(".key"))
			{
				msclr::interop::marshal_context ctx;
				const char* filePath = ctx.marshal_as<const char*>(fileDialog.FileName);
				if(ReadKeyFromFile(filePath, _privateKey) == false)
					System::Windows::Forms::MessageBox::Show("Blad przy wczytywaniu klucza prywatnego");
			}
			else
				System::Windows::Forms::MessageBox::Show("Nieobslugiwany format pliku");
		}
		else
			return;
		System::Windows::Forms::MessageBox::Show("Klucze odczytane pomyslnie");
	}

	System::Void MainWindow::_butKeySave_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		System::Windows::Forms::SaveFileDialog fileDialog;
		fileDialog.AddExtension = true;
		fileDialog.DefaultExt = "key";
		fileDialog.Filter = "RSA Key Files|*.key";
		fileDialog.Title = "Zapisz Klucz Publiczny";
		auto res = fileDialog.ShowDialog();
		if(res == System::Windows::Forms::DialogResult::OK)
		{
			if(fileDialog.FileName->EndsWith(".key"))
			{
				ImageEncoder* encoder = CreatePNGEncoder();
				msclr::interop::marshal_context ctx;
				const char* filePath = ctx.marshal_as<const char*>(fileDialog.FileName);
				if(WriteKeyToFile(filePath, _publicKey) == false)
					System::Windows::Forms::MessageBox::Show("Blad przy zapisywaniu klucza publicznego");			
			}
			else
				System::Windows::Forms::MessageBox::Show("Nieobslugiwany format pliku");
		}
		else 
			return;

		fileDialog.Title = "Zapisz Klucz Prywatny";
		res = fileDialog.ShowDialog();
		if(res == System::Windows::Forms::DialogResult::OK)
		{
			if(fileDialog.FileName->EndsWith(".key"))
			{
				ImageEncoder* encoder = CreatePNGEncoder();
				msclr::interop::marshal_context ctx;
				const char* filePath = ctx.marshal_as<const char*>(fileDialog.FileName);
				if(WriteKeyToFile(filePath, _privateKey) == false)
					System::Windows::Forms::MessageBox::Show("Blad przy zapisywaniu klucza prywatnego");		
			}
			else
				System::Windows::Forms::MessageBox::Show("Nieobslugiwany format pliku");
		}
		else 
			return;

		System::Windows::Forms::MessageBox::Show("Klucze zapisane pomyslnie");
	}

	System::Void MainWindow::_butEncrypt_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		if(_image != 0)
		{
			// Encrypted image : encrypt whole image pixel data in chunks
			// Knowing that WordByteCount is multiple of 8, let chunkSize be
			// if pixSize == 1 then chunkSize = WordByteCount / 2,
			// if pixSize == 2 then chunkSize = WordByteCount / 2,
			// if pixSize == 3 then chunkSize = WordByteCount * 3 / 4,
			// if pixSize == 4 then chunkSize = WordByteCount / 2
			uint32 chunkSize = RSA::WordByteCount / 2;
			if( _image->PixelSize() == 3 || _image->PixelSize() == 6 )
				chunkSize = RSA::WordByteCount * 3 / 4;

			uint64 imgSize  = _image->PixelSize() * _image->Width() * _image->Height();

			uint32 lastChunkSize;
			uint64 encryptedLength;
			byte* encryptedData;
			RSA::EncryptMessage_FixedChunks(_image->Data(), imgSize, 
				chunkSize, &lastChunkSize, &encryptedData, &encryptedLength, _publicKey);

			// As encrypted length is larger than base data length, we need to enlarge image
			// to show / encode it. Stored format is Rgba32,
			// For pixSize != 3 for each base pixel we have 2 pixels in encoded image
			// For pixSize for each pixel we have one pixel, but with different format
			uint32 encWidth = (_image->PixelSize() == 3 || _image->PixelSize() == 6) ? _image->Width() : _image->Width() * 2;
			uint32 encHeight = _image->Height();
			Image* encryptedImage = new Image(encWidth, encHeight, PixelFormats::Rgba32, encryptedData);
			encryptedImage->SetDecryptedFormat(_image->PixFormat());
			encryptedImage->SetDecryptedLastChunkSize(lastChunkSize);

			SetNewImage(encryptedImage);
		}
	}

	System::Void MainWindow::_butDecrypt_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		if(_image != 0)
		{
			// Encrypted image is in format Rgba32, but decrypted format may differ
			// According to decrypted format, set chunk size / pixel format / img width etc
			if(_image->IsDecrypted() == false)
			{
				// Image was not encrypted (at least not here)
				// But we can decrypt it anyway
				// If width is even, then let decrypted format be Rgba32 otherwise it will be Rgb24
				_image->SetDecryptedFormat(
					_image->Width() % 2 == 0 ? 
					PixelFormats::Rgba32 : PixelFormats::Rgb24);
				_image->SetDecryptedLastChunkSize(0);
			}

			uint32 decPixSize = PixelFormats::GetPixelSize(_image->GetDecryptedFormat());
			uint32 decHeight = _image->Height();
			uint32 decWidth = (decPixSize == 3 || decPixSize == 6) ? _image->Width() : _image->Width() / 2;

			uint32 chunkSize = RSA::WordByteCount / 2;
			if( decPixSize == 3 || decPixSize == 6 )
				chunkSize = RSA::WordByteCount * 3 / 4;

			uint64 imgSize  = _image->PixelSize() * _image->Width() * _image->Height();

			uint64 decryptedLength;
			byte* decryptedData;
			RSA::DecryptMessage_FixedChunks(_image->Data(), imgSize, 
				chunkSize, _image->GetDecryptedLastChunkSize(), &decryptedData, &decryptedLength, _privateKey);

			Image* decryptedImage = new Image(decWidth, decHeight, PixelFormats::Rgba32, decryptedData);
			decryptedImage->SetDecryptedFormat(PixelFormats::Unknown);

			SetNewImage(decryptedImage);
		}
	}
}
