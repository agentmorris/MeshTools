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

#include "tetgen_loader.h"
#include <conio.h>
#include "cTetMesh.h"
#include <malloc.h>
#include "CVertex.h"
#include "CTriangle.h"
#include "cLabelPanel.h"

// For filename utils
#include "CImageLoader.h"

#include <map>

#define GMSH_ELEMENT_TYPE_TRIANGLE 2
#define GMSH_ELEMENT_TYPE_TET      4

typedef enum {
	TETGEN_FILETYPE_NODE = 0,
	TETGEN_FILETYPE_ELEMENT,
	TETGEN_FILETYPE_FACE,
	TETGEN_FILETYPE_SMESH,
	TETGEN_FILETYPE_GMSH_1
	// ANODE is not a separate file type
};


bool cTetGenLoader::m_remove_small_tets_at_import = DEFAULT_REMOVE_SMALL_TETS_AT_IMPORT;
float cTetGenLoader::m_small_import_tet_volume = DEFAULT_SMALL_IMPORT_TET_VOLUME;

cTetGenLoader::cTetGenLoader() {

	current_mesh = 0;

	// Material 0: red
	default_materials[0].m_ambient.set(1.0, 0.2, 0.2, 1.0);
	default_materials[0].m_diffuse.set(1.0, 0.2, 0.2, 1.0);

	// Material 1: green
	default_materials[1].m_ambient.set(0.2, 1.0, 0.2, 1.0);
	default_materials[1].m_diffuse.set(0.2, 1.0, 0.2, 1.0);

	// Material 2: blue
	default_materials[2].m_ambient.set(0.2, 0.2, 1.0, 1.0);
	default_materials[2].m_diffuse.set(0.2, 0.2, 1.0, 1.0);

	// Material 3: yellow
	default_materials[3].m_ambient.set(1.0, 1.0, 0.2, 1.0);
	default_materials[3].m_diffuse.set(1.0, 1.0, 0.2, 1.0);

	// Material 4: purple
	default_materials[4].m_ambient.set(1.0, 0.2, 1.0, 1.0);
	default_materials[4].m_diffuse.set(1.0, 0.2, 1.0, 1.0);

	// Material 5: aqua
	default_materials[5].m_ambient.set(0.2, 1.0, 1.0, 1.0);
	default_materials[5].m_diffuse.set(0.2, 1.0, 1.0, 1.0);
}

cMesh* cTetGenLoader::load_tetgen_file(cWorld* world, const char* filename, const cMesh* mesh_factory) {

	// Determine the type...
	loaded_face_file = false;
	loaded_element_file = false;
	face_attributes.clear();
	tet_attributes.clear();

	// Get the extension
	char* extension = find_extension(filename);

	if (extension == 0) {
		_cprintf("Could not get extension for tetgen file %s\n", filename);
		return 0;
	}

	int type = -1;
	bool anode_file = false;
	if (strcmp(extension, "node") == 0 || strcmp(extension, "anode") == 0) {
		type = TETGEN_FILETYPE_NODE;
		if (strcmp(extension, "anode") == 0) anode_file = true;
	}
	else if (strcmp(extension, "ele") == 0) {
		type = TETGEN_FILETYPE_ELEMENT;
	}
	else if (strcmp(extension, "face") == 0) {
		type = TETGEN_FILETYPE_FACE;
	}
	else if (strcmp(extension, "smesh") == 0) {
		type = TETGEN_FILETYPE_SMESH;
	}
	else if (strcmp(extension, "msh") == 0) {
		type = TETGEN_FILETYPE_GMSH_1;
	}
	else {
		_cprintf("Unrecognized file extension %s for tetgen file %s\n", extension, filename);
		return 0;
	}

	strcpy(base_filename, filename);
	base_filename[extension - filename - 1] = '\0';

	if (anode_file) {
		FILE* f = fopen(filename, "r");
		if (f == 0) {
			_cprintf("Warning: could not open anode file to examine header...\n");
		}
		else {
			bool found_base_name = false;
			char buf[1000];
			char original_filename[_MAX_PATH];
			while (1) {
				char* cresult = fgets(buf, 1000, f);
				if (cresult == 0) break;
				int iresult = sscanf(buf, "# BASE_FILENAME %s", original_filename);
				if (iresult != 1) continue;
				else {
					found_base_name = true;
					_cprintf("Found base filename %s\n", original_filename);
					break;
				}
			}
			if (found_base_name == false) {
				_cprintf("Warning: could not find original filename for anode file %s\n", filename);
				_cprintf("Using base filename %s\n", base_filename);
			}
			else {

				// Okay, we found the original filename...
				/*
				char path[_MAX_PATH];
				bool foundpath = find_directory(path,filename);
				if (foundpath == 0) {
				  strcpy(base_filename,original_filename);
				}
				else {
				  sprintf(base_filename,"%s\\%s",path,original_filename);
				}
				*/

				// Discard the extension
				char* ext = find_extension(original_filename, true);
				if (ext) *ext = '\0';
				strcpy(base_filename, original_filename);

				_cprintf("Using full base pathname: %s\n", base_filename);
			}
			fclose(f);
		}

	} // if this is an anode file

	if (anode_file == false)
		sprintf(node_filename, "%s.%s", base_filename, "node");
	else
		// Anode files are always specified explicitly...
		strcpy(node_filename, filename);

	strcpy(element_filename, base_filename);
	strcpy(face_filename, base_filename);
	strcpy(smesh_filename, base_filename);
	strcpy(gmsh_filename, base_filename);

	// Face and ele filenames are the same for node and anode files
	strcat(element_filename, ".ele");
	strcat(face_filename, ".face");
	strcat(smesh_filename, ".smesh");
	strcat(gmsh_filename, ".msh");

	if (mesh_factory) {
		cMesh* m = mesh_factory->createMesh();
		current_mesh = (dynamic_cast<cTetMesh*>(m));
		if (current_mesh == 0) {
			_cprintf("Warning: could not cast supplied mesh type to cTetMesh...\n");
			delete m;
			current_mesh = new cTetMesh(world);
		}
		current_mesh->setParentWorld(world);
	}
	else {
		current_mesh = new cTetMesh(world);
	}

	if (type == TETGEN_FILETYPE_SMESH) {
		if (!(load_smesh_file(smesh_filename))) {
			_cprintf("Could not load smesh file %s\n", smesh_filename);
			delete current_mesh;
			return 0;
		}
	}

	else if (type == TETGEN_FILETYPE_GMSH_1) {
		if (!(load_gmsh_file(gmsh_filename))) {
			_cprintf("Could not load gmsh file %s\n", gmsh_filename);
			delete current_mesh;
			return 0;
		}
	}

	// Some combination of node/ele/face files
	else {

		// Load the node file no matter what the input is
		if (!(load_node_file(node_filename))) {
			_cprintf("Could not load node file %s\n", node_filename);
			delete current_mesh;
			return 0;
		}

		if (type == TETGEN_FILETYPE_FACE) {

			if (!(load_face_file(face_filename))) {
				_cprintf("Could not load face file %s\n", face_filename);
				delete current_mesh;
				return 0;
			}

			// Try to load the element file, but it's not required
			if (!(load_element_file(element_filename))) {
				_cprintf("Warning: could not load element file %s\n", element_filename);
			}

		} // If the caller supplied a face filename

		else if (type == TETGEN_FILETYPE_ELEMENT) {

			// Try to load the face file, but it's not required
			if (!(load_face_file(face_filename))) {
				_cprintf("Warning: could not load face file %s\n", face_filename);
			}

			if (!(load_element_file(element_filename))) {
				_cprintf("Could not load element file %s\n", element_filename);
				delete current_mesh;
				return 0;
			}

		} // If the caller supplied an element filename

		else if (type == TETGEN_FILETYPE_NODE) {

			// Try the face file first
			if (!(load_face_file(face_filename))) {
				_cprintf("Warning: could not load face file %s\n", face_filename);
			}

			if (!(load_element_file(element_filename))) {
				_cprintf("Warning: could not load element file %s\n", element_filename);
			}

			if (loaded_element_file == false && loaded_face_file == false) {
				_cprintf("Error: could not load face file or element file\n");
				delete current_mesh;
				return 0;
			}
		} // If the caller supplied a node filename

		else {
			_cprintf("Unrecognized mesh type in tetgen loader...\n");
			return 0;
		}

	}

	// TODO: which order should this happen in?

	_cprintf("Before removing redundancy, loaded mesh has %d vertices, %d faces, and %d tets\n",
		current_mesh->getNumVertices(true),
		current_mesh->getNumTriangles(true),
		current_mesh->getNumTets(true)
	);
	current_mesh->removeRedundantTriangles(true);

	current_mesh->fixTriangleOrientations(true);
	_cprintf("Computing normals...\n");
	current_mesh->computeAllNormals(true);
	_cprintf("Computed normals...\n");

	current_mesh->useCulling(true, true);
	current_mesh->useColors(false, true);
	current_mesh->useMaterial(true, true);
	current_mesh->useTexture(false, true);

	// In case we do turn on xparency later, use multipass...
	current_mesh->setTransparencyRenderMode(true, true);
	current_mesh->setTransparencyLevel(0.5, false, true);
	current_mesh->enableTransparency(false, true);

	_cprintf("Loaded mesh has %d vertices, %d faces, and %d tets\n",
		current_mesh->getNumVertices(true),
		current_mesh->getNumTriangles(true),
		current_mesh->getNumTets(true)
	);

	return current_mesh;

} // load_tetgen_file


bool cTetGenLoader::read_element_file(FILE* f, const char* filename, int element_format) {

	_cprintf("remove tets: %d, %f\n", (int)m_remove_small_tets_at_import, m_small_import_tet_volume);
	/*
	First line:
	  <# of tetrahedra> <nodes per tetrahedron> <# of attributes>
	Remaining lines list of # of tetrahedra:
	  <tetrahedron #> <node> <node> <node> <node> ... [attributes]
	*/

	char buf[1000];
	char* cresult;
	int nresult[3];

	// Get the header line
	if (element_format != ELEMENT_FORMAT_GMSH_FILE) {
		while (1) {

			cresult = fgets(buf, 1000, f);

			if (cresult == 0) {
				_cprintf("Error reading header line from file %s\n", filename);
				fclose(f);
				return false;
			}

			// Skip leading comments
			if (buf[0] != '#' && buf[0] != '\0' && buf[0] != '\n') break;

		}
	}

	unsigned int ntets, nodes_per_tet, nmarkers;

	if (element_format == ELEMENT_FORMAT_ELE_FILE) {

		cresult = strtok(buf, "\t ");
		nresult[0] = sscanf(cresult, "%u", &ntets);
		cresult = strtok(0, "\t ");
		nresult[1] = sscanf(cresult, "%u", &nodes_per_tet);
		cresult = strtok(0, "\t ");
		nresult[2] = sscanf(cresult, "%u", &nmarkers);

		if (nresult[0] == 0 || nresult[1] == 0 || nresult[2] == 0) {
			_cprintf("Error reading header from element file %s\n", filename);
			fclose(f);
			return false;
		}

	}

	else if (element_format == ELEMENT_FORMAT_GMSH_FILE) {

		nodes_per_tet = 4;

		// TODO: handle markers in gmsh files
		// nmarkers = 1;
		nmarkers = 0;
		ntets = m_precounted_tets;
	}

	else {
		_cprintf("Unrecognized node file format...\n");
	}

	_cprintf("Loading %d %d-node tets with %d boundary markers\n",
		ntets, nodes_per_tet, nmarkers);

	// TODO: create submesh map if attributes are present in the _node_ file
	std::map<int, int> boundary_to_mesh_map;
	std::vector<cTetMesh*> child_meshes;
	cTetMesh* top_level_mesh = this->current_mesh;

	// Always ignore tet markers if face markers are available
	if (face_attributes.size() != 0) nmarkers = 0;

	// TODO: make this available externally
	bool force_no_tet_attributes = true;
	if (force_no_tet_attributes) nmarkers = 0;

	// Allocate tets
	if (nmarkers == 0) {
		top_level_mesh->m_nTets = ntets;
		top_level_mesh->m_tets = new unsigned int[nodes_per_tet*ntets];
		if (loaded_face_file == false)
			top_level_mesh->setMaterial(default_materials[0]);
	}

	// Allocate faces if necessary
	if (loaded_face_file == 0 && nmarkers == 0) {
		unsigned int nfaces = ntets * 4;
		_cprintf("Reserving %d faces...\n", nfaces);
		top_level_mesh->pTriangles()->reserve(nfaces);
		cTriangle t(0, 0, 0, 0);
		top_level_mesh->pTriangles()->resize(nfaces, t);
		top_level_mesh->setMaterial(default_materials[0]);
	}

	bool error = false;

	unsigned int ntets_read = 0;
	unsigned int line_number = 0;

	int* markers = (int*)malloc(sizeof(int)*nmarkers);
	unsigned int* tet_indices = (unsigned int*)malloc(sizeof(unsigned int)*nodes_per_tet);

	int one_indexed_tets = 0;

	// Stores the first tet # for gmsh files
	unsigned int tet_offset = 0;

	while (1) {

		line_number++;
		char raw_line[1000];
		cresult = fgets(buf, 1000, f);
		if (cresult == 0) break;

		// Make a backup copy of the line
		strcpy(raw_line, buf);
		raw_line[strlen(raw_line) - 1] = '\0';

		// Skip comments
		if (buf[0] == '#') continue;

		if (strncmp(buf, "$ENDELM", strlen("$ENDELM")) == 0) break;

		// Read and check tet #
		char* token = strtok(buf, "\t ");
		if (!token) {
			_cprintf("Tet number read error at line %d (%s)\n", line_number, raw_line);
			error = true; break;
		}

		unsigned int tet_index;

		int result = sscanf(token, "%u", &tet_index);

		if (element_format == ELEMENT_FORMAT_GMSH_FILE) {

			// This is silly; there's no reason to keep track of tet numbering
			/*
			if (ntets_read == 0) {
			  one_indexed_tets = 0;
			  tet_offset = tet_index;
			}
			tet_index-=tet_offset;
			*/
			one_indexed_tets = 0;
			tet_index = ntets_read;
		}

		if (one_indexed_tets) tet_index--;

		if (result == 0 || tet_index < 0 || tet_index >= ntets) {
			_cprintf("Tet index error at line %d (%s)\n", line_number, raw_line);
			error = true;
			break;
		}

		if (ntets_read == 0) {
			if (tet_index == 1) {
				_cprintf("Assuming one-indexed tets...\n");
				one_indexed_tets = 1;
				tet_index--;
			}
		}

		// This is all silly; there's no reason to keep track of tet numbering

		// over-ride the tet index...
		tet_index = ntets_read;

		// Read the leading tokens
		if (element_format == ELEMENT_FORMAT_GMSH_FILE) {

			int result, element_type, reg_phys, reg_elem, number_of_nodes;

			// Read element type
			token = strtok(0, "\t ");
			result = sscanf(token, "%u", &element_type);
			if (result == 0 || element_type != GMSH_ELEMENT_TYPE_TET) {
				_cprintf("Illegal gmsh tet at line %s\n", raw_line);
				error = true; break;
			}

			// Read physical properties
			token = strtok(0, "\t ");
			if (!token) { error = true; break; }
			result = sscanf(token, "%u", &reg_phys);
			if (result == 0) {
				_cprintf("Illegal element spec at line %s\n", raw_line);
				error = true; break;
			}

			token = strtok(0, "\t ");
			if (!token) { error = true; break; }
			result = sscanf(token, "%u", &reg_elem);
			if (result == 0) {
				_cprintf("Illegal element spec at line %s\n", raw_line);
				error = true; break;
			}

			// Read number of nodes
			token = strtok(0, "\t ");
			if (!token) { error = true; break; }
			result = sscanf(token, "%u", &number_of_nodes);
			if (result == 0 || number_of_nodes != 4) {
				_cprintf("Illegal node count at line %s\n", raw_line);
				error = true; break;
			}
		}

		// Read values
		for (unsigned int k = 0; k < nodes_per_tet; k++) {

			token = strtok(0, "\t ");

			if (!token) {
				_cprintf("Value read error 1 at line %d (%s)\n", line_number, raw_line);
				error = true; break;
			}

			unsigned int value;
			int result = sscanf(token, "%u", &value);
			if (result == 0) {
				_cprintf("Value read error 2 at line %d (%s)\n", line_number, raw_line);
				error = true;
				break;
			}

			tet_indices[k] = value;

			// It _seems_ that one-indexed tet files use one-indexed vertex indices
			if (element_format == ELEMENT_FORMAT_ELE_FILE) {
				if (one_indexed_tets) {
					if (tet_indices[k] == 0) {
						_cprintf("Strange... a one-indexed tet file has a vertex index of zero.\n");
					}
					else tet_indices[k]--;
				}
			}

			else if (element_format == ELEMENT_FORMAT_GMSH_FILE) {

				// Load point index from point map
				std::map<unsigned int, unsigned int>::iterator iter =
					gmsh_point_map.find(tet_indices[k]);
				if (iter == gmsh_point_map.end()) {
					_cprintf("Warning: couldn't find point %d for tet %d\n", tet_indices[k], tet_index);
				}
				tet_indices[k] = (*iter).second;
			}
		}

		// Read marker(s) if present 
		for (unsigned int k = 0; k < nmarkers; k++) {

			token = strtok(0, "\t ");

			if (!token) {
				_cprintf("Error reading marker %d at line %d (%s)\n", k, line_number, raw_line);
				error = true; break;
			}

			int value;
			int result = sscanf(token, "%d", &value);
			if (result == 0) {
				_cprintf("Marker read error at line %d (%s)\n", line_number, raw_line);
				error = true;
				break;
			}

			markers[k] = value;

		}

		// We need to compute a normal for each of these vertices    
		cVector3d normals[4];
		cVector3d positions[4];
		cVector3d center_of_mass(0, 0, 0);
		std::vector<cVertex>* vertex_vector = top_level_mesh->pVertices();
		cVertex* vertex_array = (cVertex*) &((*vertex_vector)[0]);

		// Find the position of each vertex and compute the c.o.m for
		// this tet
		for (int k = 0; k < 4; k++) {
			int vertex_index = tet_indices[k];
			positions[k] = vertex_array[vertex_index].getPos();
			center_of_mass += positions[k];
		}

		center_of_mass /= 4.0;

		// Compute _signed_ tet volume
		double v = (1.0 / 6.0) *
			(positions[1] - positions[0]).dot(
			((positions[2] - positions[0]).crossAndReturn(positions[3] - positions[0]))
			);

		if (m_remove_small_tets_at_import && (fabs(v) < m_small_import_tet_volume)) {
			_cprintf("Ignoring tet %d due to volume error (%lf)...\n", tet_index, v);
			continue;
		}

		// Add each local normal to the total normal for each vertex;
		// we'll normalize everyone later
		for (int k = 0; k < 4; k++) {
			normals[k] = cSub(positions[k], center_of_mass);

			// Ignore weird degenerate tets
			if (normals[k].length() < 0.00001) {
				_cprintf("Ignoring tet %d due to normal error (%lf)...\n", tet_index, (double)(normals[k].length()));
				continue;
			}

			normals[k].normalize();
			int vertex_index = tet_indices[k];
			cVertex* vp = vertex_array + vertex_index;
			vp->setNormal(vp->getNormal() + normals[k]);
		}

		// We'll only bother to load tets into different meshes for 
		// different attributes if we haven't already done that for
		// faces
		int already_loaded_submeshes = 0;
		if (face_attributes.size() != 0) already_loaded_submeshes = 1;

		if (nmarkers == 0 || already_loaded_submeshes == 1) {

			// Put this in our tet array
			memcpy(top_level_mesh->m_tets + (nodes_per_tet*tet_index), tet_indices, nodes_per_tet * sizeof(unsigned int));

			// Create triangles in our face array if necessary
			if (loaded_face_file == false) {

				// TODO: what is the actual location of the four "important"
				// tets in the 10-node format?

				unsigned int face_index = tet_index * 4;
				std::vector<cTriangle>* tri_vector = top_level_mesh->pTriangles();
				cTriangle* tri_array = (cTriangle*) &((*tri_vector)[0]);

				cTriangle triangles[4];
				for (int t = 0; t < 4; t++) {
					triangles[t] = cTriangle(top_level_mesh,
						tet_indices[(tet_triangle_faces[t][0])],
						tet_indices[(tet_triangle_faces[t][1])],
						tet_indices[(tet_triangle_faces[t][2])]);
					triangles[t].m_allocated = 1;
					triangles[t].m_index = face_index;
					tri_array[face_index++] = triangles[t];
				}
			}
		}

		// We have some attributes and we haven't already colored
		// our meshes based on the face file
		else {

			// _cprintf("Loading boundary markers (%d)\n",nmarkers);

			int boundary = markers[0];

			// Find the mesh for this marker
			cTetMesh* mesh;

			std::map<int, int>::iterator iter = boundary_to_mesh_map.find(boundary);

			// If this is a new mesh
			if (iter == boundary_to_mesh_map.end()) {

				// Create a child mesh
				cTetMesh* child = (cTetMesh*)(top_level_mesh->createMesh());
				child->setParentWorld(top_level_mesh->getParentWorld());
				child_meshes.push_back(child);

				// Add a mapping from this marker to this mesh
				int mesh_index = child_meshes.size() - 1;
				boundary_to_mesh_map[boundary] = mesh_index;

				_cprintf("Creating mesh %d for tet attribute %d\n", mesh_index, boundary);

				tet_attributes.push_back(boundary);

				// Assign a material property to this mesh
				int property = mesh_index % NUM_DEFAULT_MATERIALS;
				child->setMaterial(default_materials[property], 0);

				// Tell this mesh where his vertex buffer lives
				child->vertex_array_holder = top_level_mesh;
				top_level_mesh->addChild(child);

				mesh = child;
			}
			else {
				int mesh_index = (*iter).second;
				mesh = child_meshes[mesh_index];
			}

			// Put triangles for this tet in the mesh
			cTriangle triangles[4];
			for (int t = 0; t < 4; t++) {
				mesh->newTriangle(
					tet_indices[(tet_triangle_faces[t][0])],
					tet_indices[(tet_triangle_faces[t][1])],
					tet_indices[(tet_triangle_faces[t][2])]
				);
			}
		}

		// _cprintf("Loaded a tet (%d) %d %d %d\n",face_index,face[0],face[1],face[2]);
		ntets_read++;

	}

	delete[] markers;
	delete[] tet_indices;

	if (ntets_read != ntets) {
		_cprintf("Expected %d tets, found %d\n", ntets, ntets_read);
		if (ntets_read > ntets) {
			_cprintf("...quitting...\n");
			error = true;
		}
		else {
			top_level_mesh->m_nTets = ntets_read;
		}
	}

	if (error) {
		_cprintf("Error at line %d in tet file %s\n", line_number, filename);
	}
	else {
		loaded_element_file = true;

		// Normalize all the vertices
		std::vector<cVertex> * vertex_vector = top_level_mesh->pVertices();
		cVertex* vertex_array = (cVertex*) &((*vertex_vector)[0]);
		unsigned int nvertices = vertex_vector->size();
		for (unsigned int k = 0; k < nvertices; k++) {
			cVector3d* n = &(vertex_array[k].m_normal);
			n->normalize();
		}
		_cprintf("Successfully loaded element file %s (%u tets)\n", filename, ntets_read);
	}

	return (!error);

}


bool cTetGenLoader::read_node_file(FILE* f, const char* filename, int node_format) {

	// Node format:
	/*
	First line:
	   <# of points> <dimension (must be 3)> <# of attributes> <# of boundary markers (0 or 1)>
	Remaining lines list # of points:
	  <point #> <x> <y> <z> [attributes] [boundary marker]
	*/

	// Gmsh format:
	/*
	First two lines:
	  $NOD
	  <# of points>
	Remaining lines list # of points:
	  <point #> <x> <y> <z>
	*/


	char buf[1000];
	char* cresult;
	int nresult[4];

	// Get the header line
	while (1) {

		cresult = fgets(buf, 1000, f);

		if (cresult == 0) {
			_cprintf("Error reading header line from file %s\n", filename);
			return false;
		}

		// Skip leading comments, unless they give us scale/offset information
		if (buf[0] == '#') {
			char* token = buf + 2;
			cTetMesh* top_level_mesh = this->current_mesh;
			if (strncmp(token, "SCALE", strlen("SCALE")) == 0) {
				token += (strlen("SCALE") + 1);
				float scale;
				int result = sscanf(token, "%f", &scale);
				if (result != 1) {
					_cprintf("Could not read scale factor from %s\n", buf);
					continue;
				}
				top_level_mesh->mesh_prescale = scale;
				_cprintf("Prescale: %f\n", scale);
			}
			else if (strncmp(token, "OFFSET", strlen("OFFSET")) == 0) {
				token += (strlen("OFFSET") + 1);
				float ox, oy, oz;
				int result = sscanf(token, "%f %f %f", &ox, &oy, &oz);
				if (result != 3) {
					_cprintf("Could not read offset from %s\n", buf);
					continue;
				}
				top_level_mesh->mesh_preoffset.set(ox, oy, oz);
				_cprintf("Pre-offset: %s\n", top_level_mesh->mesh_preoffset.str(3).c_str());
			}
			else if (strncmp(token, "ZERO", strlen("ZERO")) == 0) {
				token += (strlen("ZERO") + 1);
				float ox, oy, oz;
				int result = sscanf(token, "%f %f %f", &ox, &oy, &oz);
				if (result != 3) {
					_cprintf("Could not read offset from %s\n", buf);
					continue;
				}
				top_level_mesh->mesh_prezero.set(ox, oy, oz);
				_cprintf("Pre-zero: %s\n", top_level_mesh->mesh_prezero.str(3).c_str());
			}
			else continue;
		}
		if (buf[0] != '#' && buf[0] != '\0' && buf[0] != '\n') break;
	}

	unsigned int npoints, dimension, nattributes, nmarkers;

	if (node_format == NODE_FORMAT_NODE_FILE) {

		cresult = strtok(buf, "\t ");
		nresult[0] = (cresult == 0) ? 0 : sscanf(cresult, "%u", &npoints);
		cresult = strtok(0, "\t ");
		nresult[1] = (cresult == 0) ? 0 : sscanf(cresult, "%u", &dimension);
		cresult = strtok(0, "\t ");
		nresult[2] = (cresult == 0) ? 0 : sscanf(cresult, "%u", &nattributes);
		cresult = strtok(0, "\t ");
		nresult[3] = (cresult == 0) ? 0 : sscanf(cresult, "%u", &nmarkers);

		if (nresult[0] == 0 || nresult[1] == 0 || nresult[2] == 0 || nresult[3] == 0) {
			_cprintf("Error reading header from node file %s\n", filename);
			return false;
		}

	}

	else if (node_format == NODE_FORMAT_GMSH_FILE) {
		dimension = 3;
		nattributes = 0;
		nmarkers = 0;

		// Read the $NOD line
		if (strncmp(buf, "$NOD", strlen("$NOD"))) {
			_cprintf("Error: illegal gmsh node header %s\n", buf);
			return false;
		}
		cresult = fgets(buf, 1000, f);
		if (cresult == 0) {
			_cprintf("Error reading gmsh node spec from file %s\n", filename);
			return false;
		}

		// Read the number of nodes
		nresult[0] = sscanf(buf, "%d", &npoints);

		if (nresult[0] == 0) {
			_cprintf("Error reading gmsh node spec from file %s\n", filename);
			return false;
		}
	}

	else {
		_cprintf("Unrecognized node file format...\n");
	}

	if (dimension != 3) {
		_cprintf("Only 3d node files are supported...\n");
		return false;
	}

	_cprintf("Loading %d points with %d attributes and %d boundary markers\n",
		npoints, nattributes, nmarkers);

	// TODO: create submesh map if attributes are present

	// Allocate vertices
	cMesh* mesh = this->current_mesh;

	cTetMesh* tmesh = dynamic_cast<cTetMesh*>(mesh);
	if (tmesh && (nmarkers > 0)) {
		tmesh->m_nVertexBoundaryMarkers = nmarkers;
		tmesh->m_vertexBoundaryMarkers = new int[nmarkers*npoints];
	}

	cVertex v(0, 0, 0);

	// Clear normals so we can accumulate them later
	v.setNormal(0, 0, 0);

	mesh->pVertices()->reserve(npoints);
	mesh->pVertices()->resize(npoints, v);

	bool error = false;

	unsigned int npoints_read = 0;
	unsigned int line_number = 0;

	// Read attributes if present
	double* attributes = (double*)malloc(sizeof(double)*nattributes);
	int* markers = (int*)malloc(sizeof(int)*nmarkers);

	one_indexed_points = 0;

	while (npoints_read < npoints) {
		line_number++;
		char raw_line[1000];
		cresult = fgets(buf, 1000, f);
		if (cresult == 0) break;

		// Make a backup copy of the line
		strcpy(raw_line, buf);
		raw_line[strlen(raw_line) - 1] = '\0';

		// Skip comments
		if (buf[0] == '#') continue;

		// Read and check point #
		char* token = strtok(buf, "\t ");
		if (!token) { error = true; break; }

		unsigned int point_index;

		int result = sscanf(token, "%u", &point_index);

		// gmsh files don't necessarily start numbering nodes at 0 or 1, so we offset
		// by the first index we find
		if (node_format == NODE_FORMAT_GMSH_FILE) {
			// Map this point to the right place in our vertex array
			gmsh_point_map[point_index] = npoints_read;
			point_index = npoints_read;
		}

		if (one_indexed_points) point_index--;

		if (result == 0 || point_index < 0 || point_index >= npoints) {
			_cprintf("Point index error at line %d (%s)", line_number, raw_line);
			error = true;
			break;
		}

		if (npoints_read == 0) {
			if (point_index == 1) {
				_cprintf("Assuming one-indexed points...\n");
				one_indexed_points = 1;
				point_index--;
			}
		}

		// Read values
		cVector3d pos;
		for (unsigned int k = 0; k < 3; k++) {

			token = strtok(0, "\t ");

			if (!token) { error = true; break; }

			double value;
			int result = sscanf(token, "%lf", &value);
			if (result == 0) {
				_cprintf("Value read error 3 at line %d (%s)\n", line_number, raw_line);
				error = true;
				break;
			}

			pos[k] = value;

		}

		// Read attributes
		for (unsigned int k = 0; k < nattributes; k++) {

			token = strtok(0, "\t ");

			if (!token) { error = true; break; }

			double value;
			int result = sscanf(token, "%lf", &value);
			if (result == 0) {
				_cprintf("Attribute read error at line %d (%s)", line_number, raw_line);
				error = true;
				break;
			}

			attributes[k] = value;

		}

		// TODO: create a new submesh if this is a new attribute

		// TODO: do something with the attributes

		// Read marker(s) if present 
		for (unsigned int k = 0; k < nmarkers; k++) {

			token = strtok(0, "\t ");

			if (!token) { error = true; break; }

			int value;
			int result = sscanf(token, "%d", &value);
			if (result == 0) {
				_cprintf("Marker read error at line %d (%s)", line_number, raw_line);
				error = true;
				break;
			}

			markers[k] = value;

		}

		// Store markers if necessary
		if (tmesh && (nmarkers > 0)) {
			int* markerpos = tmesh->m_vertexBoundaryMarkers + (nmarkers*point_index);
			for (unsigned int k = 0; k < nmarkers; k++) {
				markerpos[k] = markers[k];
			}
		}

		// Put this in our vertex array    
		cVertex v(pos.x, pos.y, pos.z);

		// We clear all the normals so we can add them up later
		v.setNormal(0, 0, 0);
		(*(mesh->pVertices()))[point_index] = v;

		npoints_read++;

		// _cprintf("Loaded a point (%d) %lf %lf %lf\n",point_index,pos[0],pos[1],pos[2]);

		// TODO: put this in the appropriate submesh's array
	}

	delete[] attributes;
	delete[] markers;

	if (npoints_read != npoints) {
		_cprintf("Expected %d vertices, found %d\n", npoints, npoints_read);
		error = true;
	}

	if (error) {
		_cprintf("Error at line %d in node file %s\n", line_number, filename);
	}
	else {
		_cprintf("Successfully loaded node file %s (%u nodes)\n", filename, npoints_read);
	}

	return (!error);

}

bool cTetGenLoader::load_gmsh_file(const char* filename) {

	_cprintf("Loading gmsh file %s\n", filename);

	loaded_face_file = false;

	FILE* f = fopen(filename, "rb");

	if (f == 0) {
		_cprintf("Could not open gmsh file %s\n", filename);
		return false;
	}

	bool nresult = read_node_file(f, filename, NODE_FORMAT_GMSH_FILE);

	char buf[1000];

	// Read out the $ENDNOD line
	char* cresult;
	int result;

	cresult = fgets(buf, 1000, f);
	if (cresult == 0) {
		_cprintf("Warning: error reading $ENDNOD from file %s\n", filename);
	}
	else {
		if (strncmp(buf, "$ENDNOD", strlen("$ENDNOD"))) {
			_cprintf("Warning: illegal $ENDNOD in file %s\n", filename);
		}
	}

	// Read the $ELM line and ignore it
	cresult = fgets(buf, 1000, f);
	if (cresult == 0) {
		_cprintf("Warning: error reading $ENDNOD from file %s\n", filename);
	}

	if (strncmp(buf, "$ELM", strlen("$ELM"))) {
		_cprintf("Error: illegal gmsh element header %s\n", buf);
		fclose(f);
		return false;
	}
	cresult = fgets(buf, 1000, f);
	if (cresult == 0) {
		_cprintf("Error reading gmsh element spec from file %s\n", filename);
		fclose(f);
		return false;
	}

	// Should be ntets + nfaces
	unsigned int nelements;

	// Read the number of elements
	result = sscanf(buf, "%d", &nelements);

	if (result == 0) {
		_cprintf("Error getting gmsh element spec from file %s\n", filename);
		fclose(f);
		return false;
	}

	// Now we want to keep track of how many faces and tets there are, and
	// where they each begin in the file
	unsigned int ntets = 0, nfaces = 0;

	// Where is the first face line in the file?
	unsigned int face_fp = 0;

	// Where is the first tet line in the file?
	unsigned int tet_fp = 0;

	// Store the pointer to the beginning of the faces
	face_fp = ftell(f);

	// Count faces
	while (1) {

		// Assume for now that this will be the first tet line
		tet_fp = ftell(f);

		cresult = fgets(buf, 1000, f);
		if (cresult == 0) break;
		if (buf[0] == '#' || buf[0] == '\0' || buf[0] == '\n') continue;

		int element_number, element_type;
		result = sscanf(buf, "%d %d", &element_number, &element_type);
		if (result != 2) {
			_cprintf("Error processing face line %s\n", buf);
			fclose(f);
			return false;
		}
		if (element_type == GMSH_ELEMENT_TYPE_TRIANGLE) nfaces++;
		else break;
	}

	_cprintf("Found %d faces\n", nfaces);

	// Go back to the beginning of that line
	fseek(f, tet_fp, SEEK_SET);

	// Count tets
	while (1) {

		cresult = fgets(buf, 1000, f);
		if (cresult == 0) break;
		if (buf[0] == '#' || buf[0] == '\0' || buf[0] == '\n') continue;

		if (strncmp(buf, "$ENDELM", strlen("$ENDELM")) == 0) break;

		int element_number, element_type;
		result = sscanf(buf, "%d %d", &element_number, &element_type);
		if (result != 2) {
			_cprintf("Error processing tet line %s\n", buf);
			fclose(f);
			return false;
		}
		if (element_type == GMSH_ELEMENT_TYPE_TET) ntets++;
		else break;
	}

	_cprintf("Found %d tets\n", ntets);

	if (ntets + nfaces != nelements) {
		_cprintf("Warning: I found %d tets and %d faces but I should have found %d elements\n",
			ntets, nfaces, nelements);
	}

	m_precounted_faces = nfaces;
	m_precounted_tets = ntets;

	// Seek to the first face line
	fseek(f, face_fp, SEEK_SET);

	bool fresult = true;
	read_face_file(f, filename, FACE_FORMAT_GMSH_FILE);

	if (fresult == false) {
		_cprintf("Error reading faces from gmsh file\n");
		fclose(f);
		return false;
	}

	// Seek to the first tet line (technically unnecessary)
	fseek(f, tet_fp, SEEK_SET);

	bool eresult = read_element_file(f, filename, ELEMENT_FORMAT_GMSH_FILE);

	if (eresult == false) {
		_cprintf("Error reading tets from gmsh file\n");
		fclose(f);
		return false;
	}

	fclose(f);
	return true;
}



bool cTetGenLoader::load_smesh_file(const char* filename) {

	_cprintf("Loading smesh file %s\n", filename);

	loaded_face_file = false;

	FILE* f = fopen(filename, "rb");

	if (f == 0) {
		_cprintf("Could not open smesh file %s\n", filename);
		return false;
	}

	bool nresult = read_node_file(f, filename);

	if (current_mesh->getNumVertices() == 0) {
		_cprintf("Didn't find any vertices in the smesh file, trying the corresponding node file...\n");
		nresult = load_node_file(node_filename);
		if (nresult == false) {
			_cprintf("Couldn't find nodes anywhere for smesh file %s\n", filename);
			fclose(f);
			return false;
		}
	}

	bool fresult = read_face_file(f, filename, FACE_FORMAT_SMESH_FILE);

	fclose(f);

	return (nresult && fresult);
}


bool cTetGenLoader::load_face_file(const char* filename) {

	_cprintf("Loading face file %s\n", filename);

	loaded_face_file = false;

	FILE* f = fopen(filename, "rb");

	if (f == 0) {
		_cprintf("Could not open face file %s\n", filename);
		return false;
	}

	bool result = read_face_file(f, filename);

	fclose(f);

	return result;
}


bool cTetGenLoader::load_element_file(const char* filename) {

	_cprintf("Loading element file %s\n", filename);

	loaded_element_file = false;

	FILE* f = fopen(filename, "rb");

	if (f == 0) {
		_cprintf("Could not open element file %s\n", filename);
		return false;
	}

	bool result = read_element_file(f, filename);

	fclose(f);

	return result;
}


bool cTetGenLoader::load_node_file(const char* filename) {

	_cprintf("Loading node file %s\n", filename);

	FILE* f = fopen(filename, "rb");

	if (f == 0) {
		_cprintf("Could not open node file %s\n", filename);
		return false;
	}

	bool result = read_node_file(f, filename);

	fclose(f);

	return result;
}


bool cTetGenLoader::read_face_file(FILE* f, const char* filename, int face_format) {

	// Tetgen format:
	/*
	First line:
	  <# of faces> <boundary marker (0 or 1)>
	Remaining lines list of # of faces:
	  <face #> <node> <node> <node> [boundary marker]
	*/

	/*
	One line:
	  <# of facets> <boundary markers (0 or 1)>
	Following lines list # of facets:
	  <# of corners> <corner 1> <corner 2> ... <corner #> [boundary marker]
	*/

	// gmsh format:
	// No header left

	// Each line:
	// elm-number elm-type reg-phys reg-elem number-of-nodes node-number-list

	char buf[1000];
	char* cresult;
	int nresult[2];

	// Get the header line 
	if (face_format != FACE_FORMAT_GMSH_FILE) {
		while (1) {

			cresult = fgets(buf, 1000, f);

			if (cresult == 0) {
				_cprintf("Error reading header line from file %s\n", filename);
				return false;
			}

			// Skip leading comments
			if (buf[0] != '#' && buf[0] != '\0' && buf[0] != '\n') break;

		}
	}

	unsigned int nfaces, nmarkers;

	if (face_format == FACE_FORMAT_SMESH_FILE || face_format == FACE_FORMAT_FACE_FILE) {

		cresult = strtok(buf, "\t ");
		nresult[0] = sscanf(cresult, "%u", &nfaces);
		cresult = strtok(0, "\t ");
		nresult[1] = sscanf(cresult, "%u", &nmarkers);

		if (nresult[0] == 0 || nresult[1] == 0) {
			_cprintf("Error reading header from face file %s\n", filename);
			return false;
		}

	}
	else if (face_format == FACE_FORMAT_GMSH_FILE) {
		nfaces = m_precounted_faces;
		nmarkers = 1;
	}
	else {
		_cprintf("Unrecognized face format...\n");
		return false;
	}

	_cprintf("Loading %d faces with %d boundary markers\n",
		nfaces, nmarkers);

	// TODO: create submesh map if attributes are present in the _node_ file
	std::map<int, int> boundary_to_mesh_map;
	std::vector<cTetMesh*> child_meshes;
	cMesh* top_level_mesh = this->current_mesh;

	// Allocate faces
	if (nmarkers == 0) {
		top_level_mesh->pTriangles()->reserve(nfaces);
		cTriangle t(0, 0, 0, 0);
		t.m_parent = top_level_mesh;
		top_level_mesh->pTriangles()->resize(nfaces, t);
		top_level_mesh->setMaterial(default_materials[0]);
	}

	bool error = false;

	unsigned int nfaces_read = 0;
	unsigned int line_number = 0;

	int* markers = (nmarkers > 0) ? (int*)malloc(sizeof(int)*nmarkers) : 0;

	int one_indexed_faces = 0;

	unsigned int face_index = -1;

	while (nfaces_read < nfaces) {

		char raw_line[1000];

		line_number++;
		cresult = fgets(buf, 1000, f);
		if (cresult == 0) break;

		// Make a backup copy of the line
		strcpy(raw_line, buf);

		// Skip comments
		if (buf[0] == '#' || buf[0] == '\n') continue;

		// Read and check face # or corner count
		char* token = strtok(buf, "\t ");
		if (!token) { error = true; break; }

		unsigned int corner_count = 3;
		unsigned int material_tag = 0;

		if (face_format == FACE_FORMAT_FACE_FILE) {

			int result = sscanf(token, "%u", &face_index);
			if (one_indexed_faces) face_index--;

			if (result == 0 || face_index < 0 || face_index >= nfaces) {
				_cprintf("Face index error at line %d (%s)", line_number, raw_line);
				error = true;
				break;
			}
		}

		// SMESH format
		else if (face_format == FACE_FORMAT_SMESH_FILE) {

			one_indexed_faces = 0;
			int result = sscanf(token, "%u", &corner_count);

			if (corner_count > 3) {
				_cprintf("Warning: this reader doesn't support non-triangular facets\n");
			}

			face_index++;
		} // if this is in smesh format

		else if (face_format == FACE_FORMAT_GMSH_FILE) {

			// Throw out the face # we read from file
			face_index = nfaces_read;

			int result, element_type, reg_phys, reg_elem, number_of_nodes;

			// Read element type
			token = strtok(0, "\t ");
			result = sscanf(token, "%u", &element_type);
			if (result == 0 || element_type != GMSH_ELEMENT_TYPE_TRIANGLE) {
				_cprintf("Illegal gmsh face at line %s\n", raw_line);
				error = true; break;
			}

			// Read physical properties
			token = strtok(0, "\t ");
			if (!token) { error = true; break; }
			result = sscanf(token, "%u", &reg_phys);
			if (result == 0) {
				_cprintf("Illegal element spec at line %s\n", raw_line);
				error = true; break;
			}

			material_tag = reg_phys;

			token = strtok(0, "\t ");
			if (!token) { error = true; break; }
			result = sscanf(token, "%u", &reg_elem);
			if (result == 0) {
				_cprintf("Illegal element spec at line %s\n", raw_line);
				error = true; break;
			}

			// Read number of nodes
			token = strtok(0, "\t ");
			if (!token) { error = true; break; }
			result = sscanf(token, "%u", &number_of_nodes);
			if (result == 0 || number_of_nodes != 3) {
				_cprintf("Illegal node count at line %s\n", raw_line);
				error = true; break;
			}

			one_indexed_faces = 0;
		}

		if (nfaces_read == 0) {
			if (face_index == 1) {
				_cprintf("Assuming one-indexed faces...\n");
				one_indexed_faces = 1;
				face_index--;
			}
		}

		// Read values
		unsigned int face[3];
		for (unsigned int k = 0; k < 3; k++) {

			token = strtok(0, "\t ");
			if (!token) { error = true; break; }

			unsigned int value;
			int result = sscanf(token, "%u", &value);
			if (result == 0) {
				_cprintf("Value read error 4 at line %d (%s)\n", line_number, raw_line);
				error = true;
				break;
			}

			face[k] = value;

			if (face_format == FACE_FORMAT_FACE_FILE) {
				// It _seems_ that one-indexed face files use one-indexed vertex indices
				if (one_indexed_faces) {
					if (face[k] == 0) {
						_cprintf("Strange... a one-indexed face file has a vertex index of zero.\n");
					}
					else face[k]--;
				}
			}

			else if (face_format == FACE_FORMAT_SMESH_FILE) {
				if (one_indexed_points) {
					if (face[k] == 0) {
						_cprintf("Strange... a one-indexed face file has a vertex index of zero.\n");
					}
					else face[k]--;
				}
			}

			else if (face_format == FACE_FORMAT_GMSH_FILE) {

				// Load point index from point map
				std::map<unsigned int, unsigned int>::iterator iter =
					gmsh_point_map.find(face[k]);
				if (iter == gmsh_point_map.end()) {
					_cprintf("Warning: couldn't find point %d for face %d\n", face[k], face_index);
				}
				face[k] = (*iter).second;
			}

		}

		// Discard extra corners if necessary
		corner_count -= 3;

		while (corner_count > 0) {
			token = strtok(0, "\t ");

			// Did this break onto a new line?
			if (token == 0) {
				line_number++;
				cresult = fgets(buf, 1000, f);

				if (cresult == 0) {
					_cprintf("Warning: error reading breaking smesh line...\n");
					error = true;
					break;
				}

				// Make a backup copy of the line
				strcpy(raw_line, buf);
				token = strtok(buf, "\t ");
				if (token == 0) {
					_cprintf("Warning: error reading breaking smesh line...\n");
					error = true;
					break;
				}
			}
			corner_count--;
		} // reading all corners

		if (error) break;

		// Read marker(s) if present 

		// We've already read the marker for face files
		if (face_format == FACE_FORMAT_GMSH_FILE) {
			if (nmarkers > 0) markers[0] = material_tag;
		}
		else {
			for (unsigned int k = 0; k < nmarkers; k++) {

				token = strtok(0, "\t ");

				if (!token) { error = true; break; }

				int value;
				int result = sscanf(token, "%d", &value);
				if (result == 0) {
					_cprintf("Marker read error at line %d (%s)", line_number, raw_line);
					error = true;
					break;
				}

				markers[k] = value;

			}
		}

		if (nmarkers == 0) {
			// Put this in our face array    
			cTriangle t = cTriangle(top_level_mesh, face[0], face[1], face[2]);
			t.m_allocated = true;
			t.m_index = face_index;
			t.m_parent = top_level_mesh;
			(*(top_level_mesh->pTriangles()))[face_index] = t;
		}
		else {

			int boundary = markers[0];

			// Find the mesh for this marker
			cMesh* mesh;

			std::map<int, int>::iterator iter = boundary_to_mesh_map.find(boundary);

			// If this is a new mesh
			if (iter == boundary_to_mesh_map.end()) {

				// Create a child mesh
				cTetMesh* child = (cTetMesh*)(top_level_mesh->createMesh());
				child->setParentWorld(top_level_mesh->getParentWorld());
				child_meshes.push_back(child);

				// Add a mapping from this marker to this mesh
				int mesh_index = child_meshes.size() - 1;
				boundary_to_mesh_map[boundary] = mesh_index;

				_cprintf("Creating mesh %d for boundary %d\n", mesh_index, boundary);

				face_attributes.push_back(boundary);

				// Assign a material property to this mesh
				int property = mesh_index % NUM_DEFAULT_MATERIALS;
				child->setMaterial(default_materials[property], 0);

				// Tell this mesh where his vertex buffer lives
				child->vertex_array_holder = top_level_mesh;
				top_level_mesh->addChild(child);

				mesh = child;
			}
			else {
				int mesh_index = (*iter).second;
				mesh = child_meshes[mesh_index];
			}

			// Put this triangle in the mesh
			mesh->newTriangle(face[0], face[1], face[2]);

		}

		//_cprintf("Loaded a face (%d) %d %d %d\n",face_index,face[0],face[1],face[2]);
		nfaces_read++;
	}

	if (markers) delete[] markers;

	if (nfaces_read != nfaces) {
		_cprintf("Expected %d faces, found %d\n", nfaces, nfaces_read);
		error = true;
	}

	if (error) {
		_cprintf("Error at line %d in face file %s\n", line_number, filename);
	}
	else {
		loaded_face_file = true;
		_cprintf("Successfully loaded face file %s (%u faces)\n", filename, nfaces_read);
	}

	return (!error);

}


cPanel* cTetGenLoader::build_attribute_key(cWorld* world, int& num_attributes) {

	int use_face_attributes = 1;

	num_attributes = face_attributes.size();

	// If we didn't have any face attributes, see if we have
	// any tet attributes
	if (num_attributes == 0) {
		use_face_attributes = 0;
		num_attributes = tet_attributes.size();
		if (num_attributes == 0) return 0;
		use_face_attributes = 0;
	}

	cLabelPanel* panel = new cLabelPanel(world);

	std::list<int>* attribute_list =
		(use_face_attributes) ? (&face_attributes) : (&tet_attributes);

	std::list<int>::iterator iter;
	int index = 0;
	for (iter = attribute_list->begin(); iter != attribute_list->end(); iter++, index++) {
		char buf[1000];
		sprintf(buf, "Marker %d", *iter);
		panel->addLabel(buf, &(default_materials[index % NUM_DEFAULT_MATERIALS]));
	}

	/*
	panel->setTitle(use_face_attributes?
	  "Face attributes":
	  "Tet attributes");
	*/

	return panel;
}
