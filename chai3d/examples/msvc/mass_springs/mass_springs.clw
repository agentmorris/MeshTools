; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=Cmass_springsDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "mass_springs.h"
LastPage=0

ClassCount=3
Class1=Cmass_springsApp
Class2=Cmass_springsDlg
Class3=COpenGL

ResourceCount=1
Resource1=IDD_mass_springs_DIALOG

[CLS:Cmass_springsApp]
Type=0
BaseClass=CWinApp
HeaderFile=mass_springs.h
ImplementationFile=mass_springs.cpp

[CLS:Cmass_springsDlg]
Type=0
BaseClass=CDialog
HeaderFile=mass_springsDlg.h
ImplementationFile=mass_springsDlg.cpp
Filter=D
VirtualFilter=dWC
LastObject=Cmass_springsDlg

[CLS:COpenGL]
Type=0
BaseClass=CWnd
HeaderFile=OpenGL_dlgwin.h
ImplementationFile=OpenGL_dlgwin.cpp

[DLG:IDD_mass_springs_DIALOG]
Type=1
Class=Cmass_springsDlg
ControlCount=7
Control1=IDC_GL_AREA,static,1342181383
Control2=IDC_CAMZOOM_SLIDER,msctls_trackbar32,1342242840
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC_1,static,1342308352
Control5=IDC_TOGGLEHAPTICS_BUTTON,button,1342242816
Control6=IDC_ADDBALL_BUTTON,button,1342242816
Control7=IDC_REMOVEBALL_BUTTON,button,1342242816

