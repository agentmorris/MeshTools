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

#ifndef _TETGEN_LOADER_H_
#define _TETGEN_LOADER_H_

#include "CTetMesh.h"
#include "CPanel.h"
#include <list>
#include <map>

#define NUM_DEFAULT_MATERIALS 6

typedef enum {
	FACE_FORMAT_FACE_FILE = 0, FACE_FORMAT_SMESH_FILE, FACE_FORMAT_GMSH_FILE
} face_formats;

typedef enum {
	NODE_FORMAT_NODE_FILE = 0, NODE_FORMAT_GMSH_FILE
} node_formats;

typedef enum {
	ELEMENT_FORMAT_ELE_FILE = 0, ELEMENT_FORMAT_GMSH_FILE
} element_formats;

class cTetGenLoader {

public:
	cTetGenLoader();
	virtual ~cTetGenLoader() {};

	cMesh* load_tetgen_file(cWorld* world, const char* filename, const cMesh* mesh_factory = 0);

	cPanel* build_attribute_key(cWorld* world, int& num_attributes);

	cMaterial default_materials[NUM_DEFAULT_MATERIALS];

	static bool m_remove_small_tets_at_import;
	static float m_small_import_tet_volume;

private:
	cTetMesh* current_mesh;
	bool loaded_face_file;
	bool loaded_element_file;

	std::list<int> face_attributes;
	std::list<int> tet_attributes;

	// Return true for success
	bool load_element_file(const char* filename);
	bool load_node_file(const char* filename);
	bool load_face_file(const char* filename);
	bool load_smesh_file(const char* filename);
	bool load_gmsh_file(const char* filename);

	bool read_element_file(FILE* f, const char* filename, int element_format = ELEMENT_FORMAT_ELE_FILE);
	bool read_face_file(FILE* f, const char* filename, int face_format = FACE_FORMAT_FACE_FILE);
	bool read_node_file(FILE* f, const char* filename, int node_format = NODE_FORMAT_NODE_FILE);

	// Data for pre-counted element counts
	unsigned int m_precounted_tets;
	unsigned int m_precounted_faces;

	// For gmsh files, we need to map points that appeared in the file to 
	// points that we load
	std::map<unsigned int, unsigned int> gmsh_point_map;

	// Possible filenames that we might try to load given a root filename
	char base_filename[_MAX_PATH];
	char node_filename[_MAX_PATH];
	char element_filename[_MAX_PATH];
	char face_filename[_MAX_PATH];
	char smesh_filename[_MAX_PATH];
	char gmsh_filename[_MAX_PATH];

	// Did our vertices appear to start counting from 1?
	int one_indexed_points;
};
#endif
