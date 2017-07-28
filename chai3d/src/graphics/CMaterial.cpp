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
#include "CMaterial.h"
#include "CMaths.h"
#include "CMacrosGL.h"

//---------------------------------------------------------------------------
//===========================================================================
/*!
	Set the transparency level (by setting the alpha value for all color properties)

	\fn     void cMaterial::setTransparencyLevel(const float a_levelTransparency)
	\param  a_levelTransparency  Level of transparency.
*/
//===========================================================================
void cMaterial::setTransparencyLevel(float a_levelTransparency)
{
	// make sur value is in range [0.0 - 1.0]
	float level = cClamp(a_levelTransparency, 0.0f, 1.0f);

	// apply new value
	m_ambient.setA(level);
	m_diffuse.setA(level);
	m_specular.setA(level);
	m_emission.setA(level);
}


//===========================================================================
/*!
	Set the level of shininess. Value are clamped to range from 0 --> 128

	\fn     void cMaterial::setShininess(const GLuint a_shininess)
	\param  a_shininess  Level of shininess
*/
//===========================================================================
void cMaterial::setShininess(GLuint a_shininess)
{
	m_shininess = cClamp(a_shininess, (GLuint)0, (GLuint)128);
}


//===========================================================================
/*!
	Set the level of stiffness. Clamped to be a non-negative value.

	\fn     void cMaterial::setStiffness(const double a_stiffness)
	\param  a_stiffness  Level of stiffness
*/
//===========================================================================
void cMaterial::setStiffness(double a_stiffness)
{
	m_stiffness = cClamp0(a_stiffness);
}


//===========================================================================
/*!
	Set the level of static friction. Clamped to be a non-negative value.

	\fn     void cMaterial::setStaticFriction(const double a_friction)
	\param  a_friction  Level of friction.
*/
//===========================================================================
void cMaterial::setStaticFriction(double a_friction)
{
	m_static_friction = cClamp0(a_friction);
}


//===========================================================================
/*!
	Set the level of dynamic friction. Clamped to be a non-negative value.

	\fn     void cMaterial::setDynamicFriction(const double a_friction)
	\param  a_friction  Level of friction.
*/
//===========================================================================
void cMaterial::setDynamicFriction(double a_friction)
{
	m_dynamic_friction = cClamp0(a_friction);
}


//===========================================================================
/*!
	Render this material in OpenGL.

	\fn     void cMaterial::render()
*/
//===========================================================================
void cMaterial::render()
{
	glDisable(GL_COLOR_MATERIAL);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (const float *)&m_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (const float *)&m_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (const float *)&m_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, (const float *)&m_emission);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, m_shininess);
}


void cMaterial::print() const
{
	CHAI_DEBUG_PRINT("A %0.2f,%0.2f,%0.2f,%0.2f, D %0.2f,%0.2f,%0.2f,%0.2f S %0.2f,%0.2f,%0.2f,%0.2f E %0.2f,%0.2f,%0.2f,%0.2f \n",
		m_ambient[0], m_ambient[1], m_ambient[2], m_ambient[3],
		m_diffuse[0], m_diffuse[1], m_diffuse[2], m_diffuse[3],
		m_specular[0], m_specular[1], m_specular[2], m_specular[3],
		m_emission[0], m_emission[1], m_emission[2], m_emission[3]
	);
}
