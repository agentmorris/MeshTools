#include "stdafx.h"
#include "Form1.h"
#include <windows.h>

using namespace tri_tri_collisions;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	System::Threading::Thread::CurrentThread->ApartmentState = System::Threading::ApartmentState::STA;
	Application::Run(gcnew Form1());
	return 0;
}
