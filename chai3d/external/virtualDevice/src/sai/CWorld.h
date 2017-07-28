//===========================================================================
//  - UNIT -
//
//    Copyright (C) 2002. Stanford University - Robotics Laboratory
/*!
      \author     conti@robotics.stanford.edu
      \file       CWorld.cpp
      \version    1.0
      \date       01/2002
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CWorldH
#define CWorldH
//---------------------------------------------------------------------------
#include "XColor.h"
#include "CCamera.h"
#include "CLight.h"
#include "CFinger.h"
//---------------------------------------------------------------------------

//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      cWorld
      \brief      cWorld class contains the entire virtual environment.
                  including 8 lights and cameras.
*/
//===========================================================================
class cWorld
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cWorld
    cWorld();
    //! Destructor of cWorld
    ~cWorld();

    // PROPERTIES:
    //! Backround color. Initial setting is black.
    xColor4f backgroundColor;
    //! Lighting mode (ON / OFF)
    bool useLighting;
    //! Array of 8 OpenGL light sources.
    cLight *lights[8];
    //! Virtual finger representing the end effector of a haptic device.
    cFinger *finger;
    //! List of shapes inside the world.
    TList *shapes;

    // METHODS:
    //! render the world in OpenGL.
    void render(cCamera *iCamera,
                int iWindowWidth, int iWindowHeight);
    //! Compute forces and send values to Phantom haptic device.
    void renderForces();

  private:
};
#endif

//===========================================================================
// END OF FILE
//===========================================================================
