//===========================================================================
//  - UNIT -
//
//    Copyright (C) 2002. Stanford University - Robotics Laboratory
/*!
      \author     conti@robotics.stanford.edu
      \file       CFinger.cpp
      \version    1.0
      \date       01/2002
*/
//===========================================================================

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "CFinger.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include "gl/gl.h"
#include "gl/glu.h"
#include "XMatrix.h"
//---------------------------------------------------------------------------

//===========================================================================
//  - PUBLIC METHOD -
/*!
      Constructor of cFinger.

      \fn         cFinger::cFinger()
      \return     Return a pointer to new finger instance.
*/
//===========================================================================
cFinger::cFinger()
{
  // INITIALIZE POSITION OF FINGER:
  pos = xSet(0.0, 0.0, 0.0);

  // INITIALIZE RADIUS:
  radius = 0.003;

  // INITIALIZE COLOR (RED DEFAULT COLOR):
  color = xSetColor4f(1.0, 0.0, 0.0);  // Set red-green-blue components.

  // CLEAR FORCES:
  clearForce();

  // ENABLE FINGER FOR GRAPHIC RENDERING:
  enable = true;
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      Clear all forces applied onto the finger.

      \fn         void cFinger::clearForce()
*/
//===========================================================================
void cFinger::clearForce()
{
  force = xSet(0.0, 0.0, 0.0);
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      Add a force vector to the finger.

      \fn         void cFinger::addForce(xVector3d iForce)
      \param      iForce   Force vector to apply to finger.
*/
//===========================================================================
void cFinger::addForce(xVector3d iForce)
{
  force = xAdd(force, iForce);
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      Render the finger in OpenGl.

      \fn         void cFinger::render()
*/
//===========================================================================
void cFinger::render()
{
  if (enable)
  {
    // SET POSITION AND ORIENTATION OF SHAPE:
    xGLPushMatrixPos(pos);

    // SET COLOR PROPERTIES:
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glColor4fv( (const float *)&color);

    // DRAW SPHERE:
    GLUquadricObj *quadObj;
    quadObj = gluNewQuadric ();
    gluQuadricDrawStyle (quadObj, GLU_FILL);
    gluQuadricNormals (quadObj, GLU_SMOOTH);
    gluSphere(quadObj, radius, 10, 10);
    gluDeleteQuadric(quadObj);

    // POP OPENGL MATRIX
    xGLPopMatrix();  // pop position.

    // DRAW FORCE:
    xColor4f forceColor = xSetColor4f(1.0,1.0,0.0);
    glColor4fv( (const float *)&forceColor);
    xVector3d forcePos = xAdd(pos, xMul(0.05, force));
    glBegin(GL_LINES);
      glVertex3dv((const double *)&pos);
      glVertex3dv((const double *)&forcePos);
    glEnd();
  }
}


//===========================================================================
//  END OF FILE
//===========================================================================
