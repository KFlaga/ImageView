#include "MainWindow.h"

#include <Decoder.h>
#include <Encoder.h>
#include <msclr/marshal.h>
#include "ImageConverter.h"

namespace ImgOps
{
	MainWindow::MainWindow(void)
	{
		InitializeComponent();
		_image = 0;
		_magnitudeImage = 0;
		_phaseImage = 0;
		_fourier = 0;

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
					if(_image != 0)
					{
						delete _image;
					}
					_image = img;

					// We have valid image, so copy it into picture box
					ImageConverter converter;
					_frameworkImage = converter.ConvertImage(_image);
					_picBoxImage->Image = _frameworkImage;

					ResetFourier();
					_tabsImages->SelectedIndex = 0;
					_butFourier->Enabled = true;
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
}
