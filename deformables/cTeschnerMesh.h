#ifndef _TESCHNER_MESH_H_
#define _TESCHNER_MESH_H_

#include "cTetMesh.h"
#include "CVector3f.h"
#include "constraints.h"
#include "deformable_mesh_data_structures.h"
#include <vector>
#include <map>

#define DEFAULT_FLOOR_POSITION -1.0

typedef enum {
	FIXED_TET_DENSITY = 0, FIXED_TOTAL_MASS, FIXED_VERTEX_MASS
} mass_assignment_strategies;

typedef cTetMesh teschner_mesh_parent_mesh_type;

struct temp_vertex_info {
	std::vector<unsigned int> containing_tets;
	std::vector<unsigned int> opposing_faces;
	std::vector<unsigned int> containing_faces;
	std::vector<unsigned int> containing_edges;
};

struct external_material_properties {
	double youngs_modulus;
	double poisson_coeff;

	external_material_properties() {
		youngs_modulus = 100000.0;
		poisson_coeff = 0.3;
	}
};

struct solid_section {
	external_material_properties props;
	std::vector<unsigned int> tets;
};

typedef std::list<solid_section*> solid_section_list;

// Defines a force that an external object has left on some of my
// vertices
struct external_force {

	// some unique number that the user comes up with
	unsigned int tag;

	// which vertices does this apply to?
	std::vector<unsigned int> vertices;

	// the force vector itself
	cVector3f force;
};

typedef enum {
	KVOLUME = 0, KAREA, KDISTANCE, KDAMPING
} constant_names;

struct pending_constant_assignment {
	int constant_name;
	std::list<unsigned int> vertices;
	double value;
};

typedef std::map<int, external_force*, std::less<int> > force_map;
typedef std::list<external_force*> force_list;

class cTeschnerMesh : public teschner_mesh_parent_mesh_type {

public:
	cTeschnerMesh(cWorld* a_parentWorld);
	virtual ~cTeschnerMesh();

	// Stores properties that will just be used for outputting to Abaqus
	external_material_properties material_props;
	solid_section_list solid_sections;

	// users can throw whatever forces they want into this list
	// 
	// forces should be in _world_ space
	// force_map external_forces;
	force_list external_forces;

	float m_maximum_vertex_acceleration;
	float m_maximum_vertex_velocity;
	float m_mean_vertex_acceleration;
	float m_mean_vertex_velocity;

	// computes areas + volumes + lengths, then calls compute_forces,
	// then calls move_vertices
	//
	// The user should call this once per timestep.
	//
	// Returns true for success, false for errors.
	bool tick();

	// Copy relevant vertex data to the rendering data structures (for now)
	//
	// Not called internally; should be called by the user at relevant
	// timesteps.
	//
	// This is not necessarily called at every simulation timestep.
	void prepare_to_render();

	// Compute fixed connectivity data structures
	// 
	// Returns -1 if there's an error, zero if all goes well
	int initialize(cTeschnerMesh* old_model = 0);

	// Reset mesh to initial state
	int reset();

	int clear_constraints();

	virtual void renderMesh(const int a_renderMode = 0);

	inline virtual cMesh* createMesh() const { return new cTeschnerMesh(m_parentWorld); }

	// _Add_ a point force to a single vertex
	virtual void applyForce(unsigned int vertex_index, cVector3f& force);

	// The current simulation time
	float m_current_sim_time;

	// Used for binary copying
	unsigned char BEGIN_CONSTANTS;

	bool m_gravity_enabled;
	bool m_floor_force_enabled;

	// The timestep used for integration
	float m_timestep;

	// Spring constants... 
	float m_kVolumePreservation;
	float m_kAreaPreservation;
	float m_kDistancePreservation;
	float m_kDistanceDamping;

	// This is either _force_ or _acceleration_, depending
	// on GRAVIY_IS_ACCEL
	float m_gravity_force;
	float m_kFloor;

	float m_external_force_scale;
	float m_time_force_exponent;

	// A position on the y axis, only meaningful if the
	// floor is enabled
	float m_floor_position;

	bool m_render_vertex_constraints;

	// The remaining constants need to be set up _before_ initialization

	// How does mass get assigned to vertices?
	int m_mass_assignment_strategy;
	float m_mass_assignment_constant;

	// Should we allow heterogeneous constants?
	bool m_heterogeneous_constants;

	// Bits here indicate that a constant is heterogeneous
	int m_heterogeneous_constant_flags;

	// Used for binary copying
	unsigned char END_CONSTANTS;

	float* m_hetero_kVolumePreservation;
	float* m_hetero_kAreaPreservation;
	float* m_hetero_kDistancePreservation;
	float* m_hetero_kDistanceDamping;

	void prepare_heterogeneous_constant_rendering();

	constraint_set* m_constraints;

	std::list<unsigned int> m_highlighted_vertices;
	std::list<unsigned int> m_highlight_colors;

	cTeschnerMesh* m_proxy_sim_mesh;

	bool m_compute_vertex_normals;
	int m_use_cmesh_normal_computation;

	// SKINNING INFORMATION
	cMesh* m_rendering_mesh;
	unsigned char m_weights_per_vertex;
	bool m_force_render_tet_mesh;
	bool m_disable_skinned_mesh;

	// Each of these is of length:
	//
	// (nvertices in the rendering mesh) * (m_weights_per_vertex)
	int* m_vertex_effectors;
	float* m_vertex_weights;

	int m_use_coordinate_frame_based_skinning;

	// For the normal+tangential approach
	cVector3f* m_vertex_tangential_offsets;
	float* m_vertex_normal_offsets;

	// For the coordinate-frame approach
	cVector3f* m_vertex_coordframe_positions;
	int* m_opposite_edge_vertices;

	void build_rendering_weights();

	void clear_solid_sections();

	SYSTEMTIME m_sim_start_date_and_time;

	bool m_initialized;

public:

	// METHODS

	// Move all my vertices based on current forces (via Verlet integration)
	// * This is where the current and previous position arrays get swapped
	// * This is also where the current set of forces gets zero'd at each
	//   iteration... so after this is called, the _previous_ vertex
	//   array holds the most recent set of forces.
	bool move_vertices();

	// Compute forces (called at each iteration) based on current areas, etc.
	//
	// Should be called _after_ the face/tet/spring state computation functions.
	bool compute_forces();

	void compute_internal_forces();
	void compute_contact_forces();

	// Compute the area and surface normal of each triangle
	bool compute_face_areas();

	// Compute the volume of each tet
	//
	// Returns false if tet volumes suggest that the simln has gone bad
	bool compute_tet_volumes();

	// Compute the length of each spring
	bool compute_spring_lengths();

	// Compute the mass of each vertex based on tet volumes
	void initialize_vertex_masses();

	/***
	INITIALIZATION INFORMATION
	***/
	std::list<pending_constant_assignment> m_pending_constant_assignments;
	void process_pending_constant_assignments();
	void initialize_nonhomogeneity();

	/***
	DATA THAT CHANGES AT EACH ITERATION
	***/

	// VARIABLE: surface normals and areas (magnitude) for each triangle
	cVector3f* m_faceNormalsAndAreas;

	// VARIABLE: lengths for each edge
	float* m_edgeLengths;

	// VARIABLE: current tet volumes
	float* m_tetVolumes;

	// VARIABLE: vertex data
	unsigned int m_nVertices;

	cDeformableVertex* m_deformableVertices;

	// VARIABLE: positions (etc.) of each vertex in the previous frame
	cDeformableVertex* m_previousPositions;

	/***
	Backup information for mesh restoration
	***/
	cVector3f* m_bkup_faceNormalsAndAreas;
	float* m_bkup_edgeLengths;
	float* m_bkup_tetVolumes;
	cDeformableVertex* m_bkup_deformableVertices;

	// Not necessary to store this, just a reminder...
	// cDeformableVertex* m_bkup_previousPositions;


	/***
	Rest-state information
	***/

	// FIXED: rest length for each edge
	float* m_restEdgeLengths;

	// FIXED: rest area for each face
	cVector3f* m_restFaceNormalsAndAreas;

	// FIXED: rest volume for each tet
	float* m_restTetVolumes;

	/***
	FIXED CONNECTIVITY INFORMATION
	***/

	// FIXED: not necessarily used for rendering; used for computation
	face* m_faces;
	unsigned int m_nFaces;
	edge* m_edges;
	unsigned int m_nEdges;
	// unsigned int* cTetMesh::m_tets
	// unsigned int cTetMesh::m_nTets

	// Each vertex stores 6 attributes (see cDeformableVertex.h) :
	// 
	// My location in the tet/opposing face array
	// How many tets I belong to
	//
	// My location in the containing edge array
	// How many edges I belong to
	// 
	// My location in the containing face array
	// How many faces I belong to

	// These two are counted/indexed together
	unsigned int* m_containingTets;

	// In this array, a positive # means that the triangle's normal points
	// _toward_ me, a negative # the other way... the absolute value is the 
	// index of the triangle.
	int* m_opposingFaces;

	unsigned int* m_containingEdges;
	unsigned int* m_containingFaces;

};

#endif
