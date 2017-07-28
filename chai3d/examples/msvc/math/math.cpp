//===========================================================================
/*
    This file is part of the CHAI 3D visualization and haptics libraries.
    Copyright (C) 2003-2004 by CHAI 3D. All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License("GPL") version 2
    as published by the Free Software Foundation.

    For using the CHAI 3D libraries with software that can not be combined
    with the GNU GPL, and for taking advantage of the additional benefits
    of our support services, please contact CHAI 3D about acquiring a
    Professional Edition License.

    \author:    <http://www.chai3d.org>
    \author:    Chris Sewell
    \version    1.0
    \date       06/2004
*/
//===========================================================================

#include "stdafx.h"
#include "math.h"
#include "mathDlg.h"
#include <conio.h>
#include <process.h>

#ifndef M_PI
#define M_PI 3.1415926535898
#endif

#define ALLOCATE_SCOPED_GLOBALS
#include "math_globals.h"
#undef ALLOCATE_SCOPED_GLOBALS

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Turn off annoying compiler warnings
#pragma warning(disable: 4305)
#pragma warning(disable: 4800)

BEGIN_MESSAGE_MAP(CmathApp, CWinApp)
	//{{AFX_MSG_MAP(CmathApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// No one really knows why GetConsoleWindow() is not
// properly exported by the MS include files.
extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow (); 

CmathApp theApp;

CmathApp::CmathApp() {

  /*
  AllocConsole();

  HWND con_wnd = GetConsoleWindow();

  // We want the console to pop up over any non-CHAI windows, although
  // we'll put the main dialog window above it.
  ::SetForegroundWindow(con_wnd);
  
  SetWindowPos(con_wnd,HWND_TOP,0,0,0,0,SWP_NOSIZE);
  */

  g_main_app = this;
}


void CmathApp::uninitialize() {

  
}


BOOL CmathApp::InitInstance() {

	AfxEnableControlContainer();

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	g_main_dlg = new CmathDlg;
  m_pMainWnd = g_main_dlg;

  g_main_dlg->Create(IDD_math_DIALOG,NULL);

  return TRUE;
}


int CmathApp::Run() {

  MSG msg;

  /*
  // Position the console window so it looks nice...
  HWND con_wnd = GetConsoleWindow();
  RECT r;
  ::GetWindowRect(con_wnd,&r);

  // Move the real window up to the front
  ::SetWindowPos(g_main_dlg->m_hWnd,HWND_TOP,0,r.bottom,0,0,SWP_NOSIZE);  
  ::SetForegroundWindow(g_main_dlg->m_hWnd);
  */

  // Loop forever looking for messages and rendering...
  while (1) {

    while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) == TRUE) {

      if (GetMessage(&msg, NULL, 0, 0)) {

        TranslateMessage(&msg);

        if (msg.message == WM_KEYDOWN) {

        }

        else if (msg.message == WM_KEYUP) {
        
        }

        DispatchMessage(&msg);

      }

      // Quit if GetMessage(...) fails
      else return TRUE;

    }    
   
  }
  
} // Run()



