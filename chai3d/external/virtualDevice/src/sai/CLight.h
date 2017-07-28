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
#ifndef CLightH
#define CLightH
//---------------------------------------------------------------------------
#include "gl/gl.h"
#include "xColor.h"
#include "xMatrix.h"
//---------------------------------------------------------------------------

//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      cLight
      \brief      cLight describes an OpenGL virtual spot light.
*/
//===========================================================================
class cLight
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cLight.
    cLight(const GLint &iLightNumber);
    //! Destructor of cLight.
    ~cLight();

    // POSITION AND DIRECTION:
    //! Position of light source in space.
    xVector3d pos;
    //! Direction of spot light.
    xVector3d dir;

    // COLORS
    //! Ambient light component.
    xColor4f ambient;
    //! Diffuse light component.
    xColor4f diffuse;
    //! Specular light component.
    xColor4f specular;

    // ATTENUATION:
    //! Constant attenuation parameter.
    GLint attConstant;
    //! Linear attenuation parameter.
    GLint attLinear;
    //! Quadratic attenuation parameter.
    GLint attQuadratic;

    // INTENSITY:
    //! Concentration of Light.
    GLint spotExponent;

    // OPTIONS:
    //! Cut Off Angle (for Spot light);
    GLint cutOffAngle;
    //! Enable light source (on/off)
    bool enabled;

    // METHODS:
    //! Render light in OpenGL.
    void render();

  protected:
    //! Identity number of the light (1-8) for OpenGL use only.
    GLint GLLightNumber;
};

#endif

//===========================================================================
// END OF FILE
//===========================================================================
