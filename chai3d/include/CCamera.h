//===========================================================================
/*
    This file is part of the CHAI 3D visualization and haptics libraries.
    Copyright (C) 2003-2004 by CHAI 3D. All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License("GPL") version 2
    as published by the Free Software Foundation.

    For using the CHAI 3D libraries with software that can not be combined
    with the GNU GPL, and for taking advantage of the additional benefits
    of our support services, please contact CHAI 3D about acquiring a
    Professional Edition License.

    \author:    <http://www.chai3d.org>
    \author:    Francois Conti
    \author:    Dan Morris
    \version    1.1
    \date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CCameraH
#define CCameraH
//---------------------------------------------------------------------------
#include "CGenericObject.h"
#include "CMaths.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
//---------------------------------------------------------------------------
class cWorld;
//---------------------------------------------------------------------------

// Constants specifying specific stereo rendering frames
const int CHAI_MONO             =  0;
const int CHAI_STEREO_LEFT      = -1;
const int CHAI_STEREO_RIGHT     =  1;

// This constant is used to tell the _viewport_ that he should decide
// which frame(s) to render, and send MONO, LEFT, and/or RIGHT to the
// camera.
const int CHAI_STEREO_DEFAULT   =  -1000;

//---------------------------------------------------------------------------

// The maximum number of arbitrary clip planes
#define CHAI_MAX_CLIP_PLANES 6

// Struct used for enabling arbitrary clipping planes after the viewing
// matrix has been set up
struct cClippingPlane {

  // Is this clip plane enabled?
  //
  //  0 : disabled
  //  1 : enabled
  // -1 : don't touch
  int enabled;

  // The plane equation
  double peqn[4];

  cClippingPlane() {
    enabled = -1;
    peqn[0] = peqn[1] = peqn[2] = peqn[3] = 0.0f;
  }
};

//===========================================================================
/*!
      \file       CCamera.h
      \class      cCamera
      \brief      cCamera describes a virtual Camera located inside the world.
                  Its job in life is to set up the OpenGL projection matrix
                  for the current OpenGL rendering context.  The default camera
                  looks down the negative x-axis.  OpenGL folks may wonder why
                  we chose the negative x-axis... it turns out that's a better
                  representation of the standard conventions used in general
                  robotics.
*/
//===========================================================================
class cCamera : public cGenericObject
{
  friend class cWorld;

  public:

    //! Constructor of cCamera
    cCamera(cWorld* iParent);

    //! Destructor of cCamera
    virtual ~cCamera() {};


    // METHODS:

    //! Get the look/up/right vectors for this camera according to the CHAI conventions
    cVector3d getLookVector()  const { return m_localRot.getCol0(); }
    cVector3d getUpVector()    const { return m_localRot.getCol2(); }
    cVector3d getRightVector() const { return m_localRot.getCol1(); }

    //! Set the position and orientation of the camera
    virtual bool set(const cVector3d& a_localPosition, const cVector3d& a_localLookAt,
             const cVector3d& a_localUp);

    //! Get pointer to parent world
    cWorld* getParentWorld() { return (m_parentWorld); }

    //! Set near and far clipping planes
    void setClippingPlanes(const double a_distanceNear, const double a_distanceFar);

    //! Get near clipping plane
    double getNearClippingPlane() { return (m_distanceNear); }

    //! Get far clipping plane
    double getFarClippingPlane() { return (m_distanceFar); }

    //! Automatically adjust back and front clipping planes
    void adjustClippingPlanes();

    //! Enable or disable one of the (six) arbitrary clipping planes
    void enableClipPlane(const unsigned int& index, const int& enable, const double* peqn=0);

    //! Set field of view angle (in degrees)
    void setFieldViewAngle(double a_fieldViewAngle);

    //! Read field of view angle (in degrees)
    double getFieldViewAngle() { return (m_fieldViewAngle); }

    //! Set stereo focal length
    int setStereoFocalLength(double a_stereoFocalLength);

    //! Get stereo focal length
    double getStereoFocalLength() { return (m_stereoFocalLength); }

    //! Set stereo eye separation
    int setStereoEyeSeparation(double a_stereoEyeSeparation);

    //! Get stereo eye separation
    double getStereoEyeSeparation() { return (m_stereoEyeSeparation); }

    //! Render the camera in OpenGL (i.e. set up the projection matrix)...
    virtual void renderView(const int a_windowWidth, const int a_windowHeight, const int a_imageIndex = CHAI_MONO);

    //! Query whether the specified position is 'pointing at' any objects in the world
    virtual bool select(const int a_windowPosX, const int a_windowPosY,
                const int a_windowWidth, const int a_windowHeight, cGenericObject*& a_selectedObject,
                cTriangle*& a_selectedTriangle, cVector3d& a_selectedPoint, double& a_selectedDistance,
                const bool a_visibleObjectsOnly);

    //! Enable or disable additional rendering passes for transparency (see full comment)
    virtual void enableMultipassTransparency(bool enable);

    //! These are special 'children' of the camera that are rendered independently of
    //! all other objects, intended to contain 2d objects only.  The 'back' scene is
    //! rendered before the 3d objects; the 'front' scene is rendered after the
    //! 3d object.  These are made public variables to allow convenient access to
    //! the scenegraph management functions.
    //!
    //! These objects are rendered through an orthographic projection matrix, so the
    //! positive z axis faces the camera.  Depth is currently not used.  Lighting
    //! is disabled during rendering.
    cGenericObject m_front_2Dscene;
    cGenericObject m_back_2Dscene;

    //! Resets textures and displays for the world associated with this camera
    virtual void onDisplayReset(const bool a_affectChildren = true);

    //! It's useful to store the last projection matrix, for gluProject'ing things
    double m_projectionMatrix[16];

  protected:

    //! Parent world.
    cWorld *m_parentWorld;

    //! Distance to near clipping plane.
    double m_distanceNear;

    //! Distance to far clipping plane.
    double m_distanceFar;

    //! Other clipping planes
    cClippingPlane m_clipPlanes[CHAI_MAX_CLIP_PLANES];

    //! Field of view angle in degrees
    double m_fieldViewAngle;

    // Stereo Parameters:

    //! Focal length
    double m_stereoFocalLength;

    //! Eye separation
    double m_stereoEyeSeparation;

    //! If true, three rendering passes are performed to approximate back-front sorting (see long comment)
    bool m_useMultipassTransparency;

    //! Render a 2d scene within this camera's view.
    void render2dSceneGraph(cGenericObject* a_graph, int a_width, int a_height);

    //! Some apps may have the camera as a child of the world, which
    //! would cause recursion when resetting the display
    bool m_performingDisplayReset;
    
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

