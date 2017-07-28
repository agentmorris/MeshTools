; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=Cobject_loaderDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "object_loader.h"
LastPage=0

ClassCount=3
Class1=Cobject_loaderApp
Class2=Cobject_loaderDlg
Class3=COpenGL

ResourceCount=1
Resource1=IDD_object_loader_DIALOG

[CLS:Cobject_loaderApp]
Type=0
BaseClass=CWinApp
HeaderFile=object_loader.h
ImplementationFile=object_loader.cpp

[CLS:Cobject_loaderDlg]
Type=0
BaseClass=CDialog
HeaderFile=object_loaderDlg.h
ImplementationFile=object_loaderDlg.cpp
Filter=D
VirtualFilter=dWC
LastObject=IDC_STIFFNESS_TEXT

[CLS:COpenGL]
Type=0
BaseClass=CWnd
HeaderFile=OpenGL_dlgwin.h
ImplementationFile=OpenGL_dlgwin.cpp

[DLG:IDD_object_loader_DIALOG]
Type=1
Class=Cobject_loaderDlg
ControlCount=28
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
Control16=IDC_STATIC,static,1342308352
Control17=IDC_STIFFNESS_SLIDER,msctls_trackbar32,1342242840
Control18=IDC_STATICFRICTION_SLIDER,msctls_trackbar32,1342242840
Control19=IDC_DYNAMIC_FRICTION_SLIDER,msctls_trackbar32,1342242840
Control20=IDC_STIFFNESS_TEXT,static,1342308352
Control21=IDC_STATICFRICTION_TEXT,static,1342308352
Control22=IDC_DYNAMIC_FRICTION_RADIUS_TEXT,static,1342308352
Control23=IDC_TOGGLE_STEREO_BUTTON,button,1342242816
Control24=IDC_STEREOFOCUS_SLIDER,msctls_trackbar32,1342242840
Control25=IDC_STEREOFOCUS_TEXT,static,1342308352
Control26=IDC_STEREOSEP_SLIDER,msctls_trackbar32,1342242840
Control27=IDC_STEREOSEP_TEXT,static,1342308352
Control28=IDC_ANIMATION_BUTTON,button,1342242816

