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

#ifdef _MSVC
#pragma warning (disable : 4786)
#endif

//---------------------------------------------------------------------------
#ifndef CWorldH
#define CWorldH
//---------------------------------------------------------------------------
#include "chai_globals.h"
#include "CCamera.h"
#include "CGenericObject.h"
#include "CTriangle.h"
#include "CTexture2D.h"
#include "CColor.h"
#include <vector>
//---------------------------------------------------------------------------
class cLight;

// The maximum number of lights that we expect OpenGL to support
#define MAXIMUM_OPENGL_LIGHT_COUNT 8

//===========================================================================
/*!
	  \file       CWorld.h
	  \class      cWorld
	  \brief      cWorld defines the typical root of the CHAI scene graph.
				  It stores lights, allocates textures, and serves as the
				  root for scene-wide collision detection.
*/
//===========================================================================
class cWorld : public cGenericObject
{

public:
	// CONSTRUCTOR & DESTRUCTOR:
	//! Constructor of cWorld
	cWorld();
	//! Destructor of cWorld
	virtual ~cWorld();

	// METHODS:
	//! Set the background color used when rendering.
	void setBackgroundColor(const GLfloat a_red, const GLfloat a_green,
		const GLfloat a_blue);
	//! Set the background color used when rendering.
	void setBackgroundColor(const cColorf& a_color);
	//! Get the background color used when rendering.
	cColorf getBackgroundColor() const { return (m_backgroundColor); }

	//! Enable or disable the rendering of this world's light sources
	void enableLightSourceRendering(bool enable) { m_renderLightSources = enable; }

	//! Create a new bitmap texture.
	cTexture2D* newTexture();
	//! Get a pointer to a texture by passing an index into my texture list
	cTexture2D* getTexture(unsigned int a_index) { return (m_textures[a_index]); };
	//! Add a texture to my texture list
	void addTexture(cTexture2D* a_texture);
	//! Remove a texture from my texture list
	bool removeTexture(cTexture2D* a_texture);
	//! Delete a texture
	bool deleteTexture(cTexture2D* a_texture);
	//! Delete all textures
	void deleteAllTextures();

	//! Compute collision detection between a ray segment and all objects in this world
	virtual bool computeCollisionDetection(
		cVector3d& a_segmentPointA, const cVector3d& a_segmentPointB,
		cGenericObject*& a_colObject, cTriangle*& a_colTriangle, cVector3d& a_colPoint,
		double& a_colDistance, const bool a_visibleObjectsOnly = false, const int a_proxyCall = -1);

	//! Render OpenGL lights
	virtual void render(const int a_renderMode = 0);

	//! Get access to a particular light source (between 0 and MAXIMUM_OPENGL_LIGHT_COUNT-1).
	virtual cLight* getLightSource(int index);

	//! Resets textures and displays for the world
	virtual void onDisplayReset(const bool a_affectChildren = true);

	//! It's useful to store the world's modelview matrix, for rendering stuff in "global" coordinates
	double m_worldModelView[16];

protected:
	// METHODS:
	//! Add a light source to this world
	friend class cLight;
	bool addLightSource(cLight* a_light);
	//! Remove a light source from this world
	bool removeLightSource(cLight* a_light);

	// MEMBERS:

	//! Background color. Default color is black.
	cColorf m_backgroundColor;
	//! List of textures
	vector<cTexture2D*> m_textures;
	//! List of light sources
	vector<cLight*> m_lights;
	//! Should I render my light sources, or just use the current OpenGL light state?
	bool m_renderLightSources;
	//! Some apps may have multiple cameras, which would cause recursion when resetting the display
	bool m_performingDisplayReset;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

