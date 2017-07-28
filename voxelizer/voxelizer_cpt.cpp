#include "stdafx.h"
#include "voxelizer.h"

void CvoxelizerApp::cpt_current_object() {

#ifdef COMPILE_CPT

	_cprintf("Beginning cpt process...\n");

#define MAX_CPT_DISTANCE 100.0
	cpt_initialize_parameters(MAX_CPT_DISTANCE);

	/*
	cpt_initialize_grid( const cpt::number_type* cartesian_domain,
				 const int* grid_dimensions,
				 cpt::number_type* distance,
				 cpt::number_type* gradient_distance,
				 cpt::number_type* closest_point,
				 int* closest_face );
	*/

	// The first parameter specifies the bounding box
	// as xmin,ymin,zmin,xmax,ymax,zmax
	cpt::number_type bbox[6];

	cVector3d object_min = object_to_voxelize->getBoundaryMin();
	cVector3d object_max = object_to_voxelize->getBoundaryMax();
	bbox[0] = object_min.x;
	bbox[1] = object_min.y;
	bbox[2] = object_min.z;
	bbox[3] = object_max.x;
	bbox[4] = object_max.y;
	bbox[5] = object_max.z;

	// The second parameter specifies the resolution of voxelization on
	// each axis... that's currently stored in voxel_resolution

	// Now we need to allocate arrays of appropriate size for the distance,
	// gradient, closest point, and closest face arrays...

	// Compute the total number of voxels
	int total_num_voxels =
		voxel_resolution[0] * voxel_resolution[1] * voxel_resolution[2];

	if (m_distance_array)      delete[] m_distance_array;
	if (m_closest_point_array) delete[] m_closest_point_array;
	if (m_gradient_array)      delete[] m_gradient_array;
	if (m_closest_face_array)  delete[] m_closest_face_array;

	m_distance_array = new cpt::number_type[total_num_voxels];
	m_closest_point_array = 0; // new cpt::number_type[total_num_voxels*3];
	m_gradient_array = 0; // new cpt::number_type[total_num_voxels*3];
	m_closest_face_array = 0; // new int[total_num_voxels];

	cpt_initialize_grid(bbox, voxel_resolution,
		m_distance_array, m_gradient_array, m_closest_point_array, m_closest_face_array);

	// Now we need to initialize the boundary representation from
  // the vertex and triangle arrays, which will require some copying...

  /*
  cpt_initialize_brep_noclip( int num_vertices,
				  const cpt::number_type* vertices,
				  int num_faces,
				  const int* faces );
  */

	_cprintf("Starting mesh transform...\n");

	// To build these arrays, we need to walk over all children of the object
	// we're voxelizing...
	int n_vertices = object_to_voxelize->getNumVertices(true);
	int n_faces = object_to_voxelize->getNumTriangles(true);

	cpt::number_type* vertices = new cpt::number_type[n_vertices * 3];
	int* faces = new int[n_faces * 3];

	// This will hold all the parents we're still searching...
	std::list<cMesh*> meshes_to_descend;
	meshes_to_descend.push_front(object_to_voxelize);

	int cur_vertex_offset = 0;
	int cur_triangle_offset = 0;

	int i;

	// While there are still parent meshes to process
	while (meshes_to_descend.empty() == 0) {

		// Grab the next parent
		cMesh* cur_mesh = meshes_to_descend.front();
		meshes_to_descend.pop_front();

		// Put all his children on the list of parents to process
		for (i = 0; i < cur_mesh->getNumChildren(); i++) {

			cGenericObject* cur_object = cur_mesh->getChild(i);

			// Only process cMesh children
			cMesh* child_mesh = dynamic_cast<cMesh*>(cur_object);
			if (child_mesh) meshes_to_descend.push_back(child_mesh);
		}

		int local_n_vertices = cur_mesh->getNumVertices();
		int local_n_triangles = cur_mesh->getNumTriangles();

		// Grab all his faces and vertices, transforming indices
		// appropriately...

		vector<cVertex>* local_vertices = cur_mesh->pVertices();
		vector<cTriangle>* local_faces = cur_mesh->pTriangles();

		for (i = 0; i < local_n_vertices; i++) {

			cVertex v = (*local_vertices)[i];

			// Note that we use _global_ position...
			cVector3d pos = v.getGlobalPos();

			vertices[(i + cur_vertex_offset) * 3 + 0] = pos.x;
			vertices[(i + cur_vertex_offset) * 3 + 1] = pos.y;
			vertices[(i + cur_vertex_offset) * 3 + 2] = pos.z;

		}

		for (i = 0; i < local_n_triangles; i++) {

			cTriangle t = (*local_faces)[i];

			// Transform vertices into the new list...
			int a = t.getIndexVertex0() + cur_vertex_offset;
			int b = t.getIndexVertex1() + cur_vertex_offset;
			int c = t.getIndexVertex2() + cur_vertex_offset;

			faces[(i + cur_triangle_offset) * 3 + 0] = a;
			faces[(i + cur_triangle_offset) * 3 + 1] = b;
			faces[(i + cur_triangle_offset) * 3 + 2] = c;

		}

		// Update the relevant offsets
		cur_vertex_offset += local_n_vertices;
		cur_triangle_offset += local_n_triangles;

	}

	_cprintf("Finished mesh transform...\n");

	cpt_initialize_brep(n_vertices, vertices, n_faces, faces);

	_cprintf("Starting closest-point transform...\n");

	cpt_closest_point_transform();

	_cprintf("Finished closest-point transform...\n");

#define FAR_AWAY 1000.0
	cpt_flood_fill_distance(FAR_AWAY);

	/*
	int valid = cpt_is_grid_valid();
	_cprintf("Valid is %d\n",valid);
	*/

	// The world-space coordinate of the (0,0,0) voxel is now at 0,0,0
	zero_coordinate.set(voxel_size.x / 2.0, voxel_size.y / 2.0, voxel_size.z / 2.0);

	// Write out binary file header and object header
	FILE* binary_f = fopen("c:\\users\\dmorris\\tmp.voxels", "wb");

	int found_voxels = 0;
	for (i = 0; i < total_num_voxels; i++) {
		if ((m_distance_array[i] <= 0) && (m_distance_array[i] != FAR_AWAY)) found_voxels++;
	}

	if (binary_f == 0) {

		_cprintf("Could not open binary output file %s\n", "hello");

	}

	else {

		voxelfile_file_header file_hdr;
		file_hdr.header_size = sizeof(file_hdr);
		file_hdr.num_objects = 1;

		fwrite(&file_hdr, sizeof(file_hdr), 1, binary_f);

		voxelfile_object_header object_hdr;
		object_hdr.header_size = sizeof(object_hdr);

		int k;

		for (k = 0; k < 3; k++) object_hdr.voxel_resolution[k] = voxel_resolution[k];
		for (k = 0; k < 3; k++) object_hdr.voxel_size[k] = voxel_size.get(k);

		// The zero_coordinate vector now represents the center of the
		// (0,0,0) voxel, after moving and offsetting to put the ll corner
		// of the bounding box at 0.  So now I want to add back in that
		// translation...
		zero_coordinate.add(old_object_pos);
		zero_coordinate.add(object_min);
		zero_coordinate.sub(voxel_size.x / 2.0, voxel_size.y / 2.0, voxel_size.z / 2.0);

		// This gives us the zero coordinate in the post-scaling,
		// post-offset world

		_cprintf("Zero coordinate going out to file: (%lf,%lf,%lf)\n",
			zero_coordinate.x, zero_coordinate.y, zero_coordinate.z);

		for (k = 0; k < 3; k++) object_hdr.zero_coordinate[k] = zero_coordinate.get(k);

		// These represent the transformation we performed when we first
		// loaded the object: a translation, then a scale
		for (k = 0; k < 3; k++) object_hdr.model_offset[k] = model_offset.get(k);
		object_hdr.model_scale_factor = model_scale_factor;

		object_hdr.num_voxels = found_voxels;

		_cprintf("\nWriting %d voxels to the binary output file\n", object_hdr.num_voxels);

		_cprintf("\nResolution %d,%d,%d, size %f,%f,%f\n",
			object_hdr.voxel_resolution[0], object_hdr.voxel_resolution[1], object_hdr.voxel_resolution[2],
			object_hdr.voxel_size[0], object_hdr.voxel_size[1], object_hdr.voxel_size[2]);

		object_hdr.has_texture = 0;

		//strcpy(object_hdr.texture_filename,current_texture);
		//_cprintf("Storing texture %s\n",current_texture);

		fwrite(&object_hdr, sizeof(object_hdr), 1, binary_f);

		for (int i = 0; i < voxel_resolution[0]; i++) {
			for (int j = 0; j < voxel_resolution[1]; j++) {
				for (int k = 0; k < voxel_resolution[2]; k++) {

					int index = k * (voxel_resolution[0] * voxel_resolution[1]) +
						j*voxel_resolution[0] +
						i;

					double d = m_distance_array[index];

					if (d > 0 || fabs(d) == FAR_AWAY) fputc(ASCIIVOXEL_NOVOXEL, binary_f);


					else fputc(ASCIIVOXEL_INTERNAL_VOXEL, binary_f);

				}
			}
		}

		fclose(binary_f);
	}

	_cprintf("Found %d voxels (%d empty)\n", found_voxels, total_num_voxels - found_voxels);

	delete[] vertices;
	delete[] faces;

#endif // if I'm compiling cpt

}
