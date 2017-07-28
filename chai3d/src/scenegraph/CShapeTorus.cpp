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
#include "CShapeTorus.h"
#include "CDraw3D.h"

// Different compilers like slightly different GLUT's 
#ifdef _MSVC
#include "../../external/OpenGL/msvc6/glut.h"
#else
#ifdef _POSIX
#include <GL/glut.h>
#else
#include "../../external/OpenGL/bbcp6/glut.h"
#endif
#endif

//---------------------------------------------------------------------------

//===========================================================================
/*!
	Constructor of cShapeTorus.

	\fn     cShapeTorus::cShapeTorus(const double& a_insideRadius, const double& a_outsideRadius)
	\param  a_insideRadius    Inside radius of torus
	\param  a_outsideRadius   Outside radius of torus
*/
//===========================================================================
cShapeTorus::cShapeTorus(const double& a_insideRadius, const double& a_outsideRadius)
{
	// initialize radius of sphere
	setSize(a_insideRadius, a_outsideRadius);

	// set material properties
	m_material.setShininess(100);
	m_material.m_ambient.set((float)0.3, (float)0.3, (float)0.3);
	m_material.m_diffuse.set((float)0.1, (float)0.7, (float)0.8);
	m_material.m_specular.set((float)1.0, (float)1.0, (float)1.0);
	m_material.setStiffness(100.0);
};


//===========================================================================
/*!
	Render sphere in OpenGL

	\fn       void cShapeTorus::render(const int a_renderMode)
	\param    a_renderMode  See cGenericObject::render()
*/
//===========================================================================
void cShapeTorus::render(const int a_renderMode)
{
	// render material properties
	m_material.render();

	// render texture property if defined
	if (m_texture != NULL)
	{
		m_texture->render();
	}

	// draw sphere
	glutSolidTorus(m_innerRadius, m_outerRadius, 32, 32);

	// turn off texture rendering if it has been used
	glDisable(GL_TEXTURE_2D);
}


//===========================================================================
/*!
	Compute forces between tool and sphere shape

	\fn       cVector3d cShapeTorus::computeLocalForce(const cVector3d& a_localPosition)
	\param    a_localPosition    position of tool in world coordinates
	\return   return reaction force if tool is located inside sphere
*/
//===========================================================================
cVector3d cShapeTorus::computeLocalForce(const cVector3d& a_localPosition)
{

	// In the following we compute the reaction forces between the tool and the
	// sphere.
	cVector3d localForce;

	// project pointer on torus plane (z=0)
	cVector3d fingerProjection = a_localPosition;
	fingerProjection.z = 0;

	// search for the nearest point on the torus medial axis
	if (a_localPosition.lengthsq() > CHAI_SMALL)
	{
		cVector3d pointAxisTorus = cMul(m_outerRadius, cNormalize(fingerProjection));

		// compute eventual penetration of finger inside the torus
		cVector3d vectTorusFinger = cSub(a_localPosition, pointAxisTorus);

		double distance = vectTorusFinger.length();

		// finger inside torus, compute forces
		if ((distance < m_innerRadius) && (distance > 0.001))
		{
			localForce = cMul((m_innerRadius - distance) * (m_material.getStiffness()), cNormalize(vectTorusFinger));
		}

		// finger is outside torus
		else
		{
			localForce.zero();
		}
	}
	else
	{
		localForce.zero();
	}

	return (localForce);
}


//===========================================================================
/*!
	Update bounding box of current object.

	\fn       void cShapeTorus::updateBoundaryBox()
*/
//===========================================================================
void cShapeTorus::updateBoundaryBox()
{
	m_boundaryBoxMin.set(-m_outerRadius, -m_outerRadius, -(m_outerRadius - m_innerRadius));
	m_boundaryBoxMax.set(m_outerRadius, m_outerRadius, (m_outerRadius - m_innerRadius));
}


//===========================================================================
/*!
	Scale the torus with a uniform scale factor

	\fn       void cShapeTorus::scaleObject(const cVector3d& a_scaleFactors)
	\param    a_scaleFactors x,y,z scale factors
*/
//===========================================================================
void cShapeTorus::scaleObject(const cVector3d& a_scaleFactors)
{
	m_outerRadius = a_scaleFactors.x * m_outerRadius;
	m_innerRadius = a_scaleFactors.x * m_innerRadius;
}
