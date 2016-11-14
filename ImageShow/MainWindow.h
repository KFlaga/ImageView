#pragma once

#include <Image.h>
#include <Fourier.h>
#include <RSA.h>

namespace ImgOps 
{
	public ref class MainWindow : public System::Windows::Forms::Form
	{
	private:
		Image* _image;
		Image* _magnitudeImage;
		Image* _phaseImage;
		FourierData* _fourier;

		RSA::RSAKey* _publicKey;
		RSA::RSAKey* _privateKey;

		// Buttons panel:
		System::Windows::Forms::Panel^  _butsPanels;
		System::Windows::Forms::Button^  _butLoad;
		System::Windows::Forms::Button^  _butSave;
		System::Windows::Forms::Button^  _butFourier;
		System::Windows::Forms::Button^  _butFourierExp;
		System::Windows::Forms::Button^  _butKeyGen;
		System::Windows::Forms::Button^  _butKeyLoad;
		System::Windows::Forms::Button^  _butKeySave;
		System::Windows::Forms::Button^  _butEncrypt;
		System::Windows::Forms::Button^  _butDecrypt;

		// Images tab control:
		System::Windows::Forms::TabControl^  _tabsImages;
		System::Windows::Forms::PictureBox^  _picBoxImage;
		System::Windows::Forms::PictureBox^  _picBoxMagnitude;
		System::Windows::Forms::PictureBox^  _picBoxPhase;
		System::Windows::Forms::TabPage^  _tabMagnitude;
		System::Windows::Forms::TabPage^  _tabPhase;
		System::Windows::Forms::TabPage^  _tabImage;

		// Converted images:
		System::Drawing::Bitmap^ _frameworkImage;
		System::Drawing::Bitmap^ _frameworkMagnitude;
		System::Drawing::Bitmap^ _frameworkPhase;

		System::ComponentModel::Container ^components;

	public:
		MainWindow(void);

	protected:
		~MainWindow();

	private: 
		System::Void _butLoad_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void _butSave_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void _butFourier_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void _butFourierLog_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void _butKeyGen_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void _butKeyLoad_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void _butKeySave_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void _butEncrypt_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void _butDecrypt_Click(System::Object^  sender, System::EventArgs^  e);
		
		void SetNewImage(Image* image);
		void FourierTransform(bool scaleLog);
		void ResetFourier();
		bool ReadKeyFromFile(const char* filePath, RSA::RSAKey* key);
		bool WriteKeyToFile(const char* filePath, RSA::RSAKey* key);

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->_butLoad = (gcnew System::Windows::Forms::Button());
			this->_butSave = (gcnew System::Windows::Forms::Button());
			this->_tabsImages = (gcnew System::Windows::Forms::TabControl());
			this->_tabImage = (gcnew System::Windows::Forms::TabPage());
			this->_picBoxImage = (gcnew System::Windows::Forms::PictureBox());
			this->_tabMagnitude = (gcnew System::Windows::Forms::TabPage());
			this->_picBoxMagnitude = (gcnew System::Windows::Forms::PictureBox());
			this->_tabPhase = (gcnew System::Windows::Forms::TabPage());
			this->_picBoxPhase = (gcnew System::Windows::Forms::PictureBox());
			this->_butsPanels = (gcnew System::Windows::Forms::Panel());
			this->_butKeyGen = (gcnew System::Windows::Forms::Button());
			this->_butFourierExp = (gcnew System::Windows::Forms::Button());
			this->_butFourier = (gcnew System::Windows::Forms::Button());
			this->_butKeySave = (gcnew System::Windows::Forms::Button());
			this->_butKeyLoad = (gcnew System::Windows::Forms::Button());
			this->_butEncrypt = (gcnew System::Windows::Forms::Button());
			this->_butDecrypt = (gcnew System::Windows::Forms::Button());
			this->_tabsImages->SuspendLayout();
			this->_tabImage->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->_picBoxImage))->BeginInit();
			this->_tabMagnitude->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->_picBoxMagnitude))->BeginInit();
			this->_tabPhase->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->_picBoxPhase))->BeginInit();
			this->_butsPanels->SuspendLayout();
			this->SuspendLayout();
			// 
			// _butLoad
			// 
			this->_butLoad->Location = System::Drawing::Point(4, 6);
			this->_butLoad->Name = L"_butLoad";
			this->_butLoad->Size = System::Drawing::Size(108, 33);
			this->_butLoad->TabIndex = 1;
			this->_butLoad->Text = L"Wczytaj Obraz";
			this->_butLoad->UseVisualStyleBackColor = true;
			this->_butLoad->Click += gcnew System::EventHandler(this, &MainWindow::_butLoad_Click);
			// 
			// _butSave
			// 
			this->_butSave->Location = System::Drawing::Point(4, 45);
			this->_butSave->Name = L"_butSave";
			this->_butSave->Size = System::Drawing::Size(108, 32);
			this->_butSave->TabIndex = 2;
			this->_butSave->Text = L"Zapisz Obraz";
			this->_butSave->UseVisualStyleBackColor = true;
			this->_butSave->Click += gcnew System::EventHandler(this, &MainWindow::_butSave_Click);
			// 
			// _tabsImages
			// 
			this->_tabsImages->Controls->Add(this->_tabImage);
			this->_tabsImages->Controls->Add(this->_tabMagnitude);
			this->_tabsImages->Controls->Add(this->_tabPhase);
			this->_tabsImages->Location = System::Drawing::Point(12, 12);
			this->_tabsImages->Name = L"_tabsImages";
			this->_tabsImages->SelectedIndex = 0;
			this->_tabsImages->Size = System::Drawing::Size(648, 506);
			this->_tabsImages->TabIndex = 4;
			// 
			// _tabImage
			// 
			this->_tabImage->Controls->Add(this->_picBoxImage);
			this->_tabImage->Location = System::Drawing::Point(4, 22);
			this->_tabImage->Name = L"_tabImage";
			this->_tabImage->Padding = System::Windows::Forms::Padding(3);
			this->_tabImage->Size = System::Drawing::Size(640, 480);
			this->_tabImage->TabIndex = 0;
			this->_tabImage->Text = L"Obraz";
			this->_tabImage->UseVisualStyleBackColor = true;
			// 
			// _picBoxImage
			// 
			this->_picBoxImage->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->_picBoxImage->Dock = System::Windows::Forms::DockStyle::Fill;
			this->_picBoxImage->Location = System::Drawing::Point(3, 3);
			this->_picBoxImage->Name = L"_picBoxImage";
			this->_picBoxImage->Size = System::Drawing::Size(634, 474);
			this->_picBoxImage->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
			this->_picBoxImage->TabIndex = 1;
			this->_picBoxImage->TabStop = false;
			// 
			// _tabMagnitude
			// 
			this->_tabMagnitude->Controls->Add(this->_picBoxMagnitude);
			this->_tabMagnitude->Location = System::Drawing::Point(4, 22);
			this->_tabMagnitude->Name = L"_tabMagnitude";
			this->_tabMagnitude->Size = System::Drawing::Size(640, 480);
			this->_tabMagnitude->TabIndex = 1;
			this->_tabMagnitude->Text = L"Fourier Magnitude";
			this->_tabMagnitude->UseVisualStyleBackColor = true;
			// 
			// _picBoxMagnitude
			// 
			this->_picBoxMagnitude->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->_picBoxMagnitude->Dock = System::Windows::Forms::DockStyle::Fill;
			this->_picBoxMagnitude->Location = System::Drawing::Point(0, 0);
			this->_picBoxMagnitude->Name = L"_picBoxMagnitude";
			this->_picBoxMagnitude->Size = System::Drawing::Size(640, 480);
			this->_picBoxMagnitude->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
			this->_picBoxMagnitude->TabIndex = 1;
			this->_picBoxMagnitude->TabStop = false;
			// 
			// _tabPhase
			// 
			this->_tabPhase->Controls->Add(this->_picBoxPhase);
			this->_tabPhase->Location = System::Drawing::Point(4, 22);
			this->_tabPhase->Name = L"_tabPhase";
			this->_tabPhase->Size = System::Drawing::Size(640, 480);
			this->_tabPhase->TabIndex = 2;
			this->_tabPhase->Text = L"Fourier Phase";
			this->_tabPhase->UseVisualStyleBackColor = true;
			// 
			// _picBoxPhase
			// 
			this->_picBoxPhase->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->_picBoxPhase->Dock = System::Windows::Forms::DockStyle::Fill;
			this->_picBoxPhase->Location = System::Drawing::Point(0, 0);
			this->_picBoxPhase->Name = L"_picBoxPhase";
			this->_picBoxPhase->Size = System::Drawing::Size(640, 480);
			this->_picBoxPhase->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
			this->_picBoxPhase->TabIndex = 1;
			this->_picBoxPhase->TabStop = false;
			// 
			// _butsPanels
			// 
			this->_butsPanels->Controls->Add(this->_butDecrypt);
			this->_butsPanels->Controls->Add(this->_butEncrypt);
			this->_butsPanels->Controls->Add(this->_butKeyLoad);
			this->_butsPanels->Controls->Add(this->_butKeySave);
			this->_butsPanels->Controls->Add(this->_butKeyGen);
			this->_butsPanels->Controls->Add(this->_butFourierExp);
			this->_butsPanels->Controls->Add(this->_butFourier);
			this->_butsPanels->Controls->Add(this->_butLoad);
			this->_butsPanels->Controls->Add(this->_butSave);
			this->_butsPanels->Location = System::Drawing::Point(666, 34);
			this->_butsPanels->Name = L"_butsPanels";
			this->_butsPanels->Size = System::Drawing::Size(115, 373);
			this->_butsPanels->TabIndex = 0;
			// 
			// _butKeyGen
			// 
			this->_butKeyGen->Location = System::Drawing::Point(3, 155);
			this->_butKeyGen->Name = L"_butKeyGen";
			this->_butKeyGen->Size = System::Drawing::Size(108, 36);
			this->_butKeyGen->TabIndex = 5;
			this->_butKeyGen->Text = L"Generuj Klucze RSA";
			this->_butKeyGen->UseVisualStyleBackColor = true;
			this->_butKeyGen->Click += gcnew System::EventHandler(this, &MainWindow::_butKeyGen_Click);
			// 
			// _butFourierExp
			// 
			this->_butFourierExp->Location = System::Drawing::Point(4, 119);
			this->_butFourierExp->Name = L"_butFourierExp";
			this->_butFourierExp->Size = System::Drawing::Size(108, 30);
			this->_butFourierExp->TabIndex = 4;
			this->_butFourierExp->Text = L"Fourier Log";
			this->_butFourierExp->UseVisualStyleBackColor = true;
			this->_butFourierExp->Click += gcnew System::EventHandler(this, &MainWindow::_butFourierLog_Click);
			// 
			// _butFourier
			// 
			this->_butFourier->Location = System::Drawing::Point(4, 83);
			this->_butFourier->Name = L"_butFourier";
			this->_butFourier->Size = System::Drawing::Size(108, 30);
			this->_butFourier->TabIndex = 3;
			this->_butFourier->Text = L"Fourier";
			this->_butFourier->UseVisualStyleBackColor = true;
			this->_butFourier->Click += gcnew System::EventHandler(this, &MainWindow::_butFourier_Click);
			// 
			// _butKeySave
			// 
			this->_butKeySave->Location = System::Drawing::Point(4, 197);
			this->_butKeySave->Name = L"_butKeySave";
			this->_butKeySave->Size = System::Drawing::Size(108, 38);
			this->_butKeySave->TabIndex = 6;
			this->_butKeySave->Text = L"Zapisz Klucze RSA";
			this->_butKeySave->UseVisualStyleBackColor = true;
			this->_butKeySave->Click += gcnew System::EventHandler(this, &MainWindow::_butKeySave_Click);
			// 
			// _butKeyLoad
			// 
			this->_butKeyLoad->Location = System::Drawing::Point(4, 241);
			this->_butKeyLoad->Name = L"_butKeyLoad";
			this->_butKeyLoad->Size = System::Drawing::Size(108, 38);
			this->_butKeyLoad->TabIndex = 7;
			this->_butKeyLoad->Text = L"Wczytaj Klucze RSA";
			this->_butKeyLoad->UseVisualStyleBackColor = true;
			this->_butKeyLoad->Click += gcnew System::EventHandler(this, &MainWindow::_butKeyLoad_Click);
			// 
			// _butEncrypt
			// 
			this->_butEncrypt->Location = System::Drawing::Point(4, 285);
			this->_butEncrypt->Name = L"_butEncrypt";
			this->_butEncrypt->Size = System::Drawing::Size(108, 38);
			this->_butEncrypt->TabIndex = 8;
			this->_butEncrypt->Text = L"Zaszyfruj Obraz";
			this->_butEncrypt->UseVisualStyleBackColor = true;
			this->_butEncrypt->Click += gcnew System::EventHandler(this, &MainWindow::_butEncrypt_Click);
			// 
			// _butDecrypt
			// 
			this->_butDecrypt->Location = System::Drawing::Point(4, 329);
			this->_butDecrypt->Name = L"_butDecrypt";
			this->_butDecrypt->Size = System::Drawing::Size(108, 38);
			this->_butDecrypt->TabIndex = 9;
			this->_butDecrypt->Text = L"Rozszyfruj Obraz";
			this->_butDecrypt->UseVisualStyleBackColor = true;
			this->_butDecrypt->Click += gcnew System::EventHandler(this, &MainWindow::_butDecrypt_Click);
			// 
			// MainWindow
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(784, 521);
			this->Controls->Add(this->_butsPanels);
			this->Controls->Add(this->_tabsImages);
			this->MaximumSize = System::Drawing::Size(800, 560);
			this->MinimumSize = System::Drawing::Size(800, 560);
			this->Name = L"MainWindow";
			this->ShowIcon = false;
			this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
			this->Text = L"Image Ops";
			this->_tabsImages->ResumeLayout(false);
			this->_tabImage->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->_picBoxImage))->EndInit();
			this->_tabMagnitude->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->_picBoxMagnitude))->EndInit();
			this->_tabPhase->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->_picBoxPhase))->EndInit();
			this->_butsPanels->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion
	};
}
