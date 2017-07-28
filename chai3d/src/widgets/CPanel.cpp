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
	\author:    Dan Morris
	\version    1.0
	\date       3/2005
*/
//===========================================================================
#include "CPanel.h"
#include "CGenericCollision.h"


//===========================================================================
/*!
	Constructor of cPanel
	\fn         cPanel::cPanel(cWorld* a_world, bool enableCollisionDetecion=false)
	\param      a_world                     The parent CHAI world
	\param      a_enableCollisionDetecion   Should this panel exist in the world for
											cd purposes?
*/
//===========================================================================
cPanel::cPanel(cWorld* a_world, const bool a_enableCollisionDetecion) : cMesh(a_world)
{
	m_disableClipping = true;
	m_showRect = true;
	m_showEdges = true;
	m_rectColor.set(0.5f, 0.5f, 0.5f);
	m_edgeColor.set(0.8f, 0.8f, 0.8f);
	m_size.set(100.f, 100.f, 0);
	m_edgeWidth = 1.0;
	m_useLighting = false;
	m_disableDepthTest = true;
	m_usePolygonOffset = false;

	// Create four vertices
	newVertex(0, 0, 0);
	newVertex(0, 0, 0);
	newVertex(0, 0, 0);
	newVertex(0, 0, 0);

	// This will put the vertices where they go
	setSize(m_size);

	// Create two triangles in case we want to use them for CD
	newTriangle(0, 1, 2);
	newTriangle(1, 2, 3);

	// Initialize our vertex mesh
	m_vertices[0].setTexCoord(0, 0);
	m_vertices[1].setTexCoord(1, 0);
	m_vertices[2].setTexCoord(1, 1);
	m_vertices[3].setTexCoord(0, 1);

	// Over-write some cMesh variables...

	m_useMaterialProperty = false;
	m_useVertexColors = false;
	m_useTransparency = false;
	m_useTextureMapping = true;

	if (a_enableCollisionDetecion == false) {
		// 2d objects aren't involved in collisions
		this->deleteCollisionDetector();
	}

	m_useMultipassTransparency = false;
	m_useDisplayList = false;
}



//===========================================================================
/*!
	Destructor of cPanel
	\fn         cPanel::~cPanel()
*/
//===========================================================================
cPanel::~cPanel()
{

}


//===========================================================================
/*!
	Renders a cPanel to the display
	\fn         void cPanel::render(const int a_renderMode)
	\param      a_renderMode    Current rendering pass; see cGenericObject::render()
*/
//===========================================================================
void cPanel::render(const int a_renderMode)
{
	// We don't do multipass, so only render on the opaque passes
	if (a_renderMode != CHAI_RENDER_MODE_RENDER_ALL &&
		a_renderMode != CHAI_RENDER_MODE_TRANSPARENT_FRONT_ONLY) return;

	// Back up some state
	glPushAttrib(GL_ENABLE_BIT);

	// Disable clipping if necessary
	if (m_disableClipping) {
		for (int i = 0; i < GL_MAX_CLIP_PLANES; i++)
			glDisable(GL_CLIP_PLANE0 + i);
	}

	// Back up and re-initialize GL normal state
	float old_normal[3];
	glGetFloatv(GL_CURRENT_NORMAL, old_normal);

	glDisableClientState(GL_NORMAL_ARRAY);
	glNormal3f(0, 0, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (m_useLighting) glEnable(GL_LIGHTING);
	else glDisable(GL_LIGHTING);

	// No need to show backfaces, since this is a flat object
	glDisable(GL_CULL_FACE);

	// Enable or disable blending if necessary
	if (m_useTransparency)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	// Turn off the depth-test if necessary
	if (m_disableDepthTest)
	{
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
	}

	// Turn on polygon-offsetting if necessary
	if (m_usePolygonOffset)
	{
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1.0, -1.0);
	}

	// If material properties exist, render them
	if (m_showRect)
	{
		if (m_useMaterialProperty)
			m_material.render();

		else
		{
			glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
			glEnable(GL_COLOR_MATERIAL);
			m_rectColor.render();
		}
	}

	// If we have a texture, enable it
	if ((m_texture != NULL) && m_useTextureMapping)
	{
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		m_texture->render();
	}
	else
	{
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	// Set up appropriate rendering state
	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_DOUBLE, sizeof(cVertex), &m_vertices[0].m_localPos);
	glTexCoordPointer(2, GL_DOUBLE, sizeof(cVertex), &m_vertices[0].m_texCoord);

	// Render the quad if necessary
	if (m_showRect || (m_useTextureMapping && m_texture))
	{

		glBegin(GL_QUADS);
		for (int i = 0; i < 4; i++)
		{
			glArrayElement(0);
			glArrayElement(1);
			glArrayElement(2);
			glArrayElement(3);
		}
		glEnd();
	}

	glDisable(GL_TEXTURE_2D);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	// Render the outline if necessary
	if (m_showEdges)
	{

		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_LINE_SMOOTH);

		// This actually doesn't work very well on most systems, since glLineWidth
		// is limited to a small number of pixels
		glLineWidth(m_edgeWidth);

		// Set up the edge color
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);
		m_edgeColor.render();

		// The edge should be pushed up in front of the background box
		//
		// This is independent of whether polygon-offsetting is applied
		// to the whole object
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(-2.0, -2.0);

		// Draw a line quad
		glBegin(GL_LINE_LOOP);
		glArrayElement(0);
		glArrayElement(1);
		glArrayElement(2);
		glArrayElement(3);
		glEnd();

		glDisable(GL_POLYGON_OFFSET_LINE);
		glPopAttrib();
	}

	glDisableClientState(GL_VERTEX_ARRAY);

	// Fix any rendering state I might have modified...
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_LIGHTING);
	glNormal3f(old_normal[0], old_normal[1], old_normal[2]);
	glPolygonOffset(0, 0);

	// Restore the clip-plane state, etc...
	glPopAttrib();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);

}


//===========================================================================
/*!
	Set the width and height of the panel (in pixels) (z is ignored)
	\fn         void cPanel::setSize(const cVector3d& a_size)
	\param      a_size    New size of the panel, in pixels (for 2D rendering) or
						  GL units (for 3D rendering).  z is ignored.
*/
//===========================================================================
void cPanel::setSize(const cVector3d& a_size)
{
	m_size = a_size;
	m_vertices[0].setPos(-1.0*(m_size.x / 2.0), -1.0*(m_size.y / 2.0), 0);
	m_vertices[1].setPos(+1.0*(m_size.x / 2.0), -1.0*(m_size.y / 2.0), 0);
	m_vertices[2].setPos(+1.0*(m_size.x / 2.0), +1.0*(m_size.y / 2.0), 0);
	m_vertices[3].setPos(-1.0*(m_size.x / 2.0), +1.0*(m_size.y / 2.0), 0);
	if (m_collisionDetector) m_collisionDetector->initialize();
}

