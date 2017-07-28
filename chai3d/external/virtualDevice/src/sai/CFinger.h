//===========================================================================
//  - UNIT -
//
//    Copyright (C) 2002. Stanford University - Robotics Laboratory
/*!
      \author     conti@robotics.stanford.edu
      \file       CFinger.h
      \version    1.0
      \date       01/2002
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CFingerH
#define CFingerH
//---------------------------------------------------------------------------
#include "XMatrix.h"
#include "XColor.h"
//---------------------------------------------------------------------------


//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      cFinger
      \brief      cFinger described a virtual finger inside the world
                  representing the end effector of the haptic device or
                  simply the end tip of your finger. The finger is represented
                  as a small sphere which can be moved in the entire workspace
                  (x,y,z) of the world.
                  Forces can be applied to the virtual finger, for instance when
                  the finger penetrates a shape for instance or is attarcted
                  by a potential field.
*/
//===========================================================================
class cFinger
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cFinger.
     cFinger();
    //! Destructor of cFinger.
    ~cFinger(){};

    // PROPERTIES:
    //! Position of the finger in space.
    xVector3d pos;
    //! Color of the finger.
    xColor4f color;
    //! Radius of the sphere graphically representing the finger.
    double radius;
    //! Force being applied to finger
    xVector3d force;
    //! Enables or disables the graphic rendering of the finger.
    bool enable;

    // METHODS:
    //! Clear the forces applied onto the finger. Set the force to (0,0,0).
    void clearForce();
    //! Add a force vector to the finger.
    void addForce(xVector3d iForce);
    //! Render the shape in OpenGL.
    void render();
};

#endif
//===========================================================================
//  END OF FILE
//===========================================================================
