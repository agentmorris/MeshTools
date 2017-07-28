; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CvoxelizerDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "voxelizer.h"
LastPage=0

ClassCount=3
Class1=COpenGL
Class2=CvoxelizerApp
Class3=CvoxelizerDlg

ResourceCount=1
Resource1=IDD_voxelizer_DIALOG

[CLS:COpenGL]
Type=0
BaseClass=CWnd
HeaderFile=OpenGL_dlgwin.h
ImplementationFile=OpenGL_dlgwin.cpp

[CLS:CvoxelizerApp]
Type=0
BaseClass=CWinApp
HeaderFile=voxelizer.h
ImplementationFile=voxelizer.cpp

[CLS:CvoxelizerDlg]
Type=0
BaseClass=CDialog
HeaderFile=voxelizerDlg.h
ImplementationFile=voxelizerDlg.cpp
Filter=D
VirtualFilter=dWC
LastObject=CvoxelizerDlg

[DLG:IDD_voxelizer_DIALOG]
Type=1
Class=CvoxelizerDlg
ControlCount=20
Control1=IDC_GL_AREA,static,1342181383
Control2=IDC_STATIC,static,1342308352
Control3=IDC_CHECK_WIREFRAME,button,1342242819
Control4=IDC_CHECK_SHOWFRAME,button,1342242819
Control5=IDC_CHECK_SHOWBOX,button,1342242819
Control6=IDC_CHECK_SHOWNORMALS,button,1342242819
Control7=IDC_CHECK_USETEXTURE,button,1342242819
Control8=IDC_CHECK_USECOLORS,button,1342242819
Control9=IDC_CHECK_MATERIAL,button,1342242819
Control10=IDC_LOAD_MODEL_BUTTON,button,1342242816
Control11=IDC_LOAD_TEXTURE_BUTTON,button,1342242816
Control12=IDC_CAMZOOM_SLIDER,msctls_trackbar32,1342242840
Control13=IDC_STATIC,static,1342308352
Control14=IDC_STATIC,static,1342308352
Control15=IDC_TOGGLEHAPTICS_BUTTON,button,1342242816
Control16=IDC_VOXELIZE_BUTTON,button,1342242816
Control17=IDC_QUITVOXELIZING_BUTTON,button,1342242816
Control18=IDC_RENDERPOINTS_CHECK,button,1342242819
Control19=IDC_LOADSUBTRACTOR_BUTTON,button,1342242816
Control20=IDC_LOAD_MODIFIER_BUTTON,button,1342242816

