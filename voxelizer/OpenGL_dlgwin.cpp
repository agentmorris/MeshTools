// OpenGL.cpp : implementation file
//

#include "stdafx.h"
#include "OpenGL_dlgwin.h"
#include <gl/gl.h>
#include <gl/glu.h>
#include <conio.h>
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

COpenGL::COpenGL() {

}


COpenGL::~COpenGL() {
	wglMakeCurrent(NULL, NULL);
	::ReleaseDC(m_hWnd, hdc);
	wglDeleteContext(hglrc);
	DestroyWindow();

}


BEGIN_MESSAGE_MAP(COpenGL, CWnd)
	//{{AFX_MSG_MAP(COpenGL)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	//ON_WM_KEYDOWN()
	  // ON_WM_PAINT()
	  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void COpenGL::OnClose() {

	CWnd::OnClose();
}


int MySetPixelFormat(HDC hdc) {
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd 
		1,                                // version number 
	  0
		| PFD_DRAW_TO_WINDOW              // support window 
		| PFD_SUPPORT_OPENGL              // support OpenGL 
	  // | PFD_STEREO
	  // | PFD_SUPPORT_GDI
	  | PFD_DOUBLEBUFFER               // double buffered 
	  ,
		PFD_TYPE_RGBA,                    // RGBA type 
		24,                               // 24-bit color depth 
		0, 0, 0, 0, 0, 0,                 // color bits ignored 
		0,                                // no alpha buffer 
		0,                                // shift bit ignored 
		0,                                // no accumulation buffer 
		0, 0, 0, 0,                       // accum bits ignored 
		16,                               // 16-bit z-buffer     
		0,                                // no stencil buffer 
		0,                                // no auxiliary buffer 
		PFD_MAIN_PLANE,                   // main layer 
		0,                                // reserved 
		0, 0, 0                           // layer masks ignored 
	};

	int  iPixelFormat;

	// get the device context's best, available pixel format match 
	if ((iPixelFormat = ChoosePixelFormat(hdc, &pfd)) == 0) {
		_cprintf("Error: ChoosePixelFormat Failed\n");
		return -1;
	}

	// make that match the device context's current pixel format 
	if (SetPixelFormat(hdc, iPixelFormat, &pfd) == FALSE) {
		_cprintf("Error: SetPixelFormat Failed\n");
		return -1;
	}

	return 0;
}

int COpenGL::OnCreate(LPCREATESTRUCT lpCreateStruct) {

	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (MySetPixelFormat(::GetDC(m_hWnd)) == -1) return -1;

	glClearColor(0, 0, 0, 0);
	glColor3f(1, 1, 1);

	hdc = ::GetDC(m_hWnd);
	hglrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hglrc);

	return 0;
}

