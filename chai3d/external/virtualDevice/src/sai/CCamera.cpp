//===========================================================================
//  - UNIT -
//
//    Copyright (C) 2002. Stanford University - Robotics Laboratory
/*!
      \author     conti@robotics.stanford.edu
      \file       CCamera.cpp
      \version    1.0
      \date       01/2002
*/
//===========================================================================

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "CCamera.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include "gl/gl.h"
#include "gl/glu.h"
#include "XMatrix.h"
//---------------------------------------------------------------------------

//===========================================================================
//  - PUBLIC METHOD -
/*!
      Once a camera is created. position and oriente the camera by calling
      method: Set(Position, LookAt, Up). Initialy the camera is located at
      position (1,0,0) and is looking at origin (0,0,0) with vector Up (0,0,1).

      \fn         cCamera::cCamera()
      \return     Return a pointer to new camera instance.
*/
//===========================================================================
cCamera::cCamera()
{
  // SET CLIPPING PLANE:
  distNear        = 0.01;       // Distance to Near clipping plane.
  distFar         = 1000;       // Distance to Far clipping plane.
  fieldViewAngle  = 46;        // Field of view angle (degrees).

  // POSITION AND ORIENTATION OF CAMERA:
  set(xSet(1,0,0), xSet(0,0,0), xSet(0,0,1));

}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      Destructor of cCamera.

      \fn         cCamera::~cCamera()
*/
//===========================================================================
cCamera::~cCamera()
{
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      Set the Position and Orientation of the Camera. Three vectors are
      required: [iPosition] which describes the position in world coordinates
      of the camera.
      [iLookAt] which describes a point where the camera is looking at.
      Finally [iUp] to oriente the camera arround its rolling axis. [iUp]
      always points to the top of the image.

      \fn         void cCamera::set(const xVector3d &iPosition, const xVector3d
                                     &iLookAt, const xVector3d &iUp)
      \param      iPosition    Position of camera in world coordinates.
      \param      iLookAt      Center of image. Point in space focused by camera.
      \param      iUp          Vector giving the rolling orientation. If camera
                               is horitontal iUp=(0,0,1)
      \return     Return a pointer to new mass entity.
*/
//===========================================================================
void cCamera::set(const xVector3d &iPosition, const xVector3d &iLookAt, const xVector3d &iUp)
{
  pos = iPosition;
  lookAt = iLookAt;
  up = iUp;
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      Render the camera in OpenGL. User should not call this function
      directly. To render an image, call viewport->render().

      \fn         cCamera::render(int iWindowWidth, int iWindowHeight)
      \param      iWinWidth     Width of Viewport.
      \param      iWinHeight    Height of Viewport.
*/
//===========================================================================
void cCamera::render(int iWindowWidth, int iWindowHeight)
{
  double glAspect;

  // SET MODELVIEW:
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity;

  // REDEFINE THE VISIBLE VOLUME AND THE VIEWPORT:
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity;

  // PERSPECTIVE PROJECTION:
  glAspect = ((double)iWindowWidth / (double)iWindowHeight);
  gluPerspective(fieldViewAngle,  // Field of View Angle.
                 glAspect,        // Aspect ratio of viewing volume.
                 distNear,        // Distance to Near clipping plane.
                 distFar);        // Distance to Far clipping plane.

  // SET POSITION & ORIENTATION: (EYE, CENTER, UP)
  gluLookAt( pos.x,     pos.y,     pos.z,
             lookAt.x,  lookAt.y,  lookAt.z,
             up.x,      up.y,      up.z );

}


//===========================================================================
//  END OF FILE
//===========================================================================
