; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=Crecord_playerDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "record_player.h"
LastPage=0

ClassCount=3
Class1=Crecord_playerApp
Class2=Crecord_playerDlg
Class3=COpenGL

ResourceCount=1
Resource1=IDD_record_player_DIALOG

[CLS:Crecord_playerApp]
Type=0
BaseClass=CWinApp
HeaderFile=record_player.h
ImplementationFile=record_player.cpp

[CLS:Crecord_playerDlg]
Type=0
BaseClass=CDialog
HeaderFile=record_playerDlg.h
ImplementationFile=record_playerDlg.cpp
Filter=D
VirtualFilter=dWC
LastObject=IDC_COMBO1

[CLS:COpenGL]
Type=0
BaseClass=CWnd
HeaderFile=OpenGL_dlgwin.h
ImplementationFile=OpenGL_dlgwin.cpp

[DLG:IDD_record_player_DIALOG]
Type=1
Class=Crecord_playerDlg
ControlCount=6
Control1=IDC_GL_AREA,static,1342181383
Control2=IDC_CAMZOOM_SLIDER,msctls_trackbar32,1342242840
Control3=IDC_STATIC1,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_TOGGLEHAPTICS_BUTTON,button,1342242816
Control6=IDC_RECORDCOMBO,combobox,1344340227

