; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CsoundsDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "sounds.h"
LastPage=0

ClassCount=3
Class1=CsoundsApp
Class2=CsoundsDlg
Class3=COpenGL

ResourceCount=1
Resource1=IDD_sounds_DIALOG

[CLS:CsoundsApp]
Type=0
BaseClass=CWinApp
HeaderFile=sounds.h
ImplementationFile=sounds.cpp

[CLS:CsoundsDlg]
Type=0
BaseClass=CDialog
HeaderFile=soundsDlg.h
ImplementationFile=soundsDlg.cpp
Filter=D
VirtualFilter=dWC
LastObject=CsoundsDlg

[CLS:COpenGL]
Type=0
BaseClass=CWnd
HeaderFile=OpenGL_dlgwin.h
ImplementationFile=OpenGL_dlgwin.cpp

[DLG:IDD_sounds_DIALOG]
Type=1
Class=CsoundsDlg
ControlCount=5
Control1=IDC_GL_AREA,static,1342181383
Control2=IDC_CAMZOOM_SLIDER,msctls_trackbar32,1342242840
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_TOGGLEHAPTICS_BUTTON,button,1342242816

