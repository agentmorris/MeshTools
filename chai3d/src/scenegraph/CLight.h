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
	\version    1.1
	\date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CLightH
#define CLightH
//---------------------------------------------------------------------------
#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include "CColor.h"
#include "CMaths.h"
#include "CMacrosGL.h"
#include "CGenericObject.h"
//---------------------------------------------------------------------------
class cWorld;
//---------------------------------------------------------------------------


//===========================================================================
/*!
	  \file       CLight.h
	  \class      cLight
	  \brief      cLight describes an OpenGL light source, generally rendered
				  by a cWorld object, which is typically the top of a scene
				  graph.

				  By default, lights are directional and non-spot.
*/
//===========================================================================
class cLight : public cGenericObject
{

	// cWorld needs to access openGL information stored privately in cLight
	friend class cWorld;

public:
	// CONSTRUCTOR & DESTRUCTOR:

	//! Constructor of cLight.
	cLight(cWorld* a_world = 0);
	//! Destructor of cLight.
	~cLight();


	// MEMBERS:

	//! Ambient light component.
	cColorf m_ambient;
	//! Diffuse light component.
	cColorf m_diffuse;
	//! Specular light component.
	cColorf m_specular;


	// METHODS: LIGHT PROPERTIES

	//! Set the direction of the light beam... only affects _positional_ lights with angular cutoffs (spotlights)
	void setDir(const cVector3d& a_direction);
	//! Set the direction of the light beam... only affects _positional_ lights with angular cutoffs (spotlights)
	void setDir(const double a_x, const double a_y, const double a_z);
	//! Read the direction of the light beam... only affects _positional_ lights with angular cutoffs (spotlights)
	cVector3d getDir() const { return (m_localRot.getCol0()); }

	//! Set this light to be purely directional (true) or purely positional (false)
	void setDirectionalLight(bool a_directionalLight) { m_directionalLight = a_directionalLight; }
	//! Returns true for a directional light, false for a positional light
	bool getDirectionalLight() { return m_directionalLight; }

	//! Set my constant attenuation parameter
	void setAttConstant(const GLfloat& a_value) { m_attConstant = cClamp(a_value, 0.0f, 1.0f); }
	//! Read my constant attenuation parameter
	GLfloat getAttConstant() const { return (m_attConstant); }
	//! Set my linear attenuation parameter
	void setAttLinear(const GLfloat& a_value) { m_attLinear = cClamp(a_value, 0.0f, 1.0f); }
	//! Read my linear attenuation parameter
	GLfloat getAttLinear() const { return (m_attLinear); }
	//! Set my quadratic attenuation parameter
	void setAttQuadratic(const GLfloat& a_value) { m_attQuadratic = cClamp(a_value, 0.0f, 1.0f); }
	//! Read my quadratic attenuation parameter
	GLfloat getAttQuadratic() const { return (m_attQuadratic); }
	//! Set concentration level of the light
	void setSpotExponent(const GLfloat& a_value) { m_spotExponent = cClamp(a_value, 0.0f, 100.0f); }
	//! Read concentration level of the light.
	GLfloat getSpotExponent() const { return (m_spotExponent); }

	//! Set the cutoff angle in degrees (only affects spotlights) (positional lights with angular cutoffs)
	void setCutOffAngle(const GLfloat& a_value);
	//! Read Cut off angle.
	GLfloat getCutOffAngle() const { return (m_cutOffAngle); }

	//! Enable or disable this light source
	void setEnabled(const bool& a_enabled) { m_enabled = a_enabled; }
	//! Is this light source enabled?
	bool getEnabled() const { return (m_enabled); }

	//! GL reference number of the light (0-7)
	GLint m_glLightNumber;

	// METHODS:
	//! Render the light in OpenGL.
	void renderLightSource();

protected:
	// MEMBERS:

	//! Parent world
	cWorld* m_parentWorld;
	//! Constant attenuation parameter.
	GLfloat m_attConstant;
	//! Linear attenuation parameter.
	GLfloat m_attLinear;
	//! Quadratic attenuation parameter.
	GLfloat m_attQuadratic;
	//! Concentration of the light.
	GLfloat m_spotExponent;
	//! Cut off angle (for spot lights only). Only values in the range [0, 90], and the special value 180, are accepted
	GLfloat m_cutOffAngle;
	//! Enable light source (on/off)
	bool m_enabled;
	//! Is this a directional (true) or positional (false) light
	bool m_directionalLight;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

