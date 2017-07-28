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
#ifndef CViewportH
#define CViewportH
//---------------------------------------------------------------------------
#include "gl/gl.h"
#include "CWorld.h"
#include "CCamera.h"
#include "XMatrix.h"
//---------------------------------------------------------------------------

//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      cViewport
      \brief      cViewport describes a display for rendering openGl scenes.
*/
//===========================================================================
class cViewport
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cViewport.
    cViewport(TWinControl *iWinControl);
    //! Destructor of cViewport.
    ~cViewport();

    // METHODS:
    //! Render the OpenGL scene.
    void render();

    // PROPERTIES:
    //! Virtual camera located in the world connected to viewport.
    cCamera *camera;
    //! World displayed by the viewport.
    cWorld *world;


  protected:
    // DISPLAY CONTEXT:
    TWinControl *winControl;
    HWND wHandle;
    HGLRC glContext;
    HDC glDC;
    GLenum glErrorCode;
    bool glReady;
    bool enabled;
};

#endif

//===========================================================================
// END OF FILE
//===========================================================================
