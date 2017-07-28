//===========================================================================
//  - UNIT -
//
//    Copyright (C) 2002. Stanford University - Robotics Laboratory
/*!
      \author     conti@robotics.stanford.edu
      \file       CShape.cpp
      \version    1.0
      \date       01/2002
*/
//===========================================================================

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "CShape.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include "gl/gl.h"
#include "gl/glu.h"
#include "XMatrix.h"
//---------------------------------------------------------------------------

//===========================================================================
//  - PUBLIC METHOD -
/*!
      Constructor of cShape.

      \fn         cShape::cShape()
      \return     Return a pointer to new shape instance.
*/
//===========================================================================
cShape::cShape()
{
  // INITIALIZE POSITION AND ORIENTATION:
  pos = xSet(0.0, 0.0, 0.0);
  rot = xIdentity33d();

  // INITIALIZE COLOR (GRAY):
  color = xSetColor4f(0.8, 0.8, 0.8);  // Set red-green-blue components.

  // INITIALIZE SPRING CONSTANT:
  kSpring = 100;  // units: [netwons per meter]
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      Constructor of cShapePlan.

      \fn         cShapePlan::cShapePlan()
      \return     Return a pointer to new shape plan instance.
*/
//===========================================================================
cShapePlan::cShapePlan()
{
  // SETUP SPRING CONSTANT:
  kSpring = 500;  // [N/m]
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      Compute forces when finger interacts with plan.

      \fn         xVector3d cShapePlan::computeForce(xVector3d iFingerPos)
      \return     Return computed force.
*/
//===========================================================================
xVector3d cShapePlan::computeForce(xVector3d iFingerPos)
{

}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      Render shape in OpenGl.

      \fn         void cShapePlan::render()
*/
//===========================================================================
void cShapePlan::render()
{
  // SET POSITION AND ORIENTATION OF SHAPE:
  xGLPushMatrixPos(pos);
  xGLPushMatrixRot(rot);

  // SET COLOR PROPERTIES:
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glColor4fv( (const float *)&color);

  // DRAW PLAN:
  float width = 0.6;
  glBegin(GL_POLYGON);
    glNormal3d(0.0, 0.0, 1.0);
    glVertex3d(-width/2, -width/2, 0.0);
    glVertex3d( width/2, -width/2, 0.0);
    glVertex3d( width/2,  width/2, 0.0);
    glVertex3d(-width/2,  width/2, 0.0);
  glEnd();

  // POP OPENGL MATRIX
  xGLPopMatrix();  // pop rotation.
  xGLPopMatrix();  // pop position.
}


//===========================================================================
//  END OF FILE
//===========================================================================
