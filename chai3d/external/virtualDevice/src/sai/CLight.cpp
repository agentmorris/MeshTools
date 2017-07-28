//===========================================================================
//  - UNIT -
//
//    Copyright (C) 2002. Stanford University - Robotics Laboratory
/*!
      \author     conti@robotics.stanford.edu
      \file       CLight.cpp
      \version    1.0
      \date       01/2002
*/
//===========================================================================

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "CLight.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//===========================================================================
// - PUBLIC METHOD -
/*!
      When creating an OpenGL light source, an identity number between 0 and 7
      is attributed to the light source. This number is used during the
      rendering process. Lights are automatically created in class cWorld.

      \fn       cLight::cLight(const GLint &iLightNumber)
      \param    iLightNumber    OpenGL Identity number between 0 and 7.
      \return   pointer to new cLight instance.
*/
//===========================================================================
cLight::cLight(const GLint &iLightNumber)
{
  // SET THE CORRESPONDING OPEN GL LIGHT ID:
  switch ( iLightNumber )
  {
    case 0 : GLLightNumber = GL_LIGHT0; break;
    case 1 : GLLightNumber = GL_LIGHT1; break;
    case 2 : GLLightNumber = GL_LIGHT2; break;
    case 3 : GLLightNumber = GL_LIGHT3; break;
    case 4 : GLLightNumber = GL_LIGHT4; break;
    case 5 : GLLightNumber = GL_LIGHT5; break;
    case 6 : GLLightNumber = GL_LIGHT6; break;
    case 7 : GLLightNumber = GL_LIGHT7; break;
  }

  // SET POSITION & DIRECTION:
  pos = xSet(0.0, 0.0, 1.0);
  dir = xSet(0.0, 0.0, -1.0);

  // INIT COLOR:
  ambient  = xSetColor4f(0.4, 0.4, 0.4, 1.0);
  diffuse  = xSetColor4f(0.7, 0.7, 0.7, 1.0);
  specular = xSetColor4f(0.1, 0.1, 0.1, 1.0);

  // CUTOFF ANGLE:
  cutOffAngle = 180;

  // ATTENUATION:              ATT = 1 / (Kc + Kl*d + Kq*d^2)
  attConstant = 1.7;        // Attenuation Constant.
  attLinear = 1.0;          // Attenuation Linear.
  attQuadratic = 1.0;       // Attenuation Quadratic.

  // SPOT EXPONENT:
  spotExponent = 100.0;     // Repartition of Light in CutOff Angle (GAUSSIAN)

  // DISABLE LIGHT:
  enabled = false;
}


//===========================================================================
// - PUBLIC METHOD -
/*!
      Destructor of cLight.

      \fn       cLight::~cLight()
*/
//===========================================================================
cLight::~cLight()
{
  // DISABLE LIGHT:
  glDisable(GLLightNumber);
  enabled = false;
}


//===========================================================================
// - PUBLIC METHOD -
/*!
      Render the light source in OpenGL.

      \fn       void cLight::render()
*/
//===========================================================================
void cLight::render()
{

  if (enabled == true)
  {
    // ENABLE LIGHT:
    glEnable(GLLightNumber);

    // SET LIGHT COMPONENTS:
    glLightfv(GLLightNumber, GL_AMBIENT, (const float *)&ambient )  ;
    glLightfv(GLLightNumber, GL_DIFFUSE, (const float *)&diffuse);
    glLightfv(GLLightNumber, GL_SPECULAR, (const float *)&specular);

    // SET LIGHT MODEL:
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

    // POSITION LIGHT IN SPACE:
    glPushMatrix;

    float Pos4D[4];
    Pos4D[0] = (float)pos.x;
    Pos4D[1] = (float)pos.y;
    Pos4D[2] = (float)pos.z;
    Pos4D[3] = 0;
    glLightfv(GLLightNumber, GL_POSITION, (const float *)&Pos4D);

    glPopMatrix;

    //CUTOFF ANGLE:
    glLightf(GLLightNumber, GL_SPOT_CUTOFF, cutOffAngle);

    // DIRECTION OF LIGHT BEAM:
    float Dir4D[4];
    Dir4D[0] = (float)dir.x;
    Dir4D[1] = (float)dir.y;
    Dir4D[2] = (float)dir.z;
    Dir4D[3] = 0;
    glLightfv(GLLightNumber, GL_SPOT_DIRECTION, (const float *)&Dir4D);

    // SET ATTENUATIONS:
    glLightf(GLLightNumber, GL_CONSTANT_ATTENUATION, attConstant);
    glLightf(GLLightNumber, GL_LINEAR_ATTENUATION, attLinear);
    glLightf(GLLightNumber, GL_QUADRATIC_ATTENUATION, attQuadratic);

    // SET SPOT EXPONENT:
    glLightf(GLLightNumber, GL_SPOT_EXPONENT, spotExponent);
  }
  else
  {
    glDisable(GLLightNumber);
  };
}


//===========================================================================
// END OF FILE
//===========================================================================
