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

#include "stdafx.h"
#include "meshImporter.h"

#include "CFileLoaderOBJ.h"
#include "CFileLoader3DS.h"
#include "ply_loader.h"
#include "meshExporter.h"

bool importModel(const char* in_filename, cMesh*& new_object, cWorld* world,

	bool build_collision_detector, bool finalize,
	cMesh* factory,

	cLabelPanel** new_label_panel,

	transform_operation transform_to_apply,
	mesh_xform_information* xform_data,

	char* selected_filename
) {

	char filename[_MAX_PATH];

	if (in_filename) {
		strcpy(filename, in_filename);
	}

	else {
		int result = FileBrowse(filename, _MAX_PATH, 0, 0,
			"model files (*.obj, *.3ds, *.ply, *.anode, *.node, *.face, *.ele, *.smesh, *.msh)|*.obj;*.3ds;*.ply;*.anode;*.node;*.face;*.ele;*.smesh;*.msh|All Files (*.*)|*.*||",
			"Choose a model...");

		if (result < 0) {
			_cprintf("File browse canceled...\n");
			return false;
		}
	}

	int alt_pressed = GetAsyncKeyState(VK_MENU) & (1 << 15);

	if (alt_pressed == false) {
		g_objLoaderShouldGenerateExtraVertices = false;
		g_3dsLoaderShouldGenerateExtraVertices = false;
	}
	else {
		_cprintf("WARNING: alt pressed, not merging vertices...\n");
		g_objLoaderShouldGenerateExtraVertices = true;
		g_3dsLoaderShouldGenerateExtraVertices = true;
	}

	// Is this a ply file?
	int ply_file = 0;
	int tet_file = 0;

	// Get the extension
	char* extension = find_extension(filename);

	if (extension && (strcmp(extension, "ply") == 0)) ply_file = 1;

	else if (extension &&
		(
		(strcmp(extension, "anode") == 0) ||
			(strcmp(extension, "node") == 0) ||
			(strcmp(extension, "face") == 0) ||
			(strcmp(extension, "ele") == 0) ||
			(strcmp(extension, "smesh") == 0) ||
			(strcmp(extension, "msh") == 0)
			)
		) tet_file = 1;

	// If it's a ply file...
	if (ply_file) {

		cPlyLoader plyloader;

		// create a new mesh
		if (factory) new_object = factory->createMesh();
		else new_object = new default_mesh_type(world);

		bool result = plyloader.load_ply_file(new_object, world, filename);
		if (result == false) {
			delete new_object;
			return false;
		}

	}

	else if (tet_file) {

		cTetGenLoader tetLoader;
		new_object = tetLoader.load_tetgen_file(world, filename, factory);

		if (new_object == 0) return false;

		int num_attributes;
		if (new_label_panel)
			*new_label_panel = (cLabelPanel*)tetLoader.build_attribute_key(world, num_attributes);

	}

	else {

		// create a new mesh
		if (factory) new_object = factory->createMesh();
		else new_object = new default_mesh_type(world);

		_cprintf("Loading mesh file %s\n", filename);

		// load 3d object file
		int result = new_object->loadFromFile(filename);

		if (result == 0) {

			_cprintf("Could not load model %s\n", filename);
			delete new_object;
			return false;

		}

	}

	_cprintf("Loaded %d vertices and %d faces in %d meshes from model %s\n",
		new_object->getNumVertices(true), new_object->getNumTriangles(true),
		new_object->getNumDescendants(true), filename);

	if (transform_to_apply == XFORMOP_USESUPPLIED) {
		if (xform_data == 0) {
			_cprintf("No xform data supplied, not transforming...\n");
			transform_to_apply = XFORMOP_NONE;
		}
	}

	// Transform the mesh if necessary
	if (transform_to_apply != XFORMOP_NONE) {

		float scale_factor;
		cVector3d offset;


		if (transform_to_apply == XFORMOP_AUTO) {

			// Tell him to compute a bounding box...
			new_object->computeBoundaryBox(true);

			_cprintf("\nBefore xform, mesh COB is %s (%s,%s)\n\n",
				new_object->getBoundaryCenter().str(2).c_str(),
				new_object->getBoundaryMax().str(2).c_str(),
				new_object->getBoundaryMin().str(2).c_str());

			cVector3d min = new_object->getBoundaryMin();
			cVector3d max = new_object->getBoundaryMax();

			// This is the "size" of the object
			cVector3d span = max;
			span.sub(min);

			// Find his maximum dimension
			float max_size = (float)(span.x);
			if (span.y > max_size) max_size = (float)(span.y);
			if (span.z > max_size) max_size = (float)(span.z);

			// We'll center all vertices, then multiply by this amount,
			// to scale to the desired size.
			scale_factor = (float)(AUTOXFORM_MESH_SIZE / max_size);

			// To center vertices, we add this amount (-1 times the
			// center of the object's bounding box)
			offset = max;
			offset.add(min);
			offset.div(2.0);
			offset.negate();

			// offset = cMul(-1.0,new_object->getCenterOfMass(1));

			// return the transform data to the user if requested
			if (xform_data) {
				xform_data->model_offset = offset;
				xform_data->model_scale_factor = scale_factor;
			}

		}

		else if (transform_to_apply == XFORMOP_USESUPPLIED) {
			offset = xform_data->model_offset;
			scale_factor = (float)(xform_data->model_scale_factor);
		}

		_cprintf("Offsetting by (%lf,%lf,%lf), scaling by %f\n",
			offset.x, offset.y, offset.z, scale_factor);

		// Now we need to actually scale all the vertices.  However, the
		// vertices might not actually be in this object; they might
		// be in children or grand-children of this mesh (depending on how the 
		// model was defined in the file).
		// 
		// So we find all the sub-meshes we loaded from this file, by descending
		// through all available children.

		// This will hold all the meshes we need to operate on... we'll fill
		// it up as we find more children.
		std::list<cMesh*> meshes_to_scale;

		// This will hold all the parents we're still searching...
		std::list<cMesh*> meshes_to_descend;
		meshes_to_descend.push_front(new_object);

		// Keep track of how many meshes we've found, just to print
		// it out for the user
		int total_meshes = 0;

		// While there are still parent meshes to process
		while (meshes_to_descend.empty() == 0) {

			total_meshes++;

			// Grab the next parent
			cMesh* cur_mesh = meshes_to_descend.front();
			meshes_to_descend.pop_front();
			meshes_to_scale.push_back(cur_mesh);

			// Put all his children on the list of parents to process
			for (unsigned int i = 0; i < cur_mesh->getNumChildren(); i++) {

				cGenericObject* cur_object = cur_mesh->getChild(i);

				// Only process cMesh children
				cMesh* cur_mesh = dynamic_cast<cMesh*>(cur_object);
				if (cur_mesh) {
					// _cprintf("Found a mesh: %s\n",cur_mesh->m_objectName);
					meshes_to_descend.push_back(cur_mesh);
				}
			}
		}

		std::list<cMesh*>::iterator mesh_iter;

		// Now loop over _all_ the meshes we found...
		for (mesh_iter = meshes_to_scale.begin(); mesh_iter != meshes_to_scale.end(); mesh_iter++) {

			cMesh* cur_mesh = *mesh_iter;
			vector<cVertex>* vertices = cur_mesh->pVertices();
			int num_vertices = cur_mesh->getNumVertices(false);
			cVertex* cur_vertex = (cVertex*)(vertices);

			// Move and scale each vertex in this mesh...
			for (int i = 0; i < num_vertices; i++) {
				cur_vertex = cur_mesh->getVertex(i);
				cVector3d pos = cur_vertex->getPos();
				pos.add(offset);
				pos.mul(scale_factor);
				cur_vertex->setPos(pos);
				cur_vertex++;
			}

			// If we loaded a tet mesh and we're supposed to be keeping
			// track of face centers and normals
#if COMPUTE_FACE_CENTERS_AND_NORMALS
			cTetMesh* tmesh = dynamic_cast<cTetMesh*> (cur_mesh);
			if (tmesh && tmesh->face_centers) {
				std::vector<cTriangle>* tri_vector = tmesh->pTriangles();
				unsigned int ntriangles = tri_vector->size();
				for (unsigned int tri = 0; tri < ntriangles; tri++) {
					tmesh->face_centers[tri].add(offset);
					tmesh->face_centers[tri].mul(scale_factor);
				}
			}
#endif

		}

	}

	// Re-compute a bounding box
	new_object->computeBoundaryBox(true);

	if (build_collision_detector) {
		_cprintf("Building collision detector...\n");
		// new_object->createSphereTreeCollisionDetector(true,true);
		new_object->createAABBCollisionDetector(true, true);
		_cprintf("Finished building collision detector...\n");
	}

	new_object->computeGlobalPositions();

	_cprintf("After xform, mesh COB is %s\n\n", new_object->getBoundaryCenter().str(2).c_str());

	if (finalize) {
		_cprintf("Finalizing mesh for rendering...\n");
		new_object->finalize(true);
		_cprintf("Finished finalizing mesh for rendering...\n");
	}

	if (selected_filename) {
		strcpy(selected_filename, filename);
	}
	return true;
}