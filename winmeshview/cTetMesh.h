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

#ifndef _TETMESH_H_
#define _TETMESH_H_

// Tets smaller than this at simln-time will be considered
// problematic...
#define SMALL_TET_VOLUME 0.00001

// Should we remove very small tets during the import process?
#define DEFAULT_REMOVE_SMALL_TETS_AT_IMPORT true

// Tets smaller than this at import-time will be considered
// problematic...
#define DEFAULT_SMALL_IMPORT_TET_VOLUME 0.0001

#include "CVBOMesh.h"

// Triangle orientation consistent with tetgen files, deduced from:
//
// http://tetgen.berlios.de/figs/fffigs/10-nodes.gif
static const int tet_triangle_faces[4][3] =
{
  {0,1,3},
  {0,2,1},
  {1,2,3},
  {0,3,2}
};

#define PARENT_MESH_TYPE cVBOMesh

#define COMPUTE_FACE_CENTERS_AND_NORMALS 0

typedef enum TET_RENDER_MODES {
	TET_RENDER_MODE_WIREFRAME = 0,
	TET_RENDER_MODE_FILL,
	TET_RENDER_MODE_WIREFRAME_AND_FILL
};

class cTetMesh : public PARENT_MESH_TYPE {

public:
	cTetMesh(cWorld* a_world);
	virtual ~cTetMesh();

	unsigned int m_nTets;

	// 4 indices for each tet
	unsigned int* m_tets;

	// This will be -1 if this mesh is empty and has no attribute-valued vertices
	int m_attributeValue;
	int m_nAttributes;

	// How many markers are there per vertex?
	int m_nVertexBoundaryMarkers;

	// An array of nvertices*nmarkers
	int* m_vertexBoundaryMarkers;

	virtual cMesh* createMesh() { return new cTetMesh(m_parentWorld); }
	unsigned int getNumTets(bool a_includeChildren = false) const;
	virtual void renderMesh(const int a_renderMode = 0);

	virtual vector<cVertex>* pVertices() {
		if (vertex_array_holder == 0) return (&m_vertices);
		else return vertex_array_holder->pVertices();
	}

	inline virtual cMesh* createMesh() const { return new cTetMesh(m_parentWorld); }

	cVector3d* face_normals;
	cVector3d* face_centers;
	cMesh* vertex_array_holder;

	// From the TET_RENDER_MODES enumeration above
	int m_renderMode;

	// Recursively set the render mode
	void setRenderMode(int a_renderMode, bool a_includeChildren = false);

	// Recursively make triangle face orientations match their
	// vertex normals
	virtual void fixTriangleOrientations(bool a_affectChildren);

	// This tells us what scale/offset was applied to build this mesh  
	float mesh_prescale;
	cVector3d mesh_preoffset;
	cVector3d mesh_prezero;

};

#endif
