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

#ifndef _MESH_IMPORTER_H_
#define _MESH_IMPORTER_H_

#include "tetgen_loader.h"
#include "cLabelPanel.h"

typedef cVBOMesh default_mesh_type;

struct mesh_xform_information {
	cVector3d model_offset;
	double model_scale_factor;
};

typedef enum {
	XFORMOP_AUTO, XFORMOP_NONE, XFORMOP_USESUPPLIED
} transform_operations;
typedef transform_operations transform_operation;

// I'm going to scale the object so his maximum axis has a
// size of AUTOXFORM_MESH_SIZE. This will make him fit nicely in
// our viewing area.
#define AUTOXFORM_MESH_SIZE 2.0

bool importModel(const char* in_filename, cMesh*& new_object, cWorld* world,
	bool build_collision_detector = false, bool finalize = true,
	cMesh* factory = 0,

	cLabelPanel** new_label_panel = 0,

	transform_operation transform_to_apply = XFORMOP_AUTO,
	mesh_xform_information* transform_data = 0,
	char* selected_filename = 0
);

#endif