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
#ifndef CShapeTorusH
#define CShapeTorusH
//---------------------------------------------------------------------------
#include "CGenericPotentialField.h"
#include "CMaterial.h"
#include "CTexture2D.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	  \file       CShapeTorus.h
	  \class      cShapeTorus
	  \brief      cShapeTorus describes a simple torus potential field
*/
//===========================================================================
class cShapeTorus : public cGenericPotentialField
{
public:
	// CONSTRUCTOR & DESTRUCTOR:
	//! Constructor of cShapeTorus.
	cShapeTorus(const double& a_insideRadius, const double& a_outsideRadius);

	//! Destructor of cShapeTorus.
	virtual ~cShapeTorus() {};

	// MEMBERS:
	//! Material properties
	cMaterial m_material;

	//! Texture property
	cTexture2D* m_texture;

	// METHODS:
	//! Render object in OpenGL.
	virtual void render(const int a_renderMode = 0);

	//! Update bounding box of current object
	virtual void updateBoundaryBox();

	//! object scaling
	virtual void scaleObject(const cVector3d& a_scaleFactors);

	//! Compute interaction force for current object in local frame
	virtual cVector3d computeLocalForce(const cVector3d& a_localPosition);

	//! Set inside and outside radius of torus
	void setSize(const double& a_innerRadius, const double& a_outerRadius) { m_innerRadius = cAbs(a_innerRadius); m_outerRadius = cAbs(a_outerRadius); }

	//! Get inside radius of torus
	double getInnerRadius() { return (m_innerRadius); }

	//! Get inside radius of torus
	double getOuterRadius() { return (m_outerRadius); }

protected:
	//! Inside radius of torus
	double m_innerRadius;

	//! Outside radius of torus
	double m_outerRadius;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
