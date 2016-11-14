#include "MainWindow.h"
#include <windows.h>

#include <RSA.h>


[System::STAThreadAttribute]
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
{
	System::Threading::Thread::CurrentThread->ApartmentState = System::Threading::ApartmentState::STA;

	System::Windows::Forms::Application::Run(gcnew ImgOps::MainWindow());
	return 0;
}
