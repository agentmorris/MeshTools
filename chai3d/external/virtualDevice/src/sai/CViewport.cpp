//===========================================================================
//  - UNIT -
//
//    Copyright (C) 2002. Stanford University - Robotics Laboratory
/*!
      \author     conti@robotics.stanford.edu
      \file       CViewport.cpp
      \version    1.0
      \date       01/2002
*/
//===========================================================================

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include "CViewport.h"
#include "CWorld.h"
#include "CCamera.h"
#include "XMatrix.h"
//---------------------------------------------------------------------------

//===========================================================================
// - PUBLIC METHOD -
/*!
      Constructor of cCamera.

      \fn         cViewport::cViewport(TWinControl *iWinControl)
      \param      iWinControl   Panel or window display in which rendering occurs.
      \return     Return a pointer to new viewport instance.
*/
//===========================================================================
cViewport::cViewport(TWinControl *iWinControl)
{
  // INITIALIZATION:
  camera = NULL;
  world = NULL;

  // DECLARATION:
  int formatIndex;

  // GET HANDLE OF WINCONTROL:
  winControl = iWinControl;
  wHandle = iWinControl->Handle;

  // SET PIXEL FORMAT DESCRIPTOR:
  PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
	1,                     // version number
	PFD_DRAW_TO_WINDOW |   // support window
	PFD_SUPPORT_OPENGL |   // support OpenGL
	PFD_DOUBLEBUFFER,      // double buffered
	PFD_TYPE_RGBA,         // RGBA type
	32,                    // 24-bit color depth
	0, 0, 0, 0, 0, 0,      // color bits ignored
	0,                     // no alpha buffer
	0,                     // shift bit ignored
        0,                     // no accumulation buffer
	0, 0, 0, 0,            // accum bits ignored
	32,                    // 32-bit z-buffer
	0,                     // no stencil buffer
	0,                     // no auxiliary buffer
	PFD_MAIN_PLANE,        // main layer
	0,                     // reserved
	0, 0, 0                // layer masks ignored
  };

  // RETRIEVE THE HANDLE OF THE DISPLAY DEVICE CONTEXT:
  glDC = GetDC(wHandle);

  // FIND THE PIXEL FORMAT SUPPORTED BY THE DEVICE CONTEXT:
  formatIndex = ChoosePixelFormat(glDC, &pfd);
  if (formatIndex == 0)
  {
    throw exViewportError();
  }

  // SETS THE SPECIFIED DEVICE CONTEXT'S PIXEL FORMAT:
  if (!SetPixelFormat(glDC, formatIndex, &pfd))
  {
    throw exViewportError();
  }

  // OPEN GL IS READY FOR RENDERING:
  glReady = true;

  // CREATE DISPLAY CONTEXT:
  glContext = wglCreateContext(glDC);
  if (glContext == 0)
  {
    throw exViewportError();
  }
  // ENABLE VIEWPORT:
  enabled = true;
}


//===========================================================================
// - PUBLIC METHOD -
/*!
        Destructor of cViewport.

        \fn     cViewport::~cViewport()
*/
//===========================================================================
cViewport::~cViewport()
{
  // DELETE DISPLAY CONTEXT:
  wglDeleteContext(glContext);

  // FINALIZATION:
  glReady = false;
  ReleaseDC(wHandle, glDC);
}


//===========================================================================
// - PUBLIC METHOD -
/*!
        Call this method to render the OpenGL scene.

        \fn     void cViewport::render()
*/
//===========================================================================
void cViewport::render()
{
  bool Error;
  if (glReady && enabled && (camera != NULL) && (world != NULL))
  {
    // INIT:
    Error = false;  // No Error detected yet.

    // ACTIVATE DISPLAY CONTEXT:
    if (!wglMakeCurrent(glDC, glContext))
    {
      Error = true; // Error Detected.
      throw exViewportError();
    }

    //---------------------------------------------------------------------//
    // RENDER WORLD:
    //---------------------------------------------------------------------//
    if (!Error)  // If no Error detected then Render World.
    {
      // SET VIEWPORT:
      glViewport(0, 0, winControl->Width, winControl->Height);

      // RENDER WORLD:
      world->render(camera, winControl->Width, winControl->Height);

      // FLUSH OPEN GL:
      glFlush();

      // SWAP BUFFERS:
      SwapBuffers(glDC);;
    }

    //---------------------------------------------------------------------//
    // DESACTIVATE DISPLAY CONTEXT:
    //---------------------------------------------------------------------//
    wglMakeCurrent(glDC, 0);

  }
}


//===========================================================================
// END OF FILE
//===========================================================================

