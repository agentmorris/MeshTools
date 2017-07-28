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
#include "CLight.h"
#include "CMacrosGL.h"
#include "CWorld.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	  When creating an OpenGL light source, an identity number between 0 and 7
	  is attributed to the light. This number is used during the
	  rendering process. Lights are automatically created inside class cWorld.

	  \fn       cLight::cLight(cWorld* a_world)
	  \param    a_world parent world in which light source will be located
*/
//===========================================================================
cLight::cLight(cWorld* a_world)
{
	// read light number in current world
	m_parentWorld = a_world;

	// Uninitialized state...
	m_glLightNumber = 9999;

	// The GL light number will be modified here...
	if (m_parentWorld != NULL)
	{
		m_parentWorld->addLightSource(this);
	}

	// set default color parameters
	m_ambient.set(0.4f, 0.4f, 0.4f, 1.0f);
	m_diffuse.set(0.7f, 0.7f, 0.7f, 1.0f);
	m_specular.set(0.5f, 0.5f, 0.5f, 1.0f);

	// set default cutoff angle (no cutoff by default)
	m_cutOffAngle = 180.0;

	// set default attenuation parameters  ATT = 1 / (Kc + Kl*d + Kq*d^2)

	// Attenuation Constant.
	m_attConstant = 1.0;

	// Attenuation Linear.
	m_attLinear = 0.0;

	// Attenuation Quadratic.
	m_attQuadratic = 0.0;

	// set default spot exponent
	m_spotExponent = 100.0;

	// light sources are disable by default
	m_enabled = false;

	// lights are purely directional by default
	m_directionalLight = true;

}


//===========================================================================
/*!
	  Destructor of cLight.

	  Disables this light in OpenGL and removes it from the parent's list
	  of lights.

	  \fn       cLight::~cLight()
*/
//===========================================================================
cLight::~cLight()
{
	// disable light source
	// glDisable(m_glLightNumber);
	// m_enabled = false;
	if (m_parentWorld) m_parentWorld->removeLightSource(this);
}


//===========================================================================
/*!
	  Set the direction of my light beam.

	  \fn       void cLight::setDir(const cVector3d& a_direction)
	  \param    a_direction  Direction of light beam
*/
//===========================================================================
void cLight::setDir(const cVector3d& a_direction)
{

	// We arbitrarily point lights along the x axis of the stored
	// rotation matrix... this allows matrix transformations
	// to apply to lights.
	// m_localRot.setCol0(a_direction);

	cVector3d c0, c1, c2, t;
	a_direction.copyto(c0);

	// check vector
	if (c0.lengthsq() < 0.0001) { return; }

	// normalize direction vector
	c0.normalize();

	// compute 2 vector perpendicular to a_direction
	t.set(a_direction.y, a_direction.z, a_direction.x);
	t.crossr(c0, c1);
	c1.crossr(c0, c2);

	// c0.negate();
	c1.negate();
	c2.negate();

	// update rotation matrix
	m_localRot.setCol(c0, c1, c2);

}


//===========================================================================
/*!
	  Set the direction of my light beam.

	  \fn       void cLight::setDir(const double a_x, const double a_y,
				const double a_z)
	  \param    a_x   X Coordinate of light beam.
	  \param    a_y   Y Coordinate of light beam.
	  \param    a_z   Z Coordinate of light beam.
*/
//===========================================================================
void cLight::setDir(const double a_x, const double a_y, const double a_z)
{
	setDir(cVector3d(a_x, a_y, a_z));
}


//===========================================================================
/*!
	  Set the cutoff angle (in degrees) of my light beam.

	  Should range  between 0 and 90 for spot lights.  Use 180.0 to specify
	  that there should be no cutoff.

	  \fn       void cLight::setCutOffAngle(const GLfloat& a_value)
	  \param    a_value  Cutoff angle of light beam.
*/
//===========================================================================
void cLight::setCutOffAngle(const GLfloat& a_value)
{
	// temp variable
	GLfloat t_newAngle;

	// check if a negative value was given in which case light is set to
	// a non spot configuration
	if (a_value < 0) { t_newAngle = 180.0; }

	// check if angle is equal to 180.0 degrees. This corresponds to a non spot light.
	else if (a_value == 180.0) { t_newAngle = 180.0; }

	// check if value ranges between 0 and 90. This corresponds to a spot light.
	else if ((a_value >= 0.0) && (a_value <= 90.0)) { t_newAngle = a_value; }

	// value is incorrect. Light is set to a non-spot light configuration.
	else { t_newAngle = 180.0; }

	// assign new value
	m_cutOffAngle = t_newAngle;
}


//===========================================================================
/*!
	  Render this light source in OpenGL.

	  Note that if this light source is disabled, the corresponding GL light
	  source is disabled.  That means you can't disable your CHAI light but
	  turn on the GL light somewhere else.

	  \fn       void cLight::renderLightSource()
*/
//===========================================================================
void cLight::renderLightSource()
{
	// check if light source enabled
	if (m_enabled == false)
	{
		// disable OpenGL light source
		glDisable(m_glLightNumber);
		return;
	}

	computeGlobalCurrentObjectOnly();

	// enable this light in OpenGL
	glEnable(m_glLightNumber);

	// set lighting components
	glLightfv(m_glLightNumber, GL_AMBIENT, m_ambient.pColor());
	glLightfv(m_glLightNumber, GL_DIFFUSE, m_diffuse.pColor());
	glLightfv(m_glLightNumber, GL_SPECULAR, m_specular.pColor());

	// position the light source in (global) space (because we're not
	// _rendered_ as part of the scene graph)
	float position[4];

	position[0] = (float)m_globalPos.x;
	position[1] = (float)m_globalPos.y;
	position[2] = (float)m_globalPos.z;
	//position[0] = (float)m_localPos.x;
	//position[1] = (float)m_localPos.y;
	//position[2] = (float)m_localPos.z;

	// Directional light source...
	if (m_directionalLight) position[3] = 0.0f;

	// Positional light source...
	else position[3] = 1.0f;

	glLightfv(m_glLightNumber, GL_POSITION, (const float *)&position);

	// set cutoff angle
	glLightf(m_glLightNumber, GL_SPOT_CUTOFF, m_cutOffAngle);

	// set the direction of my light beam, if I'm a _positional_ spotlight
	if (m_directionalLight == false)
	{
		cVector3d dir = m_globalRot.getCol0();
		float direction[4];
		direction[0] = (float)dir.x;
		direction[1] = (float)dir.y;
		direction[2] = (float)dir.z;
		direction[3] = 0.0f;
		glLightfv(m_glLightNumber, GL_SPOT_DIRECTION, (const float *)&direction);
	}

	// set attenuation factors
	glLightf(m_glLightNumber, GL_CONSTANT_ATTENUATION, m_attConstant);
	glLightf(m_glLightNumber, GL_LINEAR_ATTENUATION, m_attLinear);
	glLightf(m_glLightNumber, GL_QUADRATIC_ATTENUATION, m_attQuadratic);

	// set exponent factor
	glLightf(m_glLightNumber, GL_SPOT_EXPONENT, m_spotExponent);

}


