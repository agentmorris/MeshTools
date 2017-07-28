; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=Cbasic_shapesDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "basic_shapes.h"
LastPage=0

ClassCount=3
Class1=Cbasic_shapesApp
Class2=Cbasic_shapesDlg
Class3=COpenGL

ResourceCount=1
Resource1=IDD_basic_shapes_DIALOG

[CLS:Cbasic_shapesApp]
Type=0
BaseClass=CWinApp
HeaderFile=basic_shapes.h
ImplementationFile=basic_shapes.cpp

[CLS:Cbasic_shapesDlg]
Type=0
BaseClass=CDialog
HeaderFile=basic_shapesDlg.h
ImplementationFile=basic_shapesDlg.cpp
Filter=D
VirtualFilter=dWC
LastObject=Cbasic_shapesDlg

[CLS:COpenGL]
Type=0
BaseClass=CWnd
HeaderFile=OpenGL_dlgwin.h
ImplementationFile=OpenGL_dlgwin.cpp

[DLG:IDD_basic_shapes_DIALOG]
Type=1
Class=Cbasic_shapesDlg
ControlCount=5
Control1=IDC_GL_AREA,static,1342181383
Control2=IDC_CAMZOOM_SLIDER,msctls_trackbar32,1342242840
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_TOGGLEHAPTICS_BUTTON,button,1342242816

