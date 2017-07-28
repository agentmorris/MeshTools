//===========================================================================
//  - UNIT -
//
//    Copyright (C) 2002. Stanford University - Robotics Laboratory
/*!
      \author     conti@robotics.stanford.edu
      \file       CShape.h
      \version    1.0
      \date       01/2002
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CShapeH
#define CShapeH
//---------------------------------------------------------------------------
#include "XMatrix.h"
#include "XColor.h"
//---------------------------------------------------------------------------


//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      cShape
      \brief
*/
//===========================================================================
class cShape
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cCamera.
     cShape();
    //! Destructor of cCamera.
    ~cShape(){};

    // PROPERTIES:
    //! Position in space.
    xVector3d pos;
    //! Orientation in space:
    xMatrix33d rot;
    //! Main color of shape.
    xColor4f color;
    //! Spring constant for force rendering. units: [netwons per meter]
    double kSpring;

    // METHODS:
    //! compute the force, given the position of the finger
    virtual xVector3d computeForce(xVector3d iFingerPos) { return (xSet(0.0, 0.0, 0.0)); };
    //! Render the shape in OpenGL.
    virtual void render() {};
};


//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      cShapePlan
      \brief      cShapePlan describes a simple surface plan. Graphic and
                  force rendering methods are implemented.
*/
//===========================================================================
class cShapePlan  : public cShape
{
  public:
    // CONSTRUCTOR & DESCTRUCTOR:
    //! Constructor of cShapePlan.
    cShapePlan();
    // METHODS:
    //! compute the force, given the position of the finger
    virtual xVector3d computeForce(xVector3d iFingerPos);
    //! Render the shape in OpenGL.
    virtual void render();
};

#endif
//===========================================================================
//  END OF FILE
//===========================================================================
