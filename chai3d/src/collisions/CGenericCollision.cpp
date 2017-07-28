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
#include "CGenericCollision.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	Constructor of cGenericCollision.

	\fn       cGenericCollision::cGenericCollision()
*/
//===========================================================================
cGenericCollision::cGenericCollision()
{
	// set material properties

	// set default color for rendering collision detector
	m_material.m_ambient.set((float)0.1, (float)0.5, (float)0.5);
	m_material.m_diffuse.set((float)0.1, (float)0.6, (float)0.6);
	m_material.m_specular.set((float)1.0, (float)1.0, (float)1.0);
	m_material.setShininess(100);

	// set default value for display depth (level 0 = root)
	m_displayDepth = 3;
}

