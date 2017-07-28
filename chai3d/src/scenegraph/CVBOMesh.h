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

#ifndef _VBO_MESH_H_
#define _VBO_MESH_H_

#include "CMesh.h"

#include "glext.h"

//! A global function used to test whether a given extension is supported by the
//! current context...
bool IsExtensionSupported(char* szTargetExtension);

//! A global function to make sure the relevant GL support exists for VBO's.
//! Returns true for success, false if vbo's are unsupported or cannot be
//! initialized.
bool InitVBO();

//! An enumeration defining the components that _might_ be represented in a
//! VBO-enabled mesh
typedef enum {
	VBO_FLAG_VERTEX = 0,
	VBO_FLAG_COLOR,
	VBO_FLAG_NORMAL,
	VBO_FLAG_TEXCOORD,
	VBO_FLAG_INDEX,
	NUM_VBO_FLAGS
} vbo_flags;

#define MASK(x) (1<<x)

typedef cMesh vbo_mesh_parent_mesh_type;

//===========================================================================
/*!
\file       CVBOMesh.h
\class      cVBOMesh
\brief      This class is a replacement for cMesh that uses vertex buffers
			for rendering.  It allows for much faster rendering, but requires
			hardware support for VBO's and does not allow vertices to be moved
			around without re-initializing the VBO.

			This class also supports "rendering by proxy", i.e. using another
			mesh object to actually do the rendering.  This is useful if you
			have multiple objects represented by the same mesh and want to
			maintain separate transforms for them without having separate
			vertex arrays/buffers loaded, and without reading them in from
			disk separately.

			This class also allows a couple of other OpenGL effects, including
			"background blending", in which an object can be rendered through other
			opaque objects as if they were partially transparent, and polygon
			offsetting.
*/
//===========================================================================
class cVBOMesh : public vbo_mesh_parent_mesh_type {

public:

	//! Constructor of cVBOMesh
	cVBOMesh(cWorld* a_world, const bool a_useVertexBuffers = true);

	//! Destructor of cVBOMesh
	virtual ~cVBOMesh();

	//! Called when the context is deleted, allows this class to re-build VBO's
	virtual void onDisplayReset(const bool a_affectChildren = true);

	//! Called to transfer all mesh data to a vertex buffer
	virtual void finalize(const bool a_affectChildren = true);

	//! Called to disable the vertex buffer
	virtual void unfinalize(const bool a_affectChildren = true);

	//! This lets me act as a "mesh factory", producing new meshes like myself
	virtual cMesh* createMesh() const { return new cVBOMesh(m_parentWorld); }

	//! Actually draws the mesh
	virtual void renderMesh(const int a_renderMode = 0);

	//! Used to initialize "rendering by proxy"; from now on I'll use a_share to draw my mesh
	virtual void renderFrom(cMesh* a_share) { m_renderingProxy = a_share; }

	//! Requests the mesh object I'm currently using for rendering; returns 0 if I'm rendering my own mesh
	virtual cMesh* getRenderingProxy() { return m_renderingProxy; }

	//! Enables "background blend" mode
	virtual void setBackgroundBlend(bool enable, bool includeChildren = false);

	//! Should GL polygon offset be used for this mesh?
	bool m_usePolygonOffset;

protected:

	//! Enables a custom blend-with-background rendering mode
	bool m_backgroundBlend;

	//! If I'm using another mesh for rendering, this is him...
	cMesh* m_renderingProxy;

	//! Should we _try_ to render from vertex buffers (we may still fail if this is true)?
	bool m_renderFromVBO;

	// If this is 0, we haven't initialized any vertex buffers yet...
	unsigned int m_activeBufferObjects;

	//! The actual vertex buffers
	GLuint m_vertexBuffers[NUM_VBO_FLAGS];

	//! How many triangles are stored in our vertex buffers
	unsigned int m_numTriangles;

	//! Are we using int*'s (rather than short*'s) for our indices?
	int m_useLongIndices;

	//! What is the maximum vertex index we might have to render?
	int m_maxVertex;

	//! Clean up
	void clean_vertex_buffers();

};

#endif
