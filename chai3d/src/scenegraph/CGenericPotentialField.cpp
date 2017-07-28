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
#include "CGenericPotentialField.h"
//---------------------------------------------------------------------------

//==========================================================================
/*!
	  Compute interaction forces between a probe and this object, descending
	  through child objects

	  \fn       cVector3d cGenericPotentialField::computeForces(const cVector3d& a_probePosition)
	  \param    a_probePosition   Position of the probe in my parent's coordinate frame
	  \return   Returns the computed force in my parent's coordinate frame
*/
//===========================================================================
cVector3d cGenericPotentialField::computeForces(const cVector3d& a_probePosition)
{
	// compute the position of the probe in local coordinates.
	cVector3d probePositionLocal;
	probePositionLocal = cMul(cTrans(m_localRot), cSub(a_probePosition, m_localPos));

	// compute interaction forces with this object
	cVector3d localForce;
	localForce = computeLocalForce(probePositionLocal);

	// compute interaction forces with children
	for (unsigned int i = 0; i < m_children.size(); i++)
	{
		cGenericObject *nextObject = m_children[i];
		cVector3d force = nextObject->computeForces(probePositionLocal);
		localForce.add(force);
	}

	// convert the reaction force into my parent coordinates
	m_globalForce = cMul(m_localRot, localForce);

	return m_globalForce;
}
