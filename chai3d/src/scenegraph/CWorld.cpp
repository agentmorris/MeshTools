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
#include "CWorld.h"
#include "CLight.h"
//---------------------------------------------------------------------------

#ifndef _MSVC
#include <float.h>
#endif


//==========================================================================
/*!
	  Constructor of cWorld.

	  \fn       cWorld::cWorld()
*/
//===========================================================================
cWorld::cWorld()
{
#ifdef _BCPP
	_control87(MCW_EM, MCW_EM);
#endif

	// set background properties
	m_backgroundColor.set(0.0f, 0.0f, 0.0f, 1.0f);

	m_renderLightSources = 1;

	m_performingDisplayReset = 0;

	memset(m_worldModelView, 0, sizeof(m_worldModelView));
}


//===========================================================================
/*!
	  Destructor of cWorld.  Deletes the world, all his children, and all
	  his textures.

	  \fn       cWorld::~cWorld()
*/
//===========================================================================
cWorld::~cWorld()
{
	// delete all children
	deleteAllChildren();

	// clear textures list
	deleteAllTextures();
}


//===========================================================================
/*!
	Create new texture and add it to textures list.

	\fn         cTexture2D* cWorld::newTexture()
	\return     Return pointer to new texture entity.
*/
//===========================================================================
cTexture2D* cWorld::newTexture()
{
	// create new texture entity
	cTexture2D* newTexture = new cTexture2D();

	// add texture to list
	m_textures.push_back(newTexture);

	// return pointer to new texture
	return (newTexture);
}


//===========================================================================
/*!
	Add texture to texture list.

	\fn         void cWorld::addTexture(cTexture2D* a_texture)
	\param      a_texture  Texture to be added to the textures list.
*/
//===========================================================================
void cWorld::addTexture(cTexture2D* a_texture)
{
	// add texture to list
	m_textures.push_back(a_texture);
}


//===========================================================================
/*!
	Remove texture from textures list. Texture is not deleted from memory.

	\fn         bool cWorld::removeTexture(cTexture2D* a_texture)
	\param      a_texture  Texture to be removed from textures list.
	\return     Return \b true if operation succeeded
*/
//===========================================================================
bool cWorld::removeTexture(cTexture2D* a_texture)
{
	// set iterator
	std::vector<cTexture2D*>::iterator nextTexture;
	nextTexture = m_textures.begin();

	// search texture in  list and remove it
	for (unsigned int i = 0; i < m_textures.size(); i++)
	{
		if ((*nextTexture) == a_texture)
		{
			// remove object from list
			m_textures.erase(nextTexture);

			// return success
			return (true);
		}
	}

	// operation failed
	return (false);
}


//===========================================================================
/*!
	Delete texture from textures list and erase it from memory.

	\fn         bool cWorld::deleteTexture(cTexture2D* a_texture)
	\param      a_texture  Texture to be deleted.
	\return     Return \b true if operation succeeded
*/
//===========================================================================
bool cWorld::deleteTexture(cTexture2D* a_texture)
{
	// remove texture from list
	bool result = removeTexture(a_texture);

	// if operation succeeds, delete object
	if (result)
	{
		delete a_texture;
	}

	// return result
	return (result);
}


//===========================================================================
/*!
	Delete all texture from memory.

	\fn         void cWorld::deleteAllTextures()
*/
//===========================================================================
void cWorld::deleteAllTextures()
{
	// delete all textures
	for (unsigned int i = 0; i < m_textures.size(); i++)
	{
		cTexture2D* nextTexture = m_textures[i];
		delete nextTexture;
	}

	// clear textures list
	m_textures.clear();
}


//===========================================================================
/*!
	Set the background color used when rendering.  This really belongs in
	cCamera or cViewport; it's a historical artifact that it lives here.

	\fn         void cWorld::setBackgroundColor(const GLfloat a_red,
				const GLfloat a_green, const GLfloat a_blue)
	\param      a_red  Red component.
	\param      a_green  Green component.
	\param      a_blue  Blue component.
*/
//===========================================================================
void cWorld::setBackgroundColor(const GLfloat a_red, const GLfloat a_green,
	const GLfloat a_blue)
{
	m_backgroundColor.set(a_red, a_green, a_blue);
}


//===========================================================================
/*!
	Set the background color used when rendering.  This really belongs in
	cCamera or cViewport; it's a historical artifact that it lives here.

	\fn         void cWorld::setBackgroundColor(const cColorf& a_color)
	\param      a_color  new background color.
*/
//===========================================================================
void cWorld::setBackgroundColor(const cColorf& a_color)
{
	m_backgroundColor = a_color;
}


//===========================================================================
/*!
	Add an OpenGL light source to the world. A maximum of eight light
	sources can be registered. For each registered light source, an
	OpenGL lightID number is defined

	\fn         bool cWorld::addLightSource(cLight* a_light)
	\param      a_light light source to register.
	\return     return \b true if light source was registered, otherwise
				return \b false.
*/
//===========================================================================
bool cWorld::addLightSource(cLight* a_light)
{
	// check if number of lights already equal to 8.
	if (m_lights.size() >= MAXIMUM_OPENGL_LIGHT_COUNT)
	{
		return (false);
	}

	// search for a free ID number
	int light_id = GL_LIGHT0;
	bool found = false;

	while (light_id < GL_LIGHT0 + MAXIMUM_OPENGL_LIGHT_COUNT)
	{

		// check if ID is not already used
		unsigned int i;
		bool free = true;
		for (i = 0; i < m_lights.size(); i++)
		{
			cLight* nextLight = m_lights[i];

			if (nextLight->m_glLightNumber == light_id)
			{
				free = false;
			}
		}

		// check if a free ID was found
		if (free)
		{
			a_light->m_glLightNumber = light_id;
			found = true;
			break;
		}

		light_id++;
	}

	// finalize
	if (found)
	{
		m_lights.push_back(a_light);
		return (true);
	}
	else
	{
		return (false);
	}
}


//===========================================================================
/*!
	Remove a light source from world.

	\fn         bool cWorld::removeLightSource(cLight* a_light)
	\param      a_light light source to be removed.
	\return     return \b true if light source was removed, otherwise
				return \b false.
*/
//===========================================================================
bool cWorld::removeLightSource(cLight* a_light)
{
	// set iterator
	std::vector<cLight*>::iterator nextLight;

	for (nextLight = m_lights.begin();
		nextLight != m_lights.end();
		nextLight++) {


		if ((*nextLight) == a_light)
		{
			// remove object from list
			m_lights.erase(nextLight);

			// return success
			return (true);
		}

	}

	// operation failed
	return (false);
}


//===========================================================================
/*!
	Get access to a particular light source (between 0 and MAXIMUM_OPENGL_LIGHT_COUNT-1).
	Returns a pointer to the requested light, or zero if it's not available.

	\fn         cLight cWorld::getLightSource(int index)
	\param      index  Specifies the light (0 -> 7) that should be accessed
	\return     return \b A pointer to a valid light or 0 if that light doesn't exist
*/
//===========================================================================
cLight* cWorld::getLightSource(int index) {

	// Make sure this is a valid index
	if (index < 0 || (unsigned int)(index) >= m_lights.size()) return 0;

	// Return the light that we were supplied with by the creator of the world
	return m_lights[index];

}


//===========================================================================
/*!
	Render the world in OpenGL.

	\fn         void cWorld::render(const int a_renderMode)
	\param      a_renderMode  Rendering Mode.
*/
//===========================================================================
void cWorld::render(const int a_renderMode)
{

	// Set up the CHAI openGL defaults (see cGenericObject::render())
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// Back up the "global" modelview matrix for future reference
	glGetDoublev(GL_MODELVIEW_MATRIX, m_worldModelView);

	if (m_renderLightSources)
	{
		// enable lighting
		glEnable(GL_LIGHTING);

		// render light sources
		unsigned int i;
		for (i = 0; i < m_lights.size(); i++)
		{
			m_lights[i]->renderLightSource();
		}
	}
}


//===========================================================================
/*!
	Determine whether the given segment intersects a triangle in this world.
	The segment is described by a start point /e a_segmentPointA and end point
	/e a_segmentPointB. Collision detection functions of all children of the
	world are called, which recursively call the collision detection functions
	for all objects in this world.  If there is more than one collision,
	the one closest to a_segmentPointA is the one returned.

	For any dynamic objects in the world with valid position and rotation
	histories (as indicated by the m_historyValid member of cGenericObject), the
	first endpoint of the segment is adjusted so that it is in the same location
	relative to the moved object as it was at the previous haptic iteration
	(provided the object's m_lastRot and m_lastPos were updated), so that
	collisions between the segment and the moving object can be properly detected.
	If the returned collision is with a moving object, the actual parameter
	corresponding to a_segmentPointA is set to the adjusted position for
	that object.

	If a collision(s) is located, information about the (closest) collision is
	stored in the corresponding parameters \e a_colObject, \e a_colTriangle,
	\e a_colPoint, and \e a_colDistance.

	\param  a_segmentPointA  Start point of segment.  Value may be changed if
							 returned collision is with a moving object.
	\param  a_segmentPointB  End point of segment.
	\param  a_colObject      Pointer to nearest collided object.
	\param  a_colTriangle    Pointer to nearest collided triangle.
	\param  a_colPoint       Position of nearest collision.
	\param  a_colDistance    Distance between segment origin and nearest collision point.
	\param  a_visibleObjectsOnly  Should we ignore invisible objects?
	\param  a_proxyCall      If this is > 0, this is a call from a proxy, and the value
							 of a_proxyCall specifies which call this is.  -1 for
							 non-proxy calls.
*/
//===========================================================================
bool cWorld::computeCollisionDetection(
	cVector3d& a_segmentPointA, const cVector3d& a_segmentPointB,
	cGenericObject*& a_colObject, cTriangle*& a_colTriangle, cVector3d& a_colPoint,
	double& a_colDistance, const bool a_visibleObjectsOnly, int a_proxyCall)
{
	// initialize objects for collision detection calls
	cGenericObject* t_colObject;
	cTriangle *t_colTriangle;
	cVector3d t_colPoint;
	bool hit = false;
	double colSquareDistance = CHAI_LARGE;
	double t_colSquareDistance = colSquareDistance;

	// get the transpose of the local rotation matrix
	cMatrix3d transLocalRot;
	m_localRot.transr(transLocalRot);

	// convert second endpoint of the segment into local coordinate frame
	cVector3d localSegmentPointB = a_segmentPointB;
	localSegmentPointB.sub(m_localPos);
	transLocalRot.mul(localSegmentPointB);

	// r_segmentPointA is the value that we will return in a_segmentPointA
	// at the end; it should be unchanged from the received value of
	// a_segmentPointA, unless the collision that will be returned is with
	// a moving object, in which case it will be adjusted so that it is in the
	// same location relative to the moving object as it was at the previous
	// haptic iteration; this is necessary so that the proxy algorithm gets the
	// correct new proxy position
	cVector3d r_segmentPointA = a_segmentPointA;

	// check for collisions with all children of this world
	unsigned int nChildren = m_children.size();
	for (unsigned int i = 0; i < nChildren; i++)
	{
		// start with the first segment point as it was received
		cVector3d l_segmentPointA = a_segmentPointA;

		// convert first endpoint of the segment into local coordinate frame
		cVector3d localSegmentPointA = l_segmentPointA;
		localSegmentPointA.sub(m_localPos);
		transLocalRot.mul(localSegmentPointA);

		// if this is a first call from the proxy algorithm, and the current
		// child is a dynamic object, adjust the first segment endpoint so that
		// it is in the same position relative to the moving object as it was
		// at the previous haptic iteration
		if ((a_proxyCall == 1) && (m_children[i]->m_historyValid))
			AdjustCollisionSegment(l_segmentPointA, localSegmentPointA, m_children[i]);

		// call this child's collision detection function to see if it (or any
		// of its descendants) are intersected by the segment
		int coll = m_children[i]->computeCollisionDetection(localSegmentPointA, localSegmentPointB,
			t_colObject, t_colTriangle, t_colPoint, t_colSquareDistance, a_visibleObjectsOnly, a_proxyCall);

		// if a collision was found with this child, and this collision is
		// closer than any others found so far...
		if ((coll == 1) && (t_colSquareDistance < colSquareDistance))
		{
			// record that there has been a collision
			hit = true;

			// set the return parameters with information about this collision
			// (they may be overwritten if a closer collision is found later
			// on in this loop)
			a_colObject = t_colObject;
			a_colTriangle = t_colTriangle;
			a_colPoint = t_colPoint;

			// this is now the shortest distance to a collision found so far
			colSquareDistance = t_colSquareDistance;

			// convert collision point into parent coordinate frame
			m_localRot.mul(a_colPoint);
			a_colPoint.add(m_localPos);

			// localSegmentPointA's position (as possibly modified in the
			// call to the child's collision detector), converted back to
			// the global coordinate frame, is currently the proxy position
			// we will want to return (unless we find a closer collision later on)
			r_segmentPointA = cAdd(cMul(m_localRot, localSegmentPointA), m_localPos);
		}
	}

	// for optimization reasons, the collision detectors only computes the
	// squared distance between a_segmentA and collision point; this
	// computes a square root to obtain the actual distance.
	a_colDistance = sqrt(colSquareDistance);

	// set the value of the actual parameter for the first segment point; this
	// is the proxy position when called by the proxy algorithm, and may be
	// different from the value passed in this parameter if the closest collision
	// was with a moving object
	a_segmentPointA = r_segmentPointA;

	// return whether there was a collision between the segment and this world
	return (hit);
}


//===========================================================================
/*!
	Called by the user or by the viewport when the world needs to have
	textures and display lists reset (e.g. after a switch to or from
	fullscreen).

	\fn     void cWorld::onDisplayReset(const bool a_affectChildren = true)
	\param  a_affectChildren  Should I pass this on to my children?
*/
//===========================================================================
void cWorld::onDisplayReset(const bool a_affectChildren) {

	// Prevent the world from getting reset multiple times when there are multiple cameras
	if (m_performingDisplayReset) return;

	m_performingDisplayReset = 1;

	// This will pass the call on to any children I might have...
	cGenericObject::onDisplayReset(a_affectChildren);

	m_performingDisplayReset = 0;
}

