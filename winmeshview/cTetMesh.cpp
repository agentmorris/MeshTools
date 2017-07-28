/******
*
* Written by Dan Morris
* dmorris@cs.stanford.edu
* http://cs.stanford.edu/~dmorris
*
* You can do anything you want with this file as long as this header
* stays on it and I am credited when it's appropriate.
*
******/

#include "cTetMesh.h"
#include "CTriangle.h"
#include "CVertex.h"

#include <conio.h>

void cTetMesh::renderMesh(const int a_renderMode) {

	glShadeModel(GL_SMOOTH);

	if (m_triangleMode != GL_FILL) {
		PARENT_MESH_TYPE::renderMesh(a_renderMode);
		return;
	}

	if (m_renderMode == TET_RENDER_MODE_FILL || m_renderMode == TET_RENDER_MODE_WIREFRAME_AND_FILL) {
		int oldTriMode = m_triangleMode;
		setWireMode(false, false);
		PARENT_MESH_TYPE::renderMesh(a_renderMode);
		m_triangleMode = oldTriMode;
	}

	if (m_renderMode == TET_RENDER_MODE_WIREFRAME || m_renderMode == TET_RENDER_MODE_WIREFRAME_AND_FILL) {

		int oldTriMode = m_triangleMode;
		bool oldUseVertexColors = m_useVertexColors;
		bool oldUseMaterialProperties = m_useMaterialProperty;
		cMaterial oldMat = m_material;

		m_useVertexColors = false;
		m_useMaterialProperty = true;
		m_material.m_ambient = m_material.m_diffuse = cColorf(0.5, 0.5, 0.5, 1.0);

		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(0.0, -1.0);

		setWireMode(true, false);
		glLineWidth(2.0);
		PARENT_MESH_TYPE::renderMesh(a_renderMode);

		glDisable(GL_POLYGON_OFFSET_LINE);

		m_useVertexColors = oldUseVertexColors;
		m_useMaterialProperty = oldUseMaterialProperties;
		m_material = oldMat;
		m_triangleMode = oldTriMode;

	}

	glShadeModel(GL_SMOOTH);

}


cTetMesh::cTetMesh(cWorld* a_world) : PARENT_MESH_TYPE(a_world) {

	face_normals = 0;
	face_centers = 0;

	m_renderMode = TET_RENDER_MODE_WIREFRAME_AND_FILL;

	m_nTets = 0;
	m_tets = 0;

	m_attributeValue = -1;
	m_nAttributes = 0;

	m_nVertexBoundaryMarkers = 0;
	m_vertexBoundaryMarkers = 0;

	vertex_array_holder = 0;

}

cTetMesh::~cTetMesh() {
	if (m_tets) delete[] m_tets;
	if (face_centers) delete[] face_centers;
	if (face_normals) delete[] face_normals;
}


void cTetMesh::setRenderMode(int a_renderMode, bool a_includeChildren) {

	m_renderMode = a_renderMode;

	// optionally count the number of triangles in my children
	if (a_includeChildren)
	{
		unsigned int i, numItems;
		numItems = m_children.size();
		for (i = 0; i < numItems; i++)
		{
			cGenericObject *nextObject = m_children[i];

			// check if nextobject is a tet mesh.
			cTetMesh *nextMesh = dynamic_cast<cTetMesh*>(nextObject);
			if (nextMesh)
			{
				nextMesh->setRenderMode(a_renderMode, true);
			}
		}
	}
}


unsigned int cTetMesh::getNumTets(bool a_includeChildren) const {

	// get the number of tets of current object
	unsigned int numTets = m_nTets;

	// optionally count the number of triangles in my children
	if (a_includeChildren)
	{
		unsigned int i, numItems;
		numItems = m_children.size();
		for (i = 0; i < numItems; i++)
		{
			cGenericObject *nextObject = m_children[i];

			// check if nextobject is a tet mesh.
			cTetMesh *nextMesh = dynamic_cast<cTetMesh*>(nextObject);
			if (nextMesh)
			{
				numTets += nextMesh->getNumTets(a_includeChildren);
			}
		}
	}

	return (numTets);

}

void cTetMesh::fixTriangleOrientations(bool a_affectChildren) {

	if (a_affectChildren) {
		unsigned int i, numItems;
		numItems = m_children.size();
		for (i = 0; i < numItems; i++)
		{
			cGenericObject *nextObject = m_children[i];

			// check if nextobject is a tet mesh.
			cTetMesh *nextMesh = dynamic_cast<cTetMesh*>(nextObject);
			if (nextMesh)
			{
				nextMesh->fixTriangleOrientations(true);
			}
		}
	}

	std::vector<cTriangle>* tri_vector = this->pTriangles();
	unsigned int ntriangles = tri_vector->size();
	if (ntriangles == 0) return;
	cTriangle* tri_array = (cTriangle*) &((*tri_vector)[0]);

#if COMPUTE_FACE_CENTERS_AND_NORMALS
	face_normals = new cVector3d[ntriangles];
	face_centers = new cVector3d[ntriangles];
#endif

	std::vector<cVertex>* vertex_vector = this->pVertices();
	cVertex* vertex_array = (cVertex*) &((*vertex_vector)[0]);

	for (unsigned int i = 0; i < ntriangles; i++) {
		cTriangle* t = tri_array + i;
		cVector3d average_normal(0, 0, 0);

		// Compute the average vertex normal
		cVertex* v0 = vertex_array + t->getIndexVertex0();
		cVertex* v1 = vertex_array + t->getIndexVertex1();
		cVertex* v2 = vertex_array + t->getIndexVertex2();
		average_normal += v0->getNormal();
		average_normal += v1->getNormal();
		average_normal += v2->getNormal();
		average_normal /= 3;

		// Now compute the normal based on the triangle face
		cVector3d p0 = v0->getPos();
		cVector3d p1 = v1->getPos();
		cVector3d p2 = v2->getPos();
		cVector3d e0 = p1 - p0;
		cVector3d e1 = p2 - p0;
		cVector3d tri_normal = cCross(e0, e1);
		tri_normal.normalize();

#if COMPUTE_FACE_CENTERS_AND_NORMALS
		face_normals[i] = tri_normal;
		face_centers[i] =
			(v0->getPos() +
				v1->getPos() +
				v2->getPos()
				);
		face_centers[i] /= 3.0;
#endif

		double result = (cDot(average_normal, tri_normal));

		// If they don't point the same way...
		if (result < 0) {

			// Swap two vertices
			t->setVertices(t->getIndexVertex0(), t->getIndexVertex2(), t->getIndexVertex1());
		}

	}

}
