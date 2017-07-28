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

// File format for the voxelizer's binary output file
//
// The file begins with a voxelfile_file_header, which tells you
// how many objects there are in the file (always one so far).
//
// Objects follow immediately after, and each object consists of:
//
// One voxelfile_object_header struct, which tells you how many
// voxels there are in this object.
//
// Voxels follow immediately after, and _every voxel_ (i*j*k) is represented
// in the file.  Each voxel consists of a one-character header, typically 
// ASCIIVOXEL_NOVOXEL or ASCIIVOXEL_TEXTURED_VOXEL.  If it's ASCIIVOXEL_NOVOXEL,
// the next voxel follows immediately.  If it's ASCIIVOXEL_TEXTURED_VOXEL, a
// voxelfile_voxel struct follows with more information.
//
// All data is aligned to one byte (no padding).
//
// When loading a model file that should line up with this voxel file,
// apply the offset _before_ the scale factor

#ifndef _VOXEL_FILE_FORMAT_H_
#define _VOXEL_FILE_FORMAT_H_

// No voxel at this coordinate
#define ASCIIVOXEL_NOVOXEL '0'

// "Plain" voxel at this coordinate (not really used any more)
#define ASCIIVOXEL_INTERNAL_VOXEL '1'

// Voxel to be followed by more information
#define ASCIIVOXEL_TEXTURED_VOXEL '2'

#define MAX_NUM_MODIFIERS 5

#pragma pack(push)
#pragma pack(1)

struct voxelfile_file_header {

	// Size of this structure
	int header_size;

	// Number of objects in this file
	int num_objects;

	// Size of the object header
	int object_header_size;

	// Size of the structure that represents each voxel
	int voxel_struct_size;

};

struct voxelfile_object_header {

	// Number of voxels in this object
	int num_voxels;

	// Maximum number of voxels along each axis
	int voxel_resolution[3];

	// Size of a voxel along each axis
	float voxel_size[3];

	// The scale factor that we multiplied the original
	// model by to get our current model
	float model_scale_factor;

	// The vector by which we offset the original model
	// to get our current model (applied _after_ scaling)
	float model_offset[3];

	// The world-space coordinate of the extreme corner of the (0,0,0)
	// voxel, after applying the previously-described scale and offset
	float zero_coordinate[3];

	// Set to 1 if this object has texture coordinates assigned
	// to each voxel
	unsigned char has_texture;

	// Filled in with a local pathname if a texture has been 
	// saved with this file
	char texture_filename[_MAX_PATH];

	voxelfile_object_header() {
		num_voxels = 0;
		voxel_resolution[0] = voxel_resolution[1] = voxel_resolution[2] = 0;
		voxel_size[0] = voxel_size[1] = voxel_size[2] = 0;
		model_scale_factor = 1.0;
		model_offset[0] = model_offset[1] = model_offset[2] = 0;
		zero_coordinate[0] = zero_coordinate[1] = zero_coordinate[2] = 0;
		has_texture = 0;
		texture_filename[0] = '\0';
	}

};


struct voxelfile_voxel {

	// Location of this voxel
	short i, j, k;

	// Set to 1 if this voxel has texture coordinates assigned
	// to it
	unsigned char has_texture;

	// Texture coordinate, if any, associated with this
	// voxel
	float u, v;

	// Set to 1 if this voxel has a surface normal assigned to it
	unsigned char has_normal;

	// Surface normal, if any, associated with this voxel
	float normal[3];

	// Set to 1 if the distance and gradient fields are meaningful
	unsigned char has_distance;

	// The current (or initial) shortest distance to the surface
	float distance_to_surface;

	// The vector from the center of this voxel to the (initial)
	// closest point on the surface
	float distance_gradient[3];

	// Set to 0 if the 'modifier distance' fields are not meaningful
	unsigned char num_modifiers;

	// The distance and vector to the mesh that was selected as a
	// 'modifier' (basically a mesh that we want to know the distance
	// to, other than the voxelized mesh itself)
	float distance_to_modifier[MAX_NUM_MODIFIERS];
	float modifier_gradient[MAX_NUM_MODIFIERS][3];

	// Set to non-zero if this voxel is on the border of the object
	unsigned char is_on_border;

	inline void initialize(int i, int j, int k) {

		this->i = i;
		this->j = j;
		this->k = k;
		has_texture = 0;
		u = v = 0.0;
		has_normal = 0;
		normal[0] = normal[1] = normal[2] = 0;
		distance_to_surface = 0;
		distance_gradient[0] = distance_gradient[1] = distance_gradient[2] = 0;
		has_distance = 0;
		num_modifiers = 0;
		is_on_border = 0;
		memset(distance_to_modifier, 0, sizeof(distance_to_modifier));
		for (int index = 0; index < MAX_NUM_MODIFIERS; index++) memset(modifier_gradient + index, 0, 3 * sizeof(float));

	}

	voxelfile_voxel() {
		initialize(0, 0, 0);
	}

	voxelfile_voxel(int i, int j, int k) {
		initialize(i, j, k);
	}

};

#pragma pack(pop)

#endif