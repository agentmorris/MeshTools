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

#ifndef _PLY_LOADER_H_
#define _PLY_LOADER_H_

#include "CMesh.h"

class cPlyLoader {

public:
	cPlyLoader() {};
	virtual ~cPlyLoader() {};

	// Returns true if the file is loaded successfully
	bool load_ply_file(cMesh* mesh, cWorld* world, const char* filename);

	cMesh* current_mesh;

	cVector3d current_vertex_pos;

	double current_face_index;
	int current_face_expected_vertices;
	std::vector<int> current_face_elements;

};
#endif
