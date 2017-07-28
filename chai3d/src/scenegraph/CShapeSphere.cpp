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
	\date       06/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CShapeSphere.h"
#include "CDraw3D.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	Constructor of cShapeSphere.

	\fn     cShapeSphere::cShapeSphere(const double& a_radius)
	\param  a_radius    Radius of sphere
*/
//===========================================================================
cShapeSphere::cShapeSphere(const double& a_radius)
{
	// initialize radius of sphere
	m_radius = cAbs(a_radius);
	m_texture = NULL;

	// set material properties
	m_material.setShininess(100);
	m_material.m_ambient.set((float)0.3, (float)0.3, (float)0.3);
	m_material.m_diffuse.set((float)0.1, (float)0.7, (float)0.8);
	m_material.m_specular.set((float)1.0, (float)1.0, (float)1.0);
};


//===========================================================================
/*!
	Render sphere in OpenGL

	\fn       void cShapeSphere::render(const int a_renderMode)
	\param    a_renderMode  See cGenericObject::render()
*/
//===========================================================================
void cShapeSphere::render(const int a_renderMode)
{
	// render material properties
	m_material.render();

	// render texture property if defined
	if (m_texture != NULL)
	{
		m_texture->render();
	}

	// allocate a new OpenGL quadric object for rendering a sphere
	GLUquadricObj *sphere;
	sphere = gluNewQuadric();

	// set rendering style
	gluQuadricDrawStyle(sphere, GLU_FILL);

	// set normal-rendering mode
	gluQuadricNormals(sphere, GLU_SMOOTH);

	// generate texture coordinates
	gluQuadricTexture(sphere, GL_TRUE);

	// render a sphere
	gluSphere(sphere, m_radius, 36, 36);

	// delete our quadric object
	gluDeleteQuadric(sphere);

	// turn off texture rendering if it has been used
	glDisable(GL_TEXTURE_2D);
}


//===========================================================================
/*!
	Compute forces between tool and sphere shape

	\fn       cVector3d cShapeSphere::computeLocalForce(const cVector3d& a_localPosition)
	\param    a_localPosition    position of tool in world coordinates
	\return   return reaction force if tool is located inside sphere
*/
//===========================================================================
cVector3d cShapeSphere::computeLocalForce(const cVector3d& a_localPosition)
{

	// In the following we compute the reaction forces between the tool and the
	// sphere.
	cVector3d localForce;

	if (a_localPosition.length() > m_radius)
	{
		// Here the tool is located outside the sphere
		localForce.set(0, 0, 0);
	}
	else
	{
		// Here the tool is located inside the sphere; we need to compute
		// the reaction force.

		// compute penetration distance between tool and surface of sphere
		double penetrationDistance = m_radius - a_localPosition.length();

		// get the material stiffness defined by the material properties of the object
		double materialStiffness = m_material.getStiffness();

		// if the pointer is located at the center of the tool, we reach
		// a singularity point
		if (a_localPosition.length() < CHAI_SMALL)
		{
			localForce.set(0, 0, 0);
		}

		// compute the direction of the reaction force. For the sphere its pretty
		// simple since it can be described by a vector going from the center of
		// the sphere towards the position tool.
		else
		{
			// compute a reaction force proportional to the penetration distance
			cVector3d forceDirection = cNormalize(a_localPosition);
			localForce = cMul(penetrationDistance * materialStiffness, forceDirection);
		}
	}

	return (localForce);
}


//===========================================================================
/*!
	Update bounding box of current object.

	\fn       void cShapeSphere::updateBoundaryBox()
*/
//===========================================================================
void cShapeSphere::updateBoundaryBox()
{
	m_boundaryBoxMin.set(-m_radius, -m_radius, -m_radius);
	m_boundaryBoxMax.set(m_radius, m_radius, m_radius);
}


//===========================================================================
/*!
	Scale object of defined scale factor

	\fn       void cShapeSphere::scaleObject(const cVector3d& a_scaleFactors)
	\param    a_scaleFactors Scale factor
*/
//===========================================================================
void cShapeSphere::scaleObject(const cVector3d& a_scaleFactors)
{
	m_radius = a_scaleFactors.x * m_radius;
}
