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
#ifndef CCameraH
#define CCameraH
//---------------------------------------------------------------------------
#include "XMatrix.h"
//---------------------------------------------------------------------------

//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      cCamera
      \brief      cCamera describes a virtual Camera located in a world.
*/
//===========================================================================
class cCamera
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cCamera.
    cCamera();
    //! Destructor of cCamera.
    ~cCamera();

    // PROPERTIES:
    //! Distance to Near clipping plane.
    double distNear;
    //! Distance to Far clipping plane.
    double distFar;
    //! Field of view. Angle in degrees.
    double fieldViewAngle;
    //! Position in space of camera.
    xVector3d pos;
    //! Focus point of the camera. (Point in space where the camera is looking at.)
    xVector3d lookAt;
    //! Up vector of camera.
    xVector3d up;

    // METHODS:
    //! Set the position and orientation of the camera.
    void set(const xVector3d &iPosition, const xVector3d &iLookAt, const xVector3d &iUp);
    //! Render the camera in OpenGL.
    void render(int iWindowWidth, int iWindowHeight);
};

#endif
//===========================================================================
//  END OF FILE
//===========================================================================
