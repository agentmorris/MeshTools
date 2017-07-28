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

#include "ply_loader.h"
#include "rply-1.01/rply.h"
#include <conio.h>

static int vertex_cb(p_ply_argument argument) {

	cPlyLoader* loader;
	cMesh* mesh;
	long uservalue;
	p_ply_element element;
	long index;

	// If we need access to the element
	ply_get_argument_element(argument, &element, &index);

	// The user pointer tells us where the mesh is
	ply_get_argument_user_data(argument, ((void**)(&loader)), &uservalue);
	mesh = loader->current_mesh;

	double value = ply_get_argument_value(argument);

	// _cprintf("Got a vertex coordinate %ld,%ld: %lf\n",index,uservalue,value);
	loader->current_vertex_pos[uservalue] = value;

	// If this is the end of a vertex
	if (uservalue == 2) mesh->newVertex(loader->current_vertex_pos);
	// _cprintf("Created %d vertices\n",mesh->getNumVertices(0));

	return 1;
}


static int face_cb(p_ply_argument argument) {

	cPlyLoader* loader;
	cMesh* mesh;
	long uservalue;
	p_ply_element element;
	long index;

	ply_get_argument_element(argument, &element, &index);

	ply_get_argument_user_data(argument, ((void**)(&loader)), &uservalue);
	mesh = loader->current_mesh;

	double value = ply_get_argument_value(argument);

	// _cprintf("Got a face index %ld,%ld: %lf\n",index,uservalue,value);

	// If this is a new face
	if (index != loader->current_face_index) {
		loader->current_face_index++;
		loader->current_face_expected_vertices = (int)value;
		// _cprintf("New face with %d vertices\n",loader->current_face_expected_vertices);

	}

	else {
		loader->current_face_elements.push_back((int)value);
		// _cprintf("Found %d vertices\n",loader->current_face_elements.size());
	}

	// If the face is done
	if (loader->current_face_elements.size() ==
		loader->current_face_expected_vertices) {

		// Tessellate the face
		int numvertices = loader->current_face_elements.size();
		int last_vertex_start = numvertices - 2;
		int triangle_start = 1;
		while (triangle_start <= last_vertex_start) {
			int v0 = loader->current_face_elements[0];
			int v1 = loader->current_face_elements[triangle_start];
			int v2 = loader->current_face_elements[triangle_start + 1];
			// _cprintf("Triangle for face %d: %d,%d,%d\n",index,v0,v1,v2);
			mesh->newTriangle(v0, v1, v2);
			triangle_start++;
		}

		loader->current_face_elements.clear();
	}

	return 1;
}


bool cPlyLoader::load_ply_file(cMesh* mesh, cWorld* world, const char* filename) {

	if (filename == 0) return 0;

	long nvertices, nfaces;

	// Open the ply file
	p_ply ply = ply_open(filename, NULL);
	if (!ply) {
		_cprintf("Could not open ply file %s\n", filename);
		return 0;
	}

	if (!ply_read_header(ply)) {
		_cprintf("Error reading header for ply file %s\n", filename);
		return 0;
	}

	// Set callbacks
	this->current_mesh = mesh;
	this->current_face_index = -1;

	// Prepare the mesh for having vertices stuck into it

	void* ptr = (void*)this;

	nvertices = ply_set_read_cb(ply, "vertex", "x", vertex_cb, ptr, 0);
	nvertices = ply_set_read_cb(ply, "vertex", "y", vertex_cb, ptr, 1);
	nvertices = ply_set_read_cb(ply, "vertex", "z", vertex_cb, ptr, 2);

	nfaces = ply_set_read_cb(ply, "face", "vertex_indices", face_cb, ptr, 0);
	if (nfaces == 0) {
		nfaces = ply_set_read_cb(ply, "face", "vertex_index", face_cb, ptr, 0);
	}

	// Set up the 
	// Read the file
	// _cprintf("Reading %ld vertices, %ld faces\n", nvertices, nfaces);
	if (!ply_read(ply)) {
		ply_close(ply);
		_cprintf("Error reading ply file %s\n", filename);
		return false;
	}

	// Close the file handle
	ply_close(ply);

	mesh->computeAllNormals(0);
	mesh->useCulling(0, 1);
	mesh->useColors(0, 1);

	return true;

}

