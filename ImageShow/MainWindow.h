#pragma once

#include <Image.h>
#include <Fourier.h>

namespace ImgOps 
{
	public ref class MainWindow : public System::Windows::Forms::Form
	{
	private:
		Image* _image;
		Image* _magnitudeImage;
		Image* _phaseImage;
		FourierData* _fourier;

		// Buttons panel:
		System::Windows::Forms::Panel^  _butsPanels;
		System::Windows::Forms::Button^  _butLoad;
		System::Windows::Forms::Button^  _butSave;
		System::Windows::Forms::Button^  _butFourier;

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
	private: System::Windows::Forms::Button^  _butFourierExp;

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
		void FourierTransform(bool scaleLog);
		void ResetFourier();

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
			this->_butFourier = (gcnew System::Windows::Forms::Button());
			this->_butFourierExp = (gcnew System::Windows::Forms::Button());
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
			this->_butLoad->Text = L"Load Image";
			this->_butLoad->UseVisualStyleBackColor = true;
			this->_butLoad->Click += gcnew System::EventHandler(this, &MainWindow::_butLoad_Click);
			// 
			// _butSave
			// 
			this->_butSave->Location = System::Drawing::Point(4, 45);
			this->_butSave->Name = L"_butSave";
			this->_butSave->Size = System::Drawing::Size(108, 32);
			this->_butSave->TabIndex = 2;
			this->_butSave->Text = L"Save Image";
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
			this->_tabImage->Text = L"Image";
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
			this->_butsPanels->Controls->Add(this->_butFourierExp);
			this->_butsPanels->Controls->Add(this->_butFourier);
			this->_butsPanels->Controls->Add(this->_butLoad);
			this->_butsPanels->Controls->Add(this->_butSave);
			this->_butsPanels->Location = System::Drawing::Point(666, 34);
			this->_butsPanels->Name = L"_butsPanels";
			this->_butsPanels->Size = System::Drawing::Size(115, 155);
			this->_butsPanels->TabIndex = 0;
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
			// _butFourierExp
			// 
			this->_butFourierExp->Location = System::Drawing::Point(4, 119);
			this->_butFourierExp->Name = L"_butFourierLog";
			this->_butFourierExp->Size = System::Drawing::Size(108, 30);
			this->_butFourierExp->TabIndex = 4;
			this->_butFourierExp->Text = L"Fourier Log";
			this->_butFourierExp->UseVisualStyleBackColor = true;
			this->_butFourierExp->Click += gcnew System::EventHandler(this, &MainWindow::_butFourierLog_Click);
			// 
			// MainWindow
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(800, 560);
			this->MaximumSize = System::Drawing::Size(800, 560);
			this->MinimumSize = System::Drawing::Size(800, 560);
			this->Controls->Add(this->_butsPanels);
			this->Controls->Add(this->_tabsImages);
			this->Name = L"MainWindow";
			this->ShowIcon = false;
			this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
			this->Text = L"Image Show";
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
