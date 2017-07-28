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
#include "CVBOMesh.h"
#include "cVertex.h"
#include "CTriangle.h"

// Typedefs for convenience
typedef float vbo_vertex_type;
typedef float vbo_normal_type;
typedef float vbo_texcoord_type;
typedef unsigned char vbo_color_type;

#ifdef _MSVC
#pragma warning(disable:4244)
#endif

// If this is defined, we use a hardware index buffer to store triangles,
// otherwise we _only_ store vertices on the hardware
#define USE_INDEX_BUFFER

// Define this to send vbo-related messages to the console
// #define DEBUG_VBO_OUTPUT

// gl procedure information...
extern PFNGLBINDBUFFERARBPROC           glBindBufferARB;
extern PFNGLDELETEBUFFERSARBPROC        glDeleteBuffersARB;
extern PFNGLGENBUFFERSARBPROC           glGenBuffersARB;
extern PFNGLISBUFFERARBPROC             glIsBufferARB;
extern PFNGLBUFFERDATAARBPROC           glBufferDataARB;
extern PFNGLBUFFERSUBDATAARBPROC        glBufferSubDataARB;
extern PFNGLGETBUFFERSUBDATAARBPROC     glGetBufferSubDataARB;
extern PFNGLMAPBUFFERARBPROC            glMapBufferARB;
extern PFNGLUNMAPBUFFERARBPROC          glUnmapBufferARB;
extern PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB;
extern PFNGLGETBUFFERPOINTERVARBPROC    glGetBufferPointervARB;
extern PFNGLDRAWRANGEELEMENTSPROC       glDrawRangeElements;

//! Have we successfully initialized VBO's?
static bool vbo_initialized = false;


//===========================================================================
/*!
Constructor of cVBOMesh
\fn         cVBOMesh::cVBOMesh(cWorld* a_world, const bool a_useVertexBuffers=true)
\param      a_world                     The parent CHAI world
\param      a_useVertexBuffers          Pass 'false' to create a cVBOMesh that does not
										initially use vertex buffers.  An explicit call
										to "finalize" will re-enable VBO's.
*/
//===========================================================================
cVBOMesh::cVBOMesh(cWorld* a_world, const bool a_useVertexBuffers) : vbo_mesh_parent_mesh_type(a_world)
{

	m_backgroundBlend = false;
	m_usePolygonOffset = false;

	// By default, we use VBO's for rendering
	m_renderFromVBO = a_useVertexBuffers;
	m_useDisplayList = false;

	// But we're not ready to render yet...
	m_activeBufferObjects = 0;
	m_renderingProxy = 0;


}


//===========================================================================
/*!
Destructor of cVBOMesh
\fn         cVBOMesh::~cVBOMesh()
*/
//===========================================================================
cVBOMesh::~cVBOMesh()
{
	clean_vertex_buffers();
}


//===========================================================================
/*!
Actually draws the mesh to the screen.  Uses a rendering proxy if one exists,
and creates a vertex buffer before rendering if necessary.
\fn         cVBOMesh::~cVBOMesh()
\param      a_renderMode    The current rendering pass; see cGenericObject::render()
*/
//===========================================================================
void cVBOMesh::renderMesh(const int a_renderMode)
{

	unsigned int i;

	if (m_renderingProxy) {
		m_renderingProxy->renderSceneGraph(a_renderMode);
		return;
	}

	// If we _should_ have finalized, but we haven't...
	if (m_renderFromVBO && (m_activeBufferObjects == 0)) {

		// Try again...
		finalize();

	}

	// If we don't have any vertex buffers...
	if (m_activeBufferObjects == 0) {
		vbo_mesh_parent_mesh_type::renderMesh(a_renderMode);
		return;
	}

	// Initialize rendering arrays
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_INDEX_ARRAY);
	glDisableClientState(GL_EDGE_FLAG_ARRAY);

	// Set polygon and face mode
	glPolygonMode(GL_FRONT_AND_BACK, m_triangleMode);

	// Set up useful rendering state
	glEnable(GL_LIGHTING);

	if (m_backgroundBlend == false) {
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);

		// Enable or disable blending
		if (m_useTransparency) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE);
		}
		else {
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}
	}

	else {

		// Turn on a _reverse_ depth test
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_GREATER);

		// Turn off the depth mask
		glDepthMask(GL_FALSE);

		// Enable blending based on a fixed constant
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR);
	}

	if (m_usePolygonOffset) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1.0, -1.0);
	}

	if (m_useMaterialProperty) m_material.render();

	// should we use vertex colors?
	int use_vertex_color_buffer = m_useVertexColors && (m_activeBufferObjects & MASK(VBO_FLAG_COLOR));

	if (use_vertex_color_buffer) {

		// Clear the effects of material properties...
		if (m_useMaterialProperty == 0) {
			float fnull[4] = { 0,0,0,0 };
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (const float *)&fnull);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, (const float *)&fnull);
		}

		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);
		glEnableClientState(GL_COLOR_ARRAY);
	}
	else {
		glDisable(GL_COLOR_MATERIAL);
		glDisableClientState(GL_COLOR_ARRAY);
	}

	// A default color for objects that don't have vertex colors or
	// material properties (otherwise they're invisible)...
	if (use_vertex_color_buffer == 0 && m_useMaterialProperty == 0) {
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glColor4f(1, 1, 1, 1);
	}

	// Set up all of the rendering buffers...

	// If we have a texture, enable it
	if ((m_texture != NULL) && m_useTextureMapping && (m_activeBufferObjects & MASK(VBO_FLAG_TEXCOORD))) {
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		m_texture->render();

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vertexBuffers[VBO_FLAG_TEXCOORD]);
		glTexCoordPointer(2, GL_FLOAT, 0, 0);
	}

	if (m_useVertexColors && (m_activeBufferObjects & MASK(VBO_FLAG_COLOR))) {
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vertexBuffers[VBO_FLAG_COLOR]);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
	}

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vertexBuffers[VBO_FLAG_NORMAL]);
	glNormalPointer(GL_FLOAT, 0, 0);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_vertexBuffers[VBO_FLAG_VERTEX]);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	// Do the drawing (if we have an index buffer)
	if (m_activeBufferObjects & MASK(VBO_FLAG_INDEX)) {

		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_vertexBuffers[VBO_FLAG_INDEX]);

		glDrawRangeElements(GL_TRIANGLES,
			0, m_maxVertex, // Vertex index range
			m_numTriangles * 3,
			m_useLongIndices ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT,
			0);

		/*
		// Useful for experimenting with buffer-mapping...

		int bsize = m_numTriangles*sizeof(unsigned short)*3;
		unsigned char* buf = new unsigned char[bsize];
		glGetBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0,bsize,buf);

		// void* buf = glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB);

		for(int i=0; i<m_numTriangles; i++) {
		  unsigned short* s = (unsigned short*)buf;
		  s+=(i*3);
		  CHAI_DEBUG_PRINT("%d,%d,%d\n",(int)(s[0]),(int)(s[1]),(int)(s[2]));
		}
		CHAI_DEBUG_PRINT("\n");

		// UnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);

		delete [] buf;

		glDrawElements(GL_TRIANGLES,m_numTriangles*3,m_useLongIndices?GL_UNSIGNED_INT:GL_UNSIGNED_SHORT,0);
		*/
	}

	// Do the drawing (one triangle at a time)
	else {

		// render all active triangles
		glBegin(GL_TRIANGLES);
		unsigned int numItems = m_numTriangles;

		unsigned int numCurrentItems = m_triangles.size();

		for (i = 0; i < numItems; i++)
		{
			bool allocated = m_triangles[i].m_allocated;
			if (allocated == false) continue;

			unsigned int index0 = m_triangles[i].m_indexVertex0;
			unsigned int index1 = m_triangles[i].m_indexVertex1;
			unsigned int index2 = m_triangles[i].m_indexVertex2;

			glArrayElement(index0);
			glArrayElement(index1);
			glArrayElement(index2);
		}
		glEnd();
	}

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, (unsigned int)NULL);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, (unsigned int)NULL);

	// Restore OpenGL settings to reasonable defaults
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0, 0);

	// Turn off any array variables I might have turned on...
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


//===========================================================================
/*!
Finalize this mesh (indicates that vertices will not move around after this)
(see cGenericObject::finalize).  This is where all the vbo-related structures
get created, and data is copied from the cMesh vertex and triangle arrays.

\fn         void cVBOMesh::finalize(const bool a_affectChildren)
\param      a_affectChildren    Should this call be passed recursively to my children?
*/
//===========================================================================
void cVBOMesh::finalize(const bool a_affectChildren)
{

	unsigned int i;

	// Let my proxy worry about this...
	if (m_renderingProxy) {
		return;
	}

	// Finalize any children that need to be finalized
	cGenericObject::finalize(a_affectChildren);

	// Make sure vertex buffers are actually supported...  
	if (vbo_initialized == false) {

		// Try to initialize vertex buffers...
		InitVBO();

		// If we _still_ haven't initialized vbo's, they're not
		// supported...
		if (vbo_initialized == false) {
#ifdef DEBUG_VBO_OUTPUT
			CHAI_DEBUG_PRINT("Could not initialize VBO's...\n");
#endif
			m_renderFromVBO = false;
			return;
		}

	}

	// Nuke our display list if we have one; vertex pointers can't
	// be used in display lists...

	// Delete any allocated display lists
	if (m_displayList != -1) glDeleteLists(m_displayList, 1);
	m_displayList = -1;
	m_useDisplayList = false;

	// Clear any "old" vertex buffers we've allocated...
	if (m_activeBufferObjects) {
		unfinalize(a_affectChildren);
	}

	std::vector<cVertex>* vertex_vector = this->pVertices();

	unsigned int vcount = vertex_vector->size();
	m_maxVertex = vcount - 1;

	if (vcount == 0) {
		// Don't create VBO's for empty meshes
		m_renderFromVBO = false;
		return;
	}

	if (m_triangles.size() == 0) {
		// Don't create VBO's for empty meshes
		m_renderFromVBO = false;
		return;
	}

	// Point of no return...
	m_renderFromVBO = true;

	// cVertex* pVertices = &(m_vertices[0]);
	cVertex* pVertices = (cVertex*) &((*vertex_vector)[0]);

	// Create a contiguous vertex array and a VBO for it
	vbo_vertex_type* vbuf = (vbo_vertex_type*)malloc(vcount * sizeof(vbo_vertex_type) * 3);

	// Create a contiguous normal array and a VBO for it
	vbo_normal_type* nbuf = (vbo_normal_type*)malloc(vcount * sizeof(vbo_normal_type) * 3);

	vbo_texcoord_type* tbuf = 0;
	vbo_color_type* cbuf = 0;

	// We always have vertices and normals
	m_activeBufferObjects = MASK(VBO_FLAG_VERTEX) | MASK(VBO_FLAG_NORMAL);

	// Create a contiguous texcoord array and a VBO for it if necessary
	if (m_useTextureMapping) {
		tbuf = (vbo_texcoord_type*)malloc(vcount * sizeof(vbo_texcoord_type) * 2);
		m_activeBufferObjects |= MASK(VBO_FLAG_TEXCOORD);
	}

	// Create a contiguous color array and a VBO for it if necessary
	if (m_useVertexColors) {
		cbuf = (vbo_color_type*)malloc(vcount * sizeof(vbo_color_type) * 4);
		m_activeBufferObjects |= MASK(VBO_FLAG_COLOR);
	}

	cVertex* curVertex = pVertices;

	// Copy all vertex information into those arrays
	for (i = 0; i < vcount; i++, curVertex++) {

		vbuf[i * 3 + 0] = curVertex->getPos().x;
		vbuf[i * 3 + 1] = curVertex->getPos().y;
		vbuf[i * 3 + 2] = curVertex->getPos().z;

		nbuf[i * 3 + 0] = curVertex->getNormal().x;
		nbuf[i * 3 + 1] = curVertex->getNormal().y;
		nbuf[i * 3 + 2] = curVertex->getNormal().z;

		if (m_activeBufferObjects & MASK(VBO_FLAG_TEXCOORD)) {
			tbuf[i * 2 + 0] = curVertex->getTexCoord().x;
			tbuf[i * 2 + 1] = curVertex->getTexCoord().y;
		}

		// If we use float colors, this needs to do scaling...
		if (m_activeBufferObjects & MASK(VBO_FLAG_COLOR)) {
			cbuf[i * 4 + 0] = (vbo_color_type)((curVertex->m_color.pColor())[0]);
			cbuf[i * 4 + 1] = (vbo_color_type)((curVertex->m_color.pColor())[1]);
			cbuf[i * 4 + 2] = (vbo_color_type)((curVertex->m_color.pColor())[2]);
			cbuf[i * 4 + 3] = (vbo_color_type)((curVertex->m_color.pColor())[3]);
		}
	} // For all vertices

	// Create vertex buffers and ship out all our data
	glGenBuffersARB(1, &(m_vertexBuffers[VBO_FLAG_VERTEX]));
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, (m_vertexBuffers[VBO_FLAG_VERTEX]));
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, vcount * sizeof(vbo_vertex_type) * 3, vbuf, GL_STATIC_DRAW_ARB);
	free(vbuf);

	glGenBuffersARB(1, &(m_vertexBuffers[VBO_FLAG_NORMAL]));
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, (m_vertexBuffers[VBO_FLAG_NORMAL]));
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, vcount * sizeof(vbo_normal_type) * 3, nbuf, GL_STATIC_DRAW_ARB);
	free(nbuf);

	if (m_activeBufferObjects & MASK(VBO_FLAG_TEXCOORD)) {
		glGenBuffersARB(1, &(m_vertexBuffers[VBO_FLAG_TEXCOORD]));
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, (m_vertexBuffers[VBO_FLAG_TEXCOORD]));
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, vcount * sizeof(vbo_texcoord_type) * 2, tbuf, GL_STATIC_DRAW_ARB);
		free(tbuf);
	}

	if (m_activeBufferObjects & MASK(VBO_FLAG_COLOR)) {
		glGenBuffersARB(1, &(m_vertexBuffers[VBO_FLAG_COLOR]));
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, (m_vertexBuffers[VBO_FLAG_COLOR]));
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, vcount * sizeof(vbo_color_type) * 4, cbuf, GL_STATIC_DRAW_ARB);
		free(cbuf);
	}

	// Holy crap did weird stuff happen when I forgot to do this...
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	unsigned int ntris = m_triangles.size();
	m_numTriangles = ntris;

	// Do we need 32-bit indices?
	m_useLongIndices = (vcount > 65535);

#ifndef USE_INDEX_BUFFER
	return;
#endif

	// How big is one index?
	int index_size = (m_useLongIndices) ? 4 : 2;

	// Create a contiguous array...
	char* tribuf = (char*)(malloc(ntris * 3 * index_size));
	unsigned int* ltris = (unsigned int*)(tribuf);
	unsigned short* stris = (unsigned short*)(tribuf);
	int curtriangleindex = 0;

	cTriangle* curtri = &(m_triangles[0]);

	// Fill the array in...
	for (i = 0; i < ntris; i++, curtri++) {
		bool allocated = curtri->m_allocated;
		if (allocated == false) continue;
		if (m_useLongIndices) {
			ltris[curtriangleindex * 3 + 0] = curtri->getIndexVertex0();
			ltris[curtriangleindex * 3 + 1] = curtri->getIndexVertex1();
			ltris[curtriangleindex * 3 + 2] = curtri->getIndexVertex2();
		}
		else {
			stris[curtriangleindex * 3 + 0] = curtri->getIndexVertex0();
			stris[curtriangleindex * 3 + 1] = curtri->getIndexVertex1();
			stris[curtriangleindex * 3 + 2] = curtri->getIndexVertex2();
		}
		curtriangleindex++;
	}

	m_numTriangles = curtriangleindex;

	// Generate a buffer and give GL our data
	glGenBuffersARB(1, &(m_vertexBuffers[VBO_FLAG_INDEX]));
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, (m_vertexBuffers[VBO_FLAG_INDEX]));
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_numTriangles * 3 * index_size, tribuf, GL_STATIC_DRAW_ARB);

	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

	m_activeBufferObjects |= MASK(VBO_FLAG_INDEX);

	free(tribuf);

#ifdef DEBUG_VBO_OUTPUT
	CHAI_DEBUG_PRINT("Successfully created a vertex buffer set...\n");
#endif

}


//===========================================================================
/*!
Disable vertex buffers and behave like a typical cMesh.

\fn         void cVBOMesh::unfinalize(const bool a_affectChildren)
\param      a_affectChildren    Should this call be passed recursively to my children?
*/
//===========================================================================
void cVBOMesh::unfinalize(const bool a_affectChildren) {

	cGenericObject::unfinalize();

	if (m_activeBufferObjects == 0) return;

	clean_vertex_buffers();

	// We're no longer rendering from vertex buffers unless explicitly re-told
	// to do so...
	m_renderFromVBO = false;
}


//===========================================================================
/*!
Clean up vertex buffer data.

\fn         void cVBOMesh::clean_vertex_buffers()
*/
//===========================================================================
void cVBOMesh::clean_vertex_buffers() {

	if (m_activeBufferObjects == 0) return;

	// Delete vertex buffers
	for (int i = 0; i < NUM_VBO_FLAGS; i++) {
		if (m_activeBufferObjects & (1 << i))
			glDeleteBuffersARB(1, &(m_vertexBuffers[i]));
	}

	m_activeBufferObjects = 0;
}


//===========================================================================
/*!
Called when the context is deleted, allows this class to re-build VBO's.  See
cGenericObject::onDisplayReset

\fn         void cVBOMesh::onDisplayReset(const bool a_affectChildren)
\param      a_affectChildren    Should this call be passed recursively to my children?
*/
//===========================================================================
void cVBOMesh::onDisplayReset(const bool a_affectChildren)
{

	vbo_mesh_parent_mesh_type::onDisplayReset(a_affectChildren);

	if (m_renderFromVBO) {
		unfinalize();
		m_renderFromVBO = true;

		// finalize() will be called at the beginning of the next rendering pass;
		// if we do it here, vertex buffer indices will get totally whack
	}

}


//===========================================================================
/*!
Called to enable or disable "background blend mode", in which an object can be
rendered through other opaque objects as if they were partially transparent.

\fn         cVBOMesh::setBackgroundBlend(bool enable, bool includeChildren)
\param      enable             Should background-blend mode be enabled?
\param      includeChildren    Should this call be passed recursively to my children?
*/
//===========================================================================
void cVBOMesh::setBackgroundBlend(bool enable, bool includeChildren)
{

	m_backgroundBlend = enable;

	if (includeChildren == false) return;

	unsigned int i;
	for (i = 0; i < m_children.size(); i++) {
		cGenericObject *nextObject = m_children[i];
		cVBOMesh* nextMesh = dynamic_cast<cVBOMesh*>(nextObject);
		if (nextMesh) nextMesh->setBackgroundBlend(enable, true);
	}

	cVBOMesh* proxy = dynamic_cast<cVBOMesh*>(m_renderingProxy);
	if (proxy) proxy->setBackgroundBlend(enable, true);

}



/*******
* Everything after this is global stuff required to initialize vertex buffering.
*******/

// GL extension function pointers

PFNGLBINDBUFFERARBPROC           glBindBufferARB = NULL;
PFNGLDELETEBUFFERSARBPROC        glDeleteBuffersARB = NULL;
PFNGLGENBUFFERSARBPROC           glGenBuffersARB = NULL;
PFNGLISBUFFERARBPROC             glIsBufferARB = NULL;
PFNGLBUFFERDATAARBPROC           glBufferDataARB = NULL;
PFNGLBUFFERSUBDATAARBPROC        glBufferSubDataARB = NULL;
PFNGLGETBUFFERSUBDATAARBPROC     glGetBufferSubDataARB = NULL;
PFNGLMAPBUFFERARBPROC            glMapBufferARB = NULL;
PFNGLUNMAPBUFFERARBPROC          glUnmapBufferARB = NULL;
PFNGLGETBUFFERPARAMETERIVARBPROC glGetBufferParameterivARB = NULL;
PFNGLGETBUFFERPOINTERVARBPROC    glGetBufferPointervARB = NULL;
PFNGLDRAWRANGEELEMENTSPROC       glDrawRangeElements = NULL;

//! A global function used to initialize vertex buffers
//!
//! Returns true if successful, false if vbo's are not supported or could not
//! be initialized.
bool InitVBO()
{

	if (vbo_initialized) return true;

	bool supported = IsExtensionSupported("GL_ARB_vertex_buffer_object");

	if (supported == false) {
#ifdef DEBUG_VBO_OUTPUT
		CHAI_DEBUG_PRINT("Extension not supported\n");
#endif
		return false;
	}

	glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
	glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");
	glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
	glIsBufferARB = (PFNGLISBUFFERARBPROC)wglGetProcAddress("glIsBufferARB");
	glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
	glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)wglGetProcAddress("glBufferSubDataARB");
	glGetBufferSubDataARB = (PFNGLGETBUFFERSUBDATAARBPROC)wglGetProcAddress("glGetBufferSubDataARB");
	glMapBufferARB = (PFNGLMAPBUFFERARBPROC)wglGetProcAddress("glMapBufferARB");
	glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)wglGetProcAddress("glUnmapBufferARB");
	glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)wglGetProcAddress("glGetBufferParameterivARB");
	glGetBufferPointervARB = (PFNGLGETBUFFERPOINTERVARBPROC)wglGetProcAddress("glGetBufferPointervARB");
	glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)wglGetProcAddress("glDrawRangeElements");

	if (!glBindBufferARB || !glDeleteBuffersARB || !glGenBuffersARB ||
		!glIsBufferARB || !glBufferDataARB || !glBufferSubDataARB ||
		!glGetBufferSubDataARB || !glMapBufferARB || !glUnmapBufferARB ||
		!glGetBufferParameterivARB || !glGetBufferPointervARB || !glDrawRangeElements) {
#ifdef DEBUG_VBO_OUTPUT
		CHAI_DEBUG_PRINT("Could not initialize vertex buffers... %x %x\n", glBindBufferARB, glDeleteBuffersARB);
#endif
		return false;
	}

#ifdef DEBUG_VBO_OUTPUT		
	CHAI_DEBUG_PRINT("Initialized vertex buffers...\n");
#endif
	vbo_initialized = true;

	return true;
}


//! A global function used to test for OpenGL extension support
//!
//! Returns true if the extension is supported, false otherwise
bool IsExtensionSupported(char* szTargetExtension)
{

	const unsigned char *pszExtensions = NULL;
	const unsigned char *pszStart;
	unsigned char *pszWhere, *pszTerminator;

	// Extension names should not have spaces
	pszWhere = (unsigned char *)strchr(szTargetExtension, ' ');
	if (pszWhere || *szTargetExtension == '\0')
		return false;

	// Get Extensions String
	pszExtensions = glGetString(GL_EXTENSIONS);

	if (pszExtensions == 0) {
#ifdef DEBUG_VBO_OUTPUT
		CHAI_DEBUG_PRINT("Could not find extensions string... have you initialized GL yet?\n");
#endif
		return false;
	}

	// Search The Extensions String For An Exact Copy
	pszStart = pszExtensions;
	for (;;)
	{
		pszWhere = (unsigned char *)strstr((const char *)pszStart, szTargetExtension);
		if (!pszWhere)
			break;
		pszTerminator = pszWhere + strlen(szTargetExtension);
		if (pszWhere == pszStart || *(pszWhere - 1) == ' ')
			if (*pszTerminator == ' ' || *pszTerminator == '\0')
				return true;
		pszStart = pszTerminator;
	}
	return false;
}


