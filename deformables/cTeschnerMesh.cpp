#include "cTeschnerMesh.h"
#include "ann_1.1/include/ann/ANN.h"

#pragma comment(lib,"ann_1.1/lib/ANN.lib")

#define DEFAULT_TIMESTEP   0.00005
#define DEFAULT_K_VOLUME   100000.0
#define DEFAULT_K_AREA     1000.0
#define DEFAULT_K_DISTANCE 1000000.0
#define DEFAULT_DISTANCE_DAMPING 20.0

#define DEFAULT_PARTICLE_MASS 0.1
#define DEFAULT_TET_DENSITY 100.0
#define DEFAULT_TOTAL_MASS 1.0

#define GRAVITY_IS_ACCELERATION 0
#define DEFAULT_GRAVITY_ACCELERATION 10.0
#define DEFAULT_GRAVITY_FORCE 1000.0

#define DEFAULT_MASS_ASSIGNMENT_STRATEGY FIXED_VERTEX_MASS

#define MAX_BAD_TET_COUNT 10

#define WEIGHTS_PER_VERTEX 5
#define EXTRA_NEIGHBORS_TO_FIND 0
// #define DEBUG_VERTEX_INDEX 1000
int g_debug_vertex_index = 0;

// Used to throw out degenerate tets
#define PARALLEL_LINE_THRESHOLD 0.9995

// Debugging forces
#define DEFAULT_K_FLOOR 1.0e6 // 30000.0

// Restricts penetration entirely, rather than applying a constraint
// force
// #define RESTRICT_FLOOR_PENETRATION


#include <set>
#include <map>
#include <math.h>
#include <float.h>
#include "CVertex.h"
using std::set;
using std::map;

#define sign(x) (x>=0?1:-1)

#define SAFE_ARRAY_DELETE(p) { if(p) { delete [] (p); (p)=NULL; } }


static float colormap[256][3] = {
  {0.000000f,0.000000f,0.562500f},
  {0.000000f,0.000000f,0.577941f},
  {0.000000f,0.000000f,0.593382f},
  {0.000000f,0.000000f,0.608824f},
  {0.000000f,0.000000f,0.624265f},
  {0.000000f,0.000000f,0.639706f},
  {0.000000f,0.000000f,0.655147f},
  {0.000000f,0.000000f,0.670588f},
  {0.000000f,0.000000f,0.686029f},
  {0.000000f,0.000000f,0.701471f},
  {0.000000f,0.000000f,0.716912f},
  {0.000000f,0.000000f,0.732353f},
  {0.000000f,0.000000f,0.747794f},
  {0.000000f,0.000000f,0.763235f},
  {0.000000f,0.000000f,0.778676f},
  {0.000000f,0.000000f,0.794118f},
  {0.000000f,0.000000f,0.809559f},
  {0.000000f,0.000000f,0.825000f},
  {0.000000f,0.000000f,0.840441f},
  {0.000000f,0.000000f,0.855882f},
  {0.000000f,0.000000f,0.871324f},
  {0.000000f,0.000000f,0.886765f},
  {0.000000f,0.000000f,0.902206f},
  {0.000000f,0.000000f,0.917647f},
  {0.000000f,0.000000f,0.933088f},
  {0.000000f,0.000000f,0.948529f},
  {0.000000f,0.000000f,0.963971f},
  {0.000000f,0.000000f,0.979412f},
  {0.000000f,0.000000f,0.994853f},
  {0.000000f,0.010294f,1.000000f},
  {0.000000f,0.025735f,1.000000f},
  {0.000000f,0.041176f,1.000000f},
  {0.000000f,0.056618f,1.000000f},
  {0.000000f,0.072059f,1.000000f},
  {0.000000f,0.087500f,1.000000f},
  {0.000000f,0.102941f,1.000000f},
  {0.000000f,0.118382f,1.000000f},
  {0.000000f,0.133824f,1.000000f},
  {0.000000f,0.149265f,1.000000f},
  {0.000000f,0.164706f,1.000000f},
  {0.000000f,0.180147f,1.000000f},
  {0.000000f,0.195588f,1.000000f},
  {0.000000f,0.211029f,1.000000f},
  {0.000000f,0.226471f,1.000000f},
  {0.000000f,0.241912f,1.000000f},
  {0.000000f,0.257353f,1.000000f},
  {0.000000f,0.272794f,1.000000f},
  {0.000000f,0.288235f,1.000000f},
  {0.000000f,0.303676f,1.000000f},
  {0.000000f,0.319118f,1.000000f},
  {0.000000f,0.334559f,1.000000f},
  {0.000000f,0.350000f,1.000000f},
  {0.000000f,0.365441f,1.000000f},
  {0.000000f,0.380882f,1.000000f},
  {0.000000f,0.396324f,1.000000f},
  {0.000000f,0.411765f,1.000000f},
  {0.000000f,0.427206f,1.000000f},
  {0.000000f,0.442647f,1.000000f},
  {0.000000f,0.458088f,1.000000f},
  {0.000000f,0.473529f,1.000000f},
  {0.000000f,0.488971f,1.000000f},
  {0.000000f,0.504412f,1.000000f},
  {0.000000f,0.519853f,1.000000f},
  {0.000000f,0.535294f,1.000000f},
  {0.000000f,0.550735f,1.000000f},
  {0.000000f,0.566176f,1.000000f},
  {0.000000f,0.581618f,1.000000f},
  {0.000000f,0.597059f,1.000000f},
  {0.000000f,0.612500f,1.000000f},
  {0.000000f,0.627941f,1.000000f},
  {0.000000f,0.643382f,1.000000f},
  {0.000000f,0.658824f,1.000000f},
  {0.000000f,0.674265f,1.000000f},
  {0.000000f,0.689706f,1.000000f},
  {0.000000f,0.705147f,1.000000f},
  {0.000000f,0.720588f,1.000000f},
  {0.000000f,0.736029f,1.000000f},
  {0.000000f,0.751471f,1.000000f},
  {0.000000f,0.766912f,1.000000f},
  {0.000000f,0.782353f,1.000000f},
  {0.000000f,0.797794f,1.000000f},
  {0.000000f,0.813235f,1.000000f},
  {0.000000f,0.828676f,1.000000f},
  {0.000000f,0.844118f,1.000000f},
  {0.000000f,0.859559f,1.000000f},
  {0.000000f,0.875000f,1.000000f},
  {0.000000f,0.890441f,1.000000f},
  {0.000000f,0.905882f,1.000000f},
  {0.000000f,0.921324f,1.000000f},
  {0.000000f,0.936765f,1.000000f},
  {0.000000f,0.952206f,1.000000f},
  {0.000000f,0.967647f,1.000000f},
  {0.000000f,0.983088f,1.000000f},
  {0.000000f,0.998529f,1.000000f},
  {0.013971f,1.000000f,0.986029f},
  {0.029412f,1.000000f,0.970588f},
  {0.044853f,1.000000f,0.955147f},
  {0.060294f,1.000000f,0.939706f},
  {0.075735f,1.000000f,0.924265f},
  {0.091176f,1.000000f,0.908824f},
  {0.106618f,1.000000f,0.893382f},
  {0.122059f,1.000000f,0.877941f},
  {0.137500f,1.000000f,0.862500f},
  {0.152941f,1.000000f,0.847059f},
  {0.168382f,1.000000f,0.831618f},
  {0.183824f,1.000000f,0.816176f},
  {0.199265f,1.000000f,0.800735f},
  {0.214706f,1.000000f,0.785294f},
  {0.230147f,1.000000f,0.769853f},
  {0.245588f,1.000000f,0.754412f},
  {0.261029f,1.000000f,0.738971f},
  {0.276471f,1.000000f,0.723529f},
  {0.291912f,1.000000f,0.708088f},
  {0.307353f,1.000000f,0.692647f},
  {0.322794f,1.000000f,0.677206f},
  {0.338235f,1.000000f,0.661765f},
  {0.353676f,1.000000f,0.646324f},
  {0.369118f,1.000000f,0.630882f},
  {0.384559f,1.000000f,0.615441f},
  {0.400000f,1.000000f,0.600000f},
  {0.415441f,1.000000f,0.584559f},
  {0.430882f,1.000000f,0.569118f},
  {0.446324f,1.000000f,0.553676f},
  {0.461765f,1.000000f,0.538235f},
  {0.477206f,1.000000f,0.522794f},
  {0.492647f,1.000000f,0.507353f},
  {0.508088f,1.000000f,0.491912f},
  {0.523529f,1.000000f,0.476471f},
  {0.538971f,1.000000f,0.461029f},
  {0.554412f,1.000000f,0.445588f},
  {0.569853f,1.000000f,0.430147f},
  {0.585294f,1.000000f,0.414706f},
  {0.600735f,1.000000f,0.399265f},
  {0.616176f,1.000000f,0.383824f},
  {0.631618f,1.000000f,0.368382f},
  {0.647059f,1.000000f,0.352941f},
  {0.662500f,1.000000f,0.337500f},
  {0.677941f,1.000000f,0.322059f},
  {0.693382f,1.000000f,0.306618f},
  {0.708824f,1.000000f,0.291176f},
  {0.724265f,1.000000f,0.275735f},
  {0.739706f,1.000000f,0.260294f},
  {0.755147f,1.000000f,0.244853f},
  {0.770588f,1.000000f,0.229412f},
  {0.786029f,1.000000f,0.213971f},
  {0.801471f,1.000000f,0.198529f},
  {0.816912f,1.000000f,0.183088f},
  {0.832353f,1.000000f,0.167647f},
  {0.847794f,1.000000f,0.152206f},
  {0.863235f,1.000000f,0.136765f},
  {0.878676f,1.000000f,0.121324f},
  {0.894118f,1.000000f,0.105882f},
  {0.909559f,1.000000f,0.090441f},
  {0.925000f,1.000000f,0.075000f},
  {0.940441f,1.000000f,0.059559f},
  {0.955882f,1.000000f,0.044118f},
  {0.971324f,1.000000f,0.028676f},
  {0.986765f,1.000000f,0.013235f},
  {1.000000f,0.997794f,0.000000f},
  {1.000000f,0.982353f,0.000000f},
  {1.000000f,0.966912f,0.000000f},
  {1.000000f,0.951471f,0.000000f},
  {1.000000f,0.936029f,0.000000f},
  {1.000000f,0.920588f,0.000000f},
  {1.000000f,0.905147f,0.000000f},
  {1.000000f,0.889706f,0.000000f},
  {1.000000f,0.874265f,0.000000f},
  {1.000000f,0.858824f,0.000000f},
  {1.000000f,0.843382f,0.000000f},
  {1.000000f,0.827941f,0.000000f},
  {1.000000f,0.812500f,0.000000f},
  {1.000000f,0.797059f,0.000000f},
  {1.000000f,0.781618f,0.000000f},
  {1.000000f,0.766176f,0.000000f},
  {1.000000f,0.750735f,0.000000f},
  {1.000000f,0.735294f,0.000000f},
  {1.000000f,0.719853f,0.000000f},
  {1.000000f,0.704412f,0.000000f},
  {1.000000f,0.688971f,0.000000f},
  {1.000000f,0.673529f,0.000000f},
  {1.000000f,0.658088f,0.000000f},
  {1.000000f,0.642647f,0.000000f},
  {1.000000f,0.627206f,0.000000f},
  {1.000000f,0.611765f,0.000000f},
  {1.000000f,0.596324f,0.000000f},
  {1.000000f,0.580882f,0.000000f},
  {1.000000f,0.565441f,0.000000f},
  {1.000000f,0.550000f,0.000000f},
  {1.000000f,0.534559f,0.000000f},
  {1.000000f,0.519118f,0.000000f},
  {1.000000f,0.503676f,0.000000f},
  {1.000000f,0.488235f,0.000000f},
  {1.000000f,0.472794f,0.000000f},
  {1.000000f,0.457353f,0.000000f},
  {1.000000f,0.441912f,0.000000f},
  {1.000000f,0.426471f,0.000000f},
  {1.000000f,0.411029f,0.000000f},
  {1.000000f,0.395588f,0.000000f},
  {1.000000f,0.380147f,0.000000f},
  {1.000000f,0.364706f,0.000000f},
  {1.000000f,0.349265f,0.000000f},
  {1.000000f,0.333824f,0.000000f},
  {1.000000f,0.318382f,0.000000f},
  {1.000000f,0.302941f,0.000000f},
  {1.000000f,0.287500f,0.000000f},
  {1.000000f,0.272059f,0.000000f},
  {1.000000f,0.256618f,0.000000f},
  {1.000000f,0.241176f,0.000000f},
  {1.000000f,0.225735f,0.000000f},
  {1.000000f,0.210294f,0.000000f},
  {1.000000f,0.194853f,0.000000f},
  {1.000000f,0.179412f,0.000000f},
  {1.000000f,0.163971f,0.000000f},
  {1.000000f,0.148529f,0.000000f},
  {1.000000f,0.133088f,0.000000f},
  {1.000000f,0.117647f,0.000000f},
  {1.000000f,0.102206f,0.000000f},
  {1.000000f,0.086765f,0.000000f},
  {1.000000f,0.071324f,0.000000f},
  {1.000000f,0.055882f,0.000000f},
  {1.000000f,0.040441f,0.000000f},
  {1.000000f,0.025000f,0.000000f},
  {1.000000f,0.009559f,0.000000f},
  {0.994118f,0.000000f,0.000000f},
  {0.978676f,0.000000f,0.000000f},
  {0.963235f,0.000000f,0.000000f},
  {0.947794f,0.000000f,0.000000f},
  {0.932353f,0.000000f,0.000000f},
  {0.916912f,0.000000f,0.000000f},
  {0.901471f,0.000000f,0.000000f},
  {0.886029f,0.000000f,0.000000f},
  {0.870588f,0.000000f,0.000000f},
  {0.855147f,0.000000f,0.000000f},
  {0.839706f,0.000000f,0.000000f},
  {0.824265f,0.000000f,0.000000f},
  {0.808824f,0.000000f,0.000000f},
  {0.793382f,0.000000f,0.000000f},
  {0.777941f,0.000000f,0.000000f},
  {0.762500f,0.000000f,0.000000f},
  {0.747059f,0.000000f,0.000000f},
  {0.731618f,0.000000f,0.000000f},
  {0.716176f,0.000000f,0.000000f},
  {0.700735f,0.000000f,0.000000f},
  {0.685294f,0.000000f,0.000000f},
  {0.669853f,0.000000f,0.000000f},
  {0.654412f,0.000000f,0.000000f},
  {0.638971f,0.000000f,0.000000f},
  {0.623529f,0.000000f,0.000000f},
  {0.608088f,0.000000f,0.000000f},
  {0.592647f,0.000000f,0.000000f},
  {0.577206f,0.000000f,0.000000f},
  {0.561765f,0.000000f,0.000000f},
  {0.546324f,0.000000f,0.000000f},
  {0.530882f,0.000000f,0.000000f},
  {0.515441f,0.000000f,0.000000f},
  {0.500000f,0.000000f,0.000000f},
};


cTeschnerMesh::cTeschnerMesh(cWorld* a_parentWorld) : teschner_mesh_parent_mesh_type(a_parentWorld) {

	m_force_render_tet_mesh = false;
	m_disable_skinned_mesh = false;
	m_compute_vertex_normals = true;
	m_use_cmesh_normal_computation = true;
	m_use_coordinate_frame_based_skinning = true;

	m_initialized = false;
	m_renderFromVBO = false;
	m_rendering_mesh = 0;
	m_vertex_effectors = 0;
	m_vertex_tangential_offsets = 0;
	m_vertex_normal_offsets = 0;
	m_vertex_weights = 0;
	m_weights_per_vertex = 0;
	m_vertex_coordframe_positions = 0;
	m_opposite_edge_vertices = 0;

	m_external_force_scale = 10000.0f;
	m_time_force_exponent = 1.5f;

	m_maximum_vertex_acceleration = -FLT_MAX;
	m_maximum_vertex_velocity = -FLT_MAX;
	m_mean_vertex_acceleration = -FLT_MAX;
	m_mean_vertex_velocity = -FLT_MAX;

	m_render_vertex_constraints = 0;

	m_constraints = 0;

	m_gravity_enabled = true;
	m_floor_force_enabled = true;

	m_nVertices = 0;

	m_current_sim_time = 0;
	GetSystemTime(&m_sim_start_date_and_time);

	m_timestep = DEFAULT_TIMESTEP;

	m_proxy_sim_mesh = 0;

	m_mass_assignment_strategy = DEFAULT_MASS_ASSIGNMENT_STRATEGY;

	if (m_mass_assignment_strategy == FIXED_VERTEX_MASS)
		m_mass_assignment_constant = DEFAULT_PARTICLE_MASS;
	else if (m_mass_assignment_strategy == FIXED_TET_DENSITY)
		m_mass_assignment_constant = DEFAULT_TET_DENSITY;
	else if (m_mass_assignment_strategy == FIXED_TOTAL_MASS)
		m_mass_assignment_constant = DEFAULT_TOTAL_MASS;

	// Set default spring constants
	if (GRAVITY_IS_ACCELERATION)
		m_gravity_force = DEFAULT_GRAVITY_ACCELERATION;
	else
		m_gravity_force = DEFAULT_GRAVITY_FORCE;

	m_kVolumePreservation = DEFAULT_K_VOLUME;
	m_kAreaPreservation = DEFAULT_K_AREA;
	m_kDistancePreservation = DEFAULT_K_DISTANCE;
	m_kDistanceDamping = DEFAULT_DISTANCE_DAMPING;
	m_kFloor = DEFAULT_K_FLOOR;
	m_floor_position = DEFAULT_FLOOR_POSITION;

	// Zero out array variables
	m_faceNormalsAndAreas = 0;
	m_edgeLengths = m_tetVolumes = 0;

	m_deformableVertices = m_previousPositions = 0;

	m_restFaceNormalsAndAreas = 0;
	m_restEdgeLengths = m_restTetVolumes = 0;

	m_faces = 0;
	m_edges = 0;
	m_nFaces = m_nEdges = 0;

	m_opposingFaces = 0;

	m_containingTets = m_containingFaces = m_containingEdges = 0;

	m_heterogeneous_constants = false;

	m_hetero_kVolumePreservation = 0;
	m_hetero_kAreaPreservation = 0;
	m_hetero_kDistancePreservation = 0;
	m_hetero_kDistanceDamping = 0;

	m_heterogeneous_constant_flags = 0;

}


cTeschnerMesh::~cTeschnerMesh() {

	// Delete lots of arrays
	SAFE_ARRAY_DELETE(m_faceNormalsAndAreas);
	SAFE_ARRAY_DELETE(m_edgeLengths);
	SAFE_ARRAY_DELETE(m_tetVolumes);
	SAFE_ARRAY_DELETE(m_deformableVertices);
	SAFE_ARRAY_DELETE(m_previousPositions);
	SAFE_ARRAY_DELETE(m_restEdgeLengths);
	SAFE_ARRAY_DELETE(m_restFaceNormalsAndAreas);
	SAFE_ARRAY_DELETE(m_restTetVolumes);
	SAFE_ARRAY_DELETE(m_faces);
	SAFE_ARRAY_DELETE(m_edges);
	SAFE_ARRAY_DELETE(m_opposingFaces);
	SAFE_ARRAY_DELETE(m_containingTets);
	SAFE_ARRAY_DELETE(m_containingFaces);
	SAFE_ARRAY_DELETE(m_containingEdges);

	SAFE_ARRAY_DELETE(m_hetero_kVolumePreservation);
	SAFE_ARRAY_DELETE(m_hetero_kAreaPreservation);
	SAFE_ARRAY_DELETE(m_hetero_kDistancePreservation);
	SAFE_ARRAY_DELETE(m_hetero_kDistanceDamping);

	SAFE_ARRAY_DELETE(m_vertex_effectors);
	SAFE_ARRAY_DELETE(m_vertex_weights);
	SAFE_ARRAY_DELETE(m_vertex_coordframe_positions);
	SAFE_ARRAY_DELETE(m_opposite_edge_vertices);

	m_nFaces = m_nEdges = 0;

	clear_solid_sections();

	// if (m_rendering_mesh) delete m_rendering_mesh;

	// Doesn't delete constraints...

}

void cTeschnerMesh::clear_solid_sections() {

	std::list<solid_section*>::iterator iter;
	iter = solid_sections.begin();
	while (iter != solid_sections.end()) {
		delete (*iter);
		iter++;
	}
	solid_sections.clear();

}

bool cTeschnerMesh::tick() {

	m_current_sim_time += m_timestep;

	// _cprintf("Sim time is %f\n",m_current_sim_time);

	// Add or remove constraints
	if (m_constraints) {

		constraint* c;

		while (1) {
			c = m_constraints->get_next_constraint_start(m_current_sim_time);
			if (c == 0) break;

			// _cprintf("Applying a constraint of type %d at time %f\n",c->type,m_current_sim_time);

			if (c->type == CONSTRAINT_FORCE_CONSTANT) {
				constraint_force_constant* fc = (constraint_force_constant*)c;

				unsigned int nVertices = fc->affects_all_vertices ? m_nVertices : fc->vertices.size();
				for (unsigned int i = 0; i < nVertices; i++) {
					unsigned int index = fc->affects_all_vertices ? i : fc->vertices[i];
					if (index < 0 || index > m_nVertices) {
						_cprintf("Illegal vertex %d (of %d)\n", i, m_nVertices);
						continue;
					}
					cDeformableVertex* v = m_deformableVertices + index;
					v->force_is_constrained++;
					v->constraint_force += fc->force;
					/*
					_cprintf("Applying force %s to vertex %d (%d,%s)\n",
					   fc->force.str().c_str(),
					  index,
					  (int)v->force_is_constrained,
					  v->constraint_force.str().c_str());
					*/

				} // for each constrained vertex


			} // constant force constraint

			else if (c->type == CONSTRAINT_POSITION_CONSTANT) {
				constraint_position_constant* pc = (constraint_position_constant*)c;


				unsigned int nVertices = pc->affects_all_vertices ? m_nVertices : pc->vertices.size();
				for (unsigned int i = 0; i < nVertices; i++) {
					unsigned int index = pc->affects_all_vertices ? i : pc->vertices[i];
					if (index < 0 || index > m_nVertices) {
						_cprintf("Illegal vertex %d (of %d)\n", i, m_nVertices);
						continue;
					}
					cDeformableVertex* v = m_deformableVertices + index;
					cVector3f pos = (pc->lock) ? v->m_pos : pc->position;
					v->position_is_constrained++;
					// _cprintf("Applying position constraint %s to vertex %d (%d)\n",pos.str().c_str(),index,(int)(v->position_is_constrained));
					v->m_pos = pos;
				} // for each constrained vertex        

			}

			else {
				_cprintf("Unknown constraint type %d\n", c->type);
			}

		}

		while (1) {
			c = m_constraints->get_next_constraint_end(m_current_sim_time);
			if (c == 0) break;

			else _cprintf("Ending constraint at time %f...\n", m_current_sim_time);

			if (c->type == CONSTRAINT_FORCE_CONSTANT) {
				constraint_force_constant* fc = (constraint_force_constant*)c;

				unsigned int nVertices = fc->affects_all_vertices ? m_nVertices : fc->vertices.size();
				for (unsigned int i = 0; i < nVertices; i++) {
					unsigned int index = fc->affects_all_vertices ? i : fc->vertices[i];
					if (index < 0 || index > m_nVertices) {
						_cprintf("Illegal vertex %d (of %d)\n", i, m_nVertices);
						continue;
					}
					cDeformableVertex* v = m_deformableVertices + index;
					_cprintf("Releasing force %s from vertex %d\n", fc->force.str().c_str(), index);
					v->force_is_constrained--;
					v->constraint_force -= fc->force;
				} // for each constrained vertex        
			}

			else if (c->type == CONSTRAINT_POSITION_CONSTANT) {
				constraint_position_constant* pc = (constraint_position_constant*)c;

				unsigned int nVertices = pc->affects_all_vertices ? m_nVertices : pc->vertices.size();
				for (unsigned int i = 0; i < nVertices; i++) {
					unsigned int index = pc->affects_all_vertices ? i : pc->vertices[i];
					if (index < 0 || index > m_nVertices) {
						_cprintf("Illegal vertex %d (of %d)\n", i, m_nVertices);
						continue;
					}
					cDeformableVertex* v = m_deformableVertices + index;
					v->position_is_constrained--;
					_cprintf("Released vertex %d (%d)\n", index, (int)(v->position_is_constrained));
				} // for each constrained vertex

			}

			else {
				_cprintf("Unknown constraint type %d\n", c->type);
			}

		} // ending constraints

	} // if we have constraints
	if (m_nVertices == 0) {
		_cprintf("Deformable mesh not initialized...\n");
		return false;
	}

	// TODO: should descend through children here
	if (m_proxy_sim_mesh) {
		return m_proxy_sim_mesh->tick();
	}

	if (!compute_face_areas()) return false;
	if (!compute_tet_volumes()) return false;
	if (!compute_spring_lengths()) return false;
	if (!compute_forces()) return false;
	if (!move_vertices()) return false;

	return true;
}


void cTeschnerMesh::initialize_vertex_masses() {

	float total_volume = 0;
	for (unsigned int i = 0; i < m_nTets; i++) {
#ifdef HANDLE_STUPID_INTEL_PROBLEM
		_cprintf("Initializing mass %d of %d\n", i, m_nTets);
#endif
		total_volume += (fabs(m_restTetVolumes[i]));
	}

	_cprintf("Total volume is %f\n", total_volume);

	float density;

	float total_mass = 0.0;
	float minmass = 10000000.0;
	float maxmass = 0.0;

	if (m_mass_assignment_strategy == FIXED_TET_DENSITY) {
		density = m_mass_assignment_constant;
	}
	else if (m_mass_assignment_strategy == FIXED_TOTAL_MASS) {
		density = m_mass_assignment_constant / total_volume;
	}

	// Distribute density or assign constant mass to each vertex
	for (unsigned int i = 0; i < m_nVertices; i++) {
		cDeformableVertex* v = m_deformableVertices + i;

		if (m_mass_assignment_strategy == FIXED_VERTEX_MASS) {
			v->m_mass = m_mass_assignment_constant;
			continue;
		}

		v->m_mass = 0;

		// For each tet I live in
		const unsigned int n_local_tets = v->m_nTets;
		unsigned int* curtet_index_ptr = m_containingTets + v->m_startingTetIndex;
		int* cur_opposing_face = m_opposingFaces + v->m_startingTetIndex;

		for (register unsigned int j = 0; j < n_local_tets; j++, curtet_index_ptr++, cur_opposing_face++) {

			int containing_tet_index = *curtet_index_ptr;

			// Find the rest volume for this tet
			float rest_volume = fabs(m_restTetVolumes[containing_tet_index]);

			// Take my share of this tet's mass
			float local_mass = rest_volume * density / 4.0f;

			v->m_mass += local_mass;

			total_mass += local_mass;
			if (v->m_mass > maxmass) maxmass = v->m_mass;
			if (v->m_mass < minmass) minmass = v->m_mass;
		}
	}

	if (m_mass_assignment_strategy != FIXED_VERTEX_MASS)
		_cprintf("Total mass: %f, min mass: %f, max mass: %f\n", total_mass, minmass, maxmass);

}


void cTeschnerMesh::prepare_to_render() {

	if (m_nVertices == 0) {
		_cprintf("Deformable mesh not initialized...\n");
		return;
	}

	// TODO: should descend through children here
	if (m_proxy_sim_mesh) {
		m_proxy_sim_mesh->prepare_to_render();
		return;
	}

	int control_pressed = GetAsyncKeyState(VK_CONTROL) & (1 << 15);

	if (m_compute_vertex_normals && (m_use_cmesh_normal_computation == (int)(true))) {
		this->computeAllNormals();
	}

	if (m_rendering_mesh && (m_disable_skinned_mesh == false)) {

		// std::vector<cVertex>* rendering_vertex_vector = m_rendering_mesh->pVerticesNonEmpty();
		std::vector<cVertex>* rendering_vertex_vector = m_rendering_mesh->pVertices();
		cVertex* vertex_array = (cVertex*) &((*rendering_vertex_vector)[0]);
		unsigned int n_rendering_vertices = rendering_vertex_vector->size();
		cVertex* cur = vertex_array;
		cVertex* end = vertex_array + n_rendering_vertices;

		int weight_offset = 0;

		// _cprintf("Rendering from array %x (%d)\n",vertex_array,n_rendering_vertices);

		unsigned int vcount = 0;

		// For each of my rendering mesh's vertices
		while (cur != end) {

			vcount++;
			cur->m_localPos.zero();

			// For each relevant effector vertex
			for (unsigned int j = 0; j < m_weights_per_vertex; j++) {

				// This will be the "vote" from this weight source
				cVector3f temppos; // (0,0,0);

				// Find the effector point
				int effector_index = m_vertex_effectors[weight_offset];

				// A -1 value says "no more effectors"
				if (effector_index < 0) break;
				cDeformableVertex* cdv = m_deformableVertices + effector_index;
				temppos = cdv->m_pos;

				if (m_use_coordinate_frame_based_skinning) {

					// One axis is the vertex normal
					cVector3f axis1 = cdv->m_normal;

					// The next will be the first triangle axis, made perpendicular to the normal
					unsigned int other_pt_idx = m_opposite_edge_vertices[weight_offset];
					cVector3f other_pt_pos = m_deformableVertices[other_pt_idx].m_pos;
					cVector3f edge = other_pt_pos - cdv->m_pos;

					// Component parallel to my normal
					cVector3f axis2_parallel = (edge*axis1) * axis1;

					// Component perpendicular to my normal
					cVector3f axis2_perpendicular = edge - axis2_parallel;

					// The normalized perpendicular is my second axis
					cVector3f axis2 = axis2_perpendicular;
					axis2.normalize();

					// The third axis will be their cross-product
					cVector3f axis3 = axis1;
					axis3.cross(axis2);

					cVector3f offset_in_vertex_space = m_vertex_coordframe_positions[weight_offset];

					cVector3f offset_in_world_space =
						axis1 * offset_in_vertex_space.x +
						axis2 * offset_in_vertex_space.y +
						axis3 * offset_in_vertex_space.z;

					temppos += offset_in_world_space;

#ifdef DEBUG_VERTEX_INDEX
					if (cur == vertex_array + DEBUG_VERTEX_INDEX) {
						int i = cur - vertex_array;
						cVector3f origin = cdv->m_pos;
						_cprintf("Handling vertex %d,%d (%d) (wo %d): origin %s, otherpos %s, axis1 %s, axis2 %s, axis3 %s, vspace %s, wspace %s\n",
							i, weight_offset, g_debug_vertex_index, weight_offset,
							origin.str(2).c_str(),
							other_pt_pos.str(2).c_str(),
							axis1.str(2).c_str(), axis2.str(2).c_str(), axis3.str(2).c_str(),
							offset_in_vertex_space.str(2).c_str(),
							offset_in_world_space.str(2).c_str());
						// _getch();
					}
#endif
				}

				else {

					// Add his current position to the stored offset times his normal + plus the
					// tangential offset
					temppos.add(m_vertex_normal_offsets[weight_offset] * (cdv->m_normal));
					temppos.add(m_vertex_tangential_offsets[weight_offset]);
				}

				// Multiply by the stored weight and store this vertex's new position
				temppos.mul(m_vertex_weights[weight_offset]);
				cur->m_localPos += cVector3d(temppos.x, temppos.y, temppos.z);
				weight_offset++;

			} // for each effector

			cur++;

		} // for each vertex in my rendering mesh
	} // if I have a rendering mesh

	// I really want to do this just when I'm rendering the boxes,
	// but right now I need to copy normals from my cMesh self
	// to my cTeschnerMesh self.

	// if ((m_rendering_mesh == 0) || control_pressed) {

	// I can avoid this whole loop-and-copy if I just copy the cmesh
	// normal computation to this class and operate directly on 
	// cDeformableVertex's

	if (1) {

		// Copy current position/normal information to parent mesh
		std::vector<cVertex>* vertex_vector = this->pVertices();
		cVertex* vertex_array = (cVertex*) &((*vertex_vector)[0]);
		m_nVertices = vertex_vector->size();

		// For each vertex
		for (unsigned int i = 0; i < m_nVertices; i++) {

			cDeformableVertex* dv = &(m_deformableVertices[i]);
			// Copy vertex position to the rendering array
			vertex_array[i].setPos(
				dv->m_pos.x,
				dv->m_pos.y,
				dv->m_pos.z);

			if (m_compute_vertex_normals) {
				if (m_use_cmesh_normal_computation) {
					// Copy vertex normal from the rendering array
					dv->m_normal.set(
						vertex_array[i].m_normal.x,
						vertex_array[i].m_normal.y,
						vertex_array[i].m_normal.z);
				}
				else {
					// Copy vertex normal to the rendering array
					vertex_array[i].setNormal(
						dv->m_normal.x,
						dv->m_normal.y,
						dv->m_normal.z);
				}
			}
		}
	}
}


inline void sort_face(face& f1) {
	register unsigned int tmp;
	if (f1.v0 > f1.v1) { tmp = f1.v1; f1.v1 = f1.v0; f1.v0 = tmp; }
	if (f1.v1 > f1.v2) { tmp = f1.v2; f1.v2 = f1.v1; f1.v1 = tmp; }
	else return;
	if (f1.v0 > f1.v1) { tmp = f1.v1; f1.v1 = f1.v0; f1.v0 = tmp; }
}


inline void sort_edge(edge& e1) {
	register unsigned int tmp;
	if (e1.v0 > e1.v1) { tmp = e1.v1; e1.v1 = e1.v0; e1.v0 = tmp; }
}


inline bool isDegenerateEdge(const edge& e1) {
	return (e1.v0 == e1.v1);
}

inline bool isDegenerateTet(const unsigned int* t, cVertex* vertex_array) {
	bool trivially_degenerate(
		t[0] == t[1] ||
		t[0] == t[2] ||
		t[0] == t[3] ||
		t[1] == t[2] ||
		t[1] == t[3] ||
		t[2] == t[3]
	);
	if (trivially_degenerate) return true;

	// Test faces for collinearity
	for (unsigned int curface = 0; curface < 4; curface++) {
		face f(
			t[(tet_triangle_faces[curface][0])],
			t[(tet_triangle_faces[curface][1])],
			t[(tet_triangle_faces[curface][2])]
		);
		cVector3f v0 = vertex_array[f.v0].getPos();
		cVector3f v1 = vertex_array[f.v1].getPos();
		cVector3f v2 = vertex_array[f.v2].getPos();

		cVector3f d0 = v1 - v0;
		cVector3f d1 = v2 - v0;
		d0.normalize();
		d1.normalize();

		double d = d0*d1;

		// Are these vectors close to parallel?
		if (fabs(d) > PARALLEL_LINE_THRESHOLD) {
			_cprintf("Degenerate face, dot is %lf...\n", d);
			return true;
		}
	}

	return false;

	// Test the tet for minimum volume?
}


struct ltface {
	inline bool operator()(const face& f1, const face& f2) const
	{
		if (f1.v0 < f2.v0) return true;
		else if (f1.v0 > f2.v0) return false;
		else if (f1.v1 < f2.v1) return true;
		else if (f1.v1 > f2.v1) return false;
		else if (f1.v2 < f2.v2) return true;
		return false;
	}
};


struct ltedge {
	inline bool operator()(const edge& e1, const edge& e2) const
	{
		if (e1.v0 < e2.v0) return true;
		else if (e1.v0 > e2.v0) return false;
		else if (e1.v1 < e2.v1) return true;
		return false;
	}
};


int cTeschnerMesh::clear_constraints() {

	if (m_initialized == false) return -1;

	if (m_constraints) {
		m_constraints->clear();
	}

	return 0;
}

int cTeschnerMesh::reset() {

	if (m_initialized == false) return -1;
	m_current_sim_time = 0.0f;
	GetSystemTime(&m_sim_start_date_and_time);

	m_localPos.set(0, 0, 0);
	m_localRot.identity();

	m_maximum_vertex_acceleration = -FLT_MAX;
	m_maximum_vertex_velocity = -FLT_MAX;
	m_mean_vertex_acceleration = -FLT_MAX;
	m_mean_vertex_velocity = -FLT_MAX;

	// Restore original data
	memcpy(m_faceNormalsAndAreas, m_bkup_faceNormalsAndAreas, m_nFaces * sizeof(cVector3f));
	memcpy(m_edgeLengths, m_bkup_edgeLengths, m_nEdges * sizeof(float));
	memcpy(m_tetVolumes, m_bkup_tetVolumes, m_nTets * sizeof(float));

	memcpy(m_deformableVertices, m_bkup_deformableVertices, m_nVertices * sizeof(cDeformableVertex));
	memcpy(m_previousPositions, m_bkup_deformableVertices, m_nVertices * sizeof(cDeformableVertex));

	if (m_constraints) {
		m_constraints->reset();
	}

	return 0;
}


void cTeschnerMesh::process_pending_constant_assignments() {

	if (m_pending_constant_assignments.size() > 0 && m_heterogeneous_constants == false) {
		initialize_nonhomogeneity();
	}

	if (!m_hetero_kVolumePreservation ||
		!m_hetero_kAreaPreservation ||
		!m_hetero_kDistancePreservation ||
		!m_hetero_kDistanceDamping) {
		_cprintf("Initializing homogeneity...\n");
		initialize_nonhomogeneity();
	}

	// For each constant assignment chunk
	// _cprintf("Setting %d nh values\n",m_pending_constant_assignments.size());

	std::list<pending_constant_assignment>::iterator iter;
	for (iter = m_pending_constant_assignments.begin();
		iter != m_pending_constant_assignments.end();
		iter++) {
		pending_constant_assignment cur = *iter;
		float* array;
		if (cur.constant_name == KVOLUME) {
			array = m_hetero_kVolumePreservation;
			m_heterogeneous_constant_flags |= (1 << KVOLUME);
		}
		else if (cur.constant_name == KAREA) {
			array = m_hetero_kAreaPreservation;
			m_heterogeneous_constant_flags |= (1 << KAREA);
		}
		else if (cur.constant_name == KDISTANCE) {
			array = m_hetero_kDistancePreservation;
			m_heterogeneous_constant_flags |= (1 << KDISTANCE);
		}
		else if (cur.constant_name == KDAMPING) {
			array = m_hetero_kDistanceDamping;
			m_heterogeneous_constant_flags |= (1 << KDAMPING);
		}
		else {
			_cprintf("Unrecognized constant name... %d\n", cur.constant_name);
			continue;
		}
		// For each vertex referenced
		std::list<unsigned int>::iterator viter;
		// _cprintf("Processing %d vertices...\n",cur.vertices.size());
		for (viter = cur.vertices.begin(); viter != cur.vertices.end(); viter++) {
			unsigned int curindex = *viter;
			if (curindex > m_nVertices) {
				_cprintf("Illegal constant assignment to vertex %d\n", curindex);
				continue;
			}
			array[curindex] = (float)cur.value;
			//_cprintf("Assigning value %f to constant %d for vertex %d\n",
			//  array[curindex],cur.constant_name,curindex);
		}
	}

	m_pending_constant_assignments.clear();

} // process_pending_constant_assignments


void cTeschnerMesh::initialize_nonhomogeneity() {

	_cprintf("Re-initializing nonhomogeneity...\n");

	m_heterogeneous_constants = true;

	SAFE_ARRAY_DELETE(m_hetero_kVolumePreservation);
	SAFE_ARRAY_DELETE(m_hetero_kAreaPreservation);
	SAFE_ARRAY_DELETE(m_hetero_kDistancePreservation);
	SAFE_ARRAY_DELETE(m_hetero_kDistanceDamping);

	m_hetero_kVolumePreservation = new float[m_nVertices];
	m_hetero_kAreaPreservation = new float[m_nVertices];
	m_hetero_kDistancePreservation = new float[m_nVertices];
	m_hetero_kDistanceDamping = new float[m_nVertices];
	for (unsigned int i = 0; i < m_nVertices; i++) {
		m_hetero_kVolumePreservation[i] = m_kVolumePreservation;
		m_hetero_kAreaPreservation[i] = m_kAreaPreservation;
		m_hetero_kDistancePreservation[i] = m_kDistancePreservation;
		m_hetero_kDistanceDamping[i] = m_kDistanceDamping;
	}

}

int cTeschnerMesh::initialize(cTeschnerMesh* old_model) {

	_cprintf("Initializing deformable mesh...\n");
	// _getch();

	if (old_model) {
		void* dst_begin = (void*)(&(this->BEGIN_CONSTANTS));
		void* src_begin = (void*)(&(old_model->BEGIN_CONSTANTS));
		void* dst_end = (void*)(&(this->END_CONSTANTS));
		unsigned int length = (char*)dst_end - (char*)dst_begin;
		memcpy(dst_begin, src_begin, length);
		if (m_constraints) m_constraints->reset();
	}

	m_current_sim_time = 0.0f;
	GetSystemTime(&m_sim_start_date_and_time);

	// TODO: I should probably descend properly through my children here
	if (m_nTets == 0) {
		if (getNumChildren() == 0) {
			_cprintf("Oops... I have no tets and no children...\n");
			return -1;
		}
		cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(getChild(0));
		if (ctm == 0) {
			_cprintf("Oops... my child is not a teschner mesh...\n");
			return -1;
		}
		_cprintf("Deferring operation to my child...\n");
		m_proxy_sim_mesh = ctm;
		return ctm->initialize();
	}

	m_proxy_sim_mesh = 0;

	// Create connectivity (face and edge) data from tet data
	set<face, ltface> faces;
	set<edge, ltedge> edges;

	// Insert a degenerate face to avoid the problem with face zero
	face degenerate_face(0, 0, 0);
	faces.insert(degenerate_face);

	unsigned int i;

	// Get access to my vertex array
	std::vector<cVertex>* vertex_vector = this->pVertices();
	cVertex* vertex_array = (cVertex*) &((*vertex_vector)[0]);
	m_nVertices = vertex_vector->size();

	// Test for degenerate tets first and adjust the
	// tet array accordingly
	std::list<unsigned int> good_tets;
	for (i = 0; i < m_nTets; i++) {

		// Grab the indices for this tet
		unsigned int* tet_indices = m_tets + i * 4;

		if (isDegenerateTet(tet_indices, vertex_array)) {
			_cprintf("Warning: degenerate tet %d (%d,%d,%d,%d)\n", i,
				tet_indices[0], tet_indices[1], tet_indices[2], tet_indices[3]);
			continue;
		}
		good_tets.push_back(i);
	}
	_cprintf("Pruned from %d tets to %d tets\n", m_nTets, good_tets.size());
	m_nTets = good_tets.size();
	unsigned int cur_good_tet_slot = 0;
	std::list<unsigned int>::iterator good_tet_iter = good_tets.begin();
	while (good_tet_iter != good_tets.end()) {
		int old_tet_index = *good_tet_iter;
		unsigned int tet[4];
		// Grab the old tet
		memcpy(tet, m_tets + old_tet_index * 4, 4 * sizeof(unsigned int));
		// Put it in the new slot
		memcpy(m_tets + cur_good_tet_slot * 4, tet, 4 * sizeof(unsigned int));

		/*
		_cprintf("Moving good tet %d (%d,%d,%d,%d) to %d\n",
		  old_tet_index,
		  tet[0],tet[1],tet[2],tet[3],
		  cur_good_tet_slot);
		  */

		good_tet_iter++;
		cur_good_tet_slot++;

	}

	for (i = 0; i < m_nTets; i++) {

		// Grab the indices for this tet
		unsigned int* tet_indices = m_tets + i * 4;

		if (isDegenerateTet(tet_indices, vertex_array)) {
			_cprintf("Warning: degenerate tet %d (%d,%d,%d,%d) shouldn't be here any more\n",
				i, tet_indices[0], tet_indices[1], tet_indices[2], tet_indices[3]);
			continue;
		}

		// Create and insert four _sorted_ faces
		for (unsigned int curface = 0; curface < 4; curface++) {
			face f(
				tet_indices[(tet_triangle_faces[curface][0])],
				tet_indices[(tet_triangle_faces[curface][1])],
				tet_indices[(tet_triangle_faces[curface][2])]
			);
			sort_face(f);
			faces.insert(f);
		}

		// Create and insert six _sorted_ edges
		edge local_edges[6];
		local_edges[0] = edge(tet_indices[0], tet_indices[1]);
		local_edges[1] = edge(tet_indices[0], tet_indices[2]);
		local_edges[2] = edge(tet_indices[0], tet_indices[3]);
		local_edges[3] = edge(tet_indices[1], tet_indices[2]);
		local_edges[4] = edge(tet_indices[1], tet_indices[3]);
		local_edges[5] = edge(tet_indices[2], tet_indices[3]);

		for (unsigned int curedge = 0; curedge < 6; curedge++) {
			sort_edge(local_edges[curedge]);
			edges.insert(local_edges[curedge]);
		}

	} // For each tet

	m_nFaces = faces.size();
	m_nEdges = edges.size();
	m_faces = new face[m_nFaces];
	m_edges = new edge[m_nEdges];

	// A map just to map triangles to indices
	std::map<face, unsigned int, ltface> face_int_map;
	std::map<face, unsigned int, ltface>::iterator face_int_map_iter;

	i = 0;
	set<face, ltface>::iterator faceiter;
	for (faceiter = faces.begin(); faceiter != faces.end(); faceiter++, i++) {
		m_faces[i] = *faceiter;
		face_int_map[*faceiter] = i;
	}

	i = 0;
	set<edge, ltedge>::iterator edgeiter;
	for (edgeiter = edges.begin(); edgeiter != edges.end(); edgeiter++, i++) {
		m_edges[i] = *edgeiter;
	}

	// Create per-pass arrays
	m_faceNormalsAndAreas = new cVector3f[m_nFaces];
	m_edgeLengths = new float[m_nEdges];
	m_tetVolumes = new float[m_nTets];

	// Create maps from vertices back to connectivity maps
	temp_vertex_info* tmpv = new temp_vertex_info[m_nVertices];

	// For each tet
	for (unsigned int i = 0; i < m_nTets; i++) {

		// For each vertex
		for (int j = 0; j < 4; j++) {

			unsigned int* curtet = m_tets + i * 4;
			unsigned int cur_vertex_index = curtet[j];

			if (cur_vertex_index == 0) {
				int k = 2;
			}

			// Add this tet to the containing tets list for this vertex
			tmpv[cur_vertex_index].containing_tets.push_back(i);

			// Where does this tet appear in this vertex's tet list?
			unsigned int local_tet_index = tmpv[cur_vertex_index].containing_tets.size() - 1;

			// Build the triangle in this tet that _doesn't_ include this
			// vertex...
			face f;
			unsigned int vertices_added = 0;
			for (int k = 0; k < 4; k++) {
				if (k == j) continue;
				f[vertices_added] = curtet[k];
				vertices_added++;
			}
			sort_face(f);

			face_int_map_iter = face_int_map.find(f);
			if (face_int_map_iter == face_int_map.end()) {
				_cprintf("Warning: I couldn't find this face in the face map...\n");
				continue;
			}

			unsigned int face_index = (*face_int_map_iter).second;

			// Put him in the opposing-faces list in the _same place_ that
			// this tet appears in the containing-tets list

			/*
			// Re-allocate if necessary
			if (tmpv[cur_vertex_index].opposing_faces.capacity() < (local_tet_index+1))
			  tmpv[cur_vertex_index].opposing_faces.reserve(local_tet_index+1);

			tmpv[cur_vertex_index].opposing_faces[local_tet_index] = face_index;
			*/
			tmpv[cur_vertex_index].opposing_faces.push_back(face_index);

			// We'll determine the _sign_ for the opposing faces later, after
			// we've computed face normals
		}
	}

	// For each face
	// 
	// (skip the zero face, since it's invalid)
	for (unsigned int i = 1; i < m_nFaces; i++) {

		// For each vertex
		for (unsigned int j = 0; j < 3; j++) {

			// Which vertex is this?
			unsigned int cur_vertex_index = ((unsigned int*)(m_faces))[3 * i + j];

			// Add this face to the containing faces list for this vertex
			tmpv[cur_vertex_index].containing_faces.push_back(i);
		}
	}

	// For each edge
	for (unsigned int i = 0; i < m_nEdges; i++) {

		// For each vertex
		for (int j = 0; j < 2; j++) {

			unsigned int cur_vertex_index = ((unsigned int*)(m_edges))[2 * i + j];
			// Add this edge to the containing edges list for this vertex
			tmpv[cur_vertex_index].containing_edges.push_back(i);
		}
	}

	// Create and fill the connectivity data structures
	unsigned int total_containing_tets = 0;
	unsigned int total_containing_edges = 0;
	unsigned int total_containing_faces = 0;

	for (unsigned int i = 0; i < m_nVertices; i++) {
		temp_vertex_info* v = tmpv + i;
		total_containing_tets += v->containing_tets.size();
		total_containing_edges += v->containing_edges.size();
		total_containing_faces += v->containing_faces.size();
	}

	m_containingTets = new unsigned int[total_containing_tets];
	m_opposingFaces = new int[total_containing_tets];
	m_containingFaces = new unsigned int[total_containing_faces];
	m_containingEdges = new unsigned int[total_containing_edges];

	// Fill the connectivity data structures
	unsigned int current_tet_slot = 0;
	unsigned int current_face_slot = 0;
	unsigned int current_edge_slot = 0;

	// Create the vertex data structures
	m_deformableVertices = new cDeformableVertex[m_nVertices];
	memset(m_deformableVertices, 0, m_nVertices * sizeof(cDeformableVertex));

	// Set up vertex positions first
	for (unsigned int i = 0; i < m_nVertices; i++) {
		cVertex* v = vertex_array + i;
		cDeformableVertex* dv = m_deformableVertices + i;
		dv->m_pos = v->getPos();
	}

	// Now compute initial face areas and tet volumes, which depend only
	// on vertex positions
	compute_tet_volumes();
	compute_spring_lengths();
	compute_face_areas();

	// Now the more complex vertex data structures...

	// For each vertex
	for (unsigned int i = 0; i < m_nVertices; i++) {
		cVertex* v = vertex_array + i;
		cDeformableVertex* dv = m_deformableVertices + i;
		temp_vertex_info* tvi = tmpv + i;

		dv->m_pos = v->getPos();

		// Copy normals from the (initialized) rendering mesh to the simulation mesh
		// if (m_compute_vertex_normals)
		dv->m_normal = v->getNormal();

		// Normal and force are still unspecified...

		dv->m_nTets = tvi->containing_tets.size();
		dv->m_nFaces = tvi->containing_faces.size();
		dv->m_nEdges = tvi->containing_edges.size();

		dv->m_startingTetIndex = current_tet_slot;
		dv->m_startingFaceIndex = current_face_slot;
		dv->m_startingEdgeIndex = current_edge_slot;

		// For each tet that I live in
		for (unsigned int j = 0; j < dv->m_nTets; j++, current_tet_slot++) {

			m_containingTets[current_tet_slot] = tvi->containing_tets[j];

			// Determine sign for the opposing face
			int face_index = tvi->opposing_faces[j];

			// The current vertex position
			cVertex* v = vertex_array + i;
			cVector3f vertex_pos = v->m_localPos;

			face opposing_face = m_faces[face_index];
			cVector3f opposing_face_vertices[3];

			// The center of this face
			cVector3f face_center(0, 0, 0);

			for (unsigned int k = 0; k < 3; k++) {
				cVertex* v = vertex_array + opposing_face[k];
				opposing_face_vertices[k] = v->m_localPos;
				face_center += opposing_face_vertices[k];
			}
			face_center /= 3.0;

			// A vector from the face center to this vertex
			cVector3f face_center_to_vertex = vertex_pos - face_center;
			face_center_to_vertex.normalize();

			// The triangle's (non-normalized) surface normal
			cVector3f face_normal = m_faceNormalsAndAreas[face_index];
			face_normal.normalize();

			// Does this face point toward me?
			bool points_toward_me = ((face_normal * face_center_to_vertex) > 0.0f);

			int sign = points_toward_me ? 1 : -1;

			/*
			_cprintf("Face %d (%f,%f,%f) points %s v %d (%f,%f,%f)\n",
			  face_index,face_normal.x,face_normal.y,face_normal.z,
			  points_toward_me?"toward":"away from",i,
			  vertex_pos.x,vertex_pos.y,vertex_pos.z);
			  */

			  // Set the sign of this face index appropriately
			tvi->opposing_faces[j] *= sign;

			// Store the _signed_ face index
			m_opposingFaces[current_tet_slot] = tvi->opposing_faces[j];
		}

		// For each face that I live in
		for (unsigned int j = 0; j < dv->m_nFaces; j++) {
			m_containingFaces[current_face_slot++] = tvi->containing_faces[j];
		}

		// For each edge that I live in
		for (unsigned int j = 0; j < dv->m_nEdges; j++) {
			m_containingEdges[current_edge_slot++] = tvi->containing_edges[j];
		}

	} // For each vertex   

	delete[] tmpv;

	_cprintf("Creating rest-state structures...\n");
	// _getch();

	// Create and initialize rest-state structures
	m_restFaceNormalsAndAreas = new cVector3f[m_nFaces];
	m_restEdgeLengths = new float[m_nEdges];
	m_restTetVolumes = new float[m_nTets];

	memcpy(m_restFaceNormalsAndAreas, m_faceNormalsAndAreas, m_nFaces * sizeof(cVector3f));
	memcpy(m_restEdgeLengths, m_edgeLengths, m_nEdges * sizeof(float));
	memcpy(m_restTetVolumes, m_tetVolumes, m_nTets * sizeof(float));

	// Create history data (same as current data)
	m_previousPositions = new cDeformableVertex[m_nVertices];
	memcpy(m_previousPositions, m_deformableVertices, m_nVertices * sizeof(cDeformableVertex));

	_cprintf("Initializing vertex masses...\n");
	// _getch();

	initialize_vertex_masses();

	_cprintf("Backing up data structures...\n");
	// _getch();

	// Make backups of variable data structures for mesh
	// restoration
	m_bkup_faceNormalsAndAreas = new cVector3f[m_nFaces];
	memcpy(m_bkup_faceNormalsAndAreas, m_faceNormalsAndAreas, m_nFaces * sizeof(cVector3f));

	m_bkup_edgeLengths = new float[m_nEdges];
	memcpy(m_bkup_edgeLengths, m_edgeLengths, m_nEdges * sizeof(float));

	m_bkup_tetVolumes = new float[m_nTets];
	memcpy(m_bkup_tetVolumes, m_tetVolumes, m_nTets * sizeof(float));

	m_bkup_deformableVertices = new cDeformableVertex[m_nVertices];
	memcpy(m_bkup_deformableVertices, m_deformableVertices, m_nVertices * sizeof(cDeformableVertex));

	m_initialized = true;

	// Do we need to initialize heterogeneous constant arrays?
	if (m_heterogeneous_constants) {
		initialize_nonhomogeneity();
		process_pending_constant_assignments();
	}

	// Useful for printing out information about specific vertices
#define PRINT_DEBUG_VERTEX_INFO

#ifdef PRINT_DEBUG_VERTEX_INFO
  // Find the minimum y
	float miny = FLT_MAX;
	float maxy = -FLT_MAX;

	for (int i = 0; i < (int)m_nVertices; i++) {
		cDeformableVertex* v = m_deformableVertices + i;
		if (v->m_pos.y < miny) miny = v->m_pos.y;
		if (v->m_pos.y > maxy) maxy = v->m_pos.y;
	}

	_cprintf("Min y is %f\n", miny);
	_cprintf("Max y is %f\n", maxy);
#endif

	_cprintf("Finished initializing deformable mesh...\n");

	return 0;
}

// #define RENDER_VERTEX_POINTS

void cTeschnerMesh::renderMesh(const int a_renderMode) {

#ifdef RENDER_VERTEX_POINTS

	if (m_nVertices > 0) {
		for (unsigned int i = 0; i < m_nVertices; i++) {
			cDeformableVertex* v = m_deformableVertices + i;
			cDeformableVertex* pv = m_previousPositions + i;
			cVector3f pos = v->m_pos;
			glColor3f(0.1, 1.0, 0.3);
			glPushMatrix();
			glTranslatef(pos.x, pos.y, pos.z);
			cDrawSphere(0.05);
			glPopMatrix();
		}
	}

#endif

#ifdef RENDER_DEBUG_FORCES

	if (m_nVertices > 0) {

		for (unsigned int i = 0; i < m_nVertices; i++) {
			cDeformableVertex* v = m_deformableVertices + i;
			cDeformableVertex* pv = m_previousPositions + i;
			cVector3f pos = v->m_pos;
			cVector3f force;

			// float force_scale_factor = 0.005;
			float force_scale_factor = 0.000005;

			glPointSize(3.0);
			glBegin(GL_POINTS);
			glVertex3f(pos.x, pos.y, pos.z);
			glEnd();

			glLineWidth(3.0);
			glBegin(GL_LINES);

			force = v->m_debug_total_force;
			force *= force_scale_factor;
			glColor3f(1, 1, 1);
			glVertex3f(pos.x, pos.y, pos.z);
			glVertex3f(pos.x + force.x, pos.y + force.y, pos.z + force.z);

			force = v->m_debug_edge_force*force_scale_factor;
			glColor3f(1, 0, 0);
			glVertex3f(pos.x, pos.y, pos.z);
			glVertex3f(pos.x + force.x, pos.y + force.y, pos.z + force.z);

			force = v->m_debug_volume_force*force_scale_factor;
			glColor3f(0, 1, 0);
			glVertex3f(pos.x, pos.y, pos.z);
			glVertex3f(pos.x + force.x, pos.y + force.y, pos.z + force.z);

			force = v->m_debug_area_force*force_scale_factor;
			glColor3f(0, 0, 1);
			glVertex3f(pos.x, pos.y, pos.z);
			glVertex3f(pos.x + force.x, pos.y + force.y, pos.z + force.z);

			force = v->m_debug_damping_force*force_scale_factor;
			glColor3f(1, 1, 0);
			glVertex3f(pos.x, pos.y, pos.z);
			glVertex3f(pos.x + force.x, pos.y + force.y, pos.z + force.z);

			force = v->m_debug_floor_force*force_scale_factor;
			glColor3f(1, 0, 1);
			glVertex3f(pos.x, pos.y, pos.z);
			glVertex3f(pos.x + force.x, pos.y + force.y, pos.z + force.z);

			force = v->m_debug_gravity_force*force_scale_factor;
			glColor3f(0, 1, 1);
			glVertex3f(pos.x, pos.y, pos.z);
			glVertex3f(pos.x + force.x, pos.y + force.y, pos.z + force.z);

			glEnd();

			glLineWidth(6.0);
			glBegin(GL_LINES);

			force =
				v->m_debug_edge_force +
				v->m_debug_volume_force +
				v->m_debug_area_force +
				v->m_debug_damping_force +
				v->m_debug_floor_force +
				v->m_debug_gravity_force;

			// force = force - v->m_debug_total_force;
			force *= force_scale_factor;
			glColor3f(0.0, 0.5, 0.0);
			glVertex3f(pos.x, pos.y, pos.z);
			glVertex3f(pos.x + force.x, pos.y + force.y, pos.z + force.z);

			glEnd();
		}

		/*
		for(register unsigned int i=0; i<m_nEdges; i++) {

		  edge e = m_edges[i];
		  cDeformableVertex v0 = m_deformableVertices[e.v0];
		  cDeformableVertex v1 = m_deformableVertices[e.v1];
		  glColor3f(1,0,0);
		  glBegin(GL_LINES);
		  glVertex3f(v0.m_pos.x,v0.m_pos.y,v0.m_pos.z);
		  glVertex3f(v1.m_pos.x,v1.m_pos.y,v1.m_pos.z);
		  glEnd();
		}
		*/

	} // if we have vertices to render

#endif


  /*
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  //glEnable(GL_POLYGON_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
  //glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
  */

	if (m_rendering_mesh) {
		// m_rendering_mesh->renderMesh();
		// m_rendering_mesh->renderSceneGraph(a_renderMode);
		// _cprintf("Rendering %d vertices\n",m_rendering_mesh->getNumVertices(true));
	}

	int control_pressed = GetAsyncKeyState(VK_CONTROL) & (1 << 15);

	if (m_rendering_mesh == 0 || control_pressed || m_force_render_tet_mesh || m_disable_skinned_mesh)
		teschner_mesh_parent_mesh_type::renderMesh(a_renderMode);

	std::list<unsigned int>::iterator highlight_iter = m_highlighted_vertices.begin();
	std::list<unsigned int>::iterator color_iter = m_highlight_colors.begin();
	for (; highlight_iter != m_highlighted_vertices.end(); highlight_iter++, color_iter++) {
		unsigned int index = *highlight_iter;
		unsigned int color_index = (color_iter == m_highlight_colors.end()) ? 0 : *color_iter;
		cDeformableVertex* dv = m_deformableVertices + index;
		glPushMatrix();
		cColorf c = CHAI_BASIC_COLORS[color_index % 8];
		glColor3f(c[0], c[1], c[2]);
		glTranslatef(dv->m_pos.x, dv->m_pos.y, dv->m_pos.z);
		cDrawSphere(0.06, 5, 5);
		glPopMatrix();
	}

	int force_constraint_number = 0;

	if (m_render_vertex_constraints) {

		for (unsigned int i = 0; i < m_nVertices; i++) {
			cDeformableVertex* dv = m_deformableVertices + i;

			// Red balls mean position constraints
			if (dv->position_is_constrained) {
				glPushMatrix();
				glColor3f(0.5, 0.5, 1.0);
				glTranslatef(dv->m_pos.x, dv->m_pos.y, dv->m_pos.z);
				cDrawSphere(0.06, 5, 5);
				glPopMatrix();
			}

#define MAX_FORCE_VECTOR_LENGTH 0.8

			// Green balls mean force constraints
			if (dv->force_is_constrained) {

				force_constraint_number++;

				float fx = dv->constraint_force.x;
				float fy = dv->constraint_force.y;
				float fz = dv->constraint_force.z;

				bool point_away = true;
				// if (force_constraint_number <= 2) point_away = true;

				if (point_away) { fx *= -1.0f; fy *= -1.0f; fz *= -1.0f; }

				float maxcomponent = max(max(fabs(fx), fabs(fy)), fabs(fz));
				if (maxcomponent > MAX_FORCE_VECTOR_LENGTH) {
					float d = (float)(MAX_FORCE_VECTOR_LENGTH / maxcomponent);
					fx *= d;
					fy *= d;
					fz *= d;
				}


				/*
				glPushMatrix();
				glColor3f(0.2,1.0,0.2);
				glTranslatef(dv->m_pos.x,dv->m_pos.y,dv->m_pos.z);
				cDrawSphere(0.06,5,5);
				glPopMatrix();
				*/
				/*
				glPushMatrix();
				glColor3f(0.2,1.0,0.2);
				glTranslatef(dv->m_pos.x,dv->m_pos.y,dv->m_pos.z);
				glLineWidth(4.0);
				//cVector3d astart(0,0,0);
				//cVector3d atip(fx,fy,fz);
				//cDrawArrow(astart,atip);
				glBegin(GL_LINES);
				glVertex3f(0,0,0);
				glVertex3f(fx,fy,fz);
				glEnd();
				glPopMatrix();
				*/

				glColor3f(0.3, 1.0, 0.3);
				cVector3d arrowstart(dv->m_pos.x - fx, dv->m_pos.y - fy, dv->m_pos.z - fz);
				cVector3d arrowtip(dv->m_pos.x, dv->m_pos.y, dv->m_pos.z);

				if (point_away)
					cDrawArrow(arrowtip, arrowstart);
				else
					cDrawArrow(arrowstart, arrowtip);

			}
		}
	} // if we're rendering vertex constraints

} // renderMesh


#define LIGHT_VERTEX_MASS 0.001

bool cTeschnerMesh::move_vertices() {

	float timestepsq = m_timestep*m_timestep;
	float two_times_timestep = 2 * m_timestep;

	// Use _current_ forces and _previous_ positions to update _current_ positions,
	// which we write to the _previous_ positions array for now.
	cDeformableVertex* cur_v = m_deformableVertices;

	// These are the same pointer; I maintain two variables
	// for clarity
	cDeformableVertex* prev_v = m_previousPositions;
	cDeformableVertex* next_v = m_previousPositions;

	cVector3f old_pos, old_vel;

	m_maximum_vertex_velocity = m_maximum_vertex_acceleration = 0.0f;
	m_mean_vertex_velocity = m_mean_vertex_acceleration = 0.0f;

	// It will be helpful to have our inverse transform around
	cMatrix3d irot = m_localRot.inv();

	for (unsigned int i = 0; i < m_nVertices; i++, cur_v++, prev_v++, next_v++) {

		if (cur_v->force_is_constrained) {
			cur_v->m_force += cur_v->constraint_force;

			/*
			if (i == 0) {
			  _cprintf("constraint: %s, %s\n",cur_v->constraint_force.str(2).c_str(),cur_v->m_force.str(2).c_str());
			}
			*/
		}

		next_v->m_normal = cur_v->m_normal;

		// Zero out forces for the next time around
		next_v->m_force.set(0, 0, 0);
		next_v->m_mass = cur_v->m_mass;

		next_v->force_is_constrained = cur_v->force_is_constrained;
		next_v->constraint_force = cur_v->constraint_force;
		next_v->position_is_constrained = cur_v->position_is_constrained;

#ifdef RENDER_DEBUG_FORCES
		next_v->m_debug_edge_force = cur_v->m_debug_edge_force;
		next_v->m_debug_volume_force = cur_v->m_debug_volume_force;
		next_v->m_debug_area_force = cur_v->m_debug_area_force;
		next_v->m_debug_damping_force = cur_v->m_debug_damping_force;
		next_v->m_debug_floor_force = cur_v->m_debug_floor_force;
		next_v->m_debug_gravity_force = cur_v->m_debug_gravity_force;
		next_v->m_debug_total_force = cur_v->m_force;

		cur_v->m_debug_edge_force.set(0, 0, 0);
		cur_v->m_debug_volume_force.set(0, 0, 0);
		cur_v->m_debug_area_force.set(0, 0, 0);
		cur_v->m_debug_damping_force.set(0, 0, 0);
		cur_v->m_debug_floor_force.set(0, 0, 0);
		cur_v->m_debug_gravity_force.set(0, 0, 0);
#endif

		// Need to back up the position since next and prev are
		// the same array
		old_pos = prev_v->m_pos;
		old_vel = prev_v->m_velocity;

		if (next_v->position_is_constrained > 0) {
			// _cprintf("Constrained vertex\n");
			// Don't change his position
			next_v->m_pos = cur_v->m_pos;
		}
		else {
			next_v->m_pos = 2.0f*cur_v->m_pos - old_pos +
				timestepsq * cur_v->m_force / cur_v->m_mass;
		}

#ifdef RESTRICT_FLOOR_PENETRATION
		// Find the global position of this vertex
		cVector3d global_pos(
			next_v->m_pos.x, next_v->m_pos.y, next_v->m_pos.z
		);
		m_localRot.mul(global_pos);
		global_pos += m_localPos;

		if (global_pos.y < m_floor_position)
			global_pos.y = m_floor_position;

		global_pos -= m_localPos;
		irot.mul(global_pos);
		next_v->m_pos.set(global_pos);

#endif

		// Maintain max vertex velocity and acceleration
		next_v->m_velocity = (next_v->m_pos - old_pos) / (two_times_timestep);
		cVector3f nextvel = (next_v->m_velocity - old_vel) / (two_times_timestep);

		// Velocity will just be displacement for thresholding (steady-state) computation
		float vel = (next_v->m_pos - cur_v->m_pos).length(); // next_v->m_velocity.length();    
		float accel = (next_v->m_velocity.length() - old_vel.length());

		m_mean_vertex_velocity += vel;
		m_mean_vertex_acceleration += accel;

		if (vel > m_maximum_vertex_velocity) {
			m_maximum_vertex_velocity = vel;
		}
		if (accel > m_maximum_vertex_acceleration) {
			m_maximum_vertex_acceleration = accel;

			/*
			_cprintf("v%d vel (%f) %s, accel %f, force %s\n",i,
			  vel,
			  next_v->m_velocity.str(2).c_str(),
			  accel,
			  cur_v->m_force.str(2).c_str());
			_cprintf("Moving from %.6f,%.6f,%.6f to %.6f,%.6f,%.6f\n\n",
			  cur_v->m_pos.x,cur_v->m_pos.y,cur_v->m_pos.z,
			  next_v->m_pos.x,next_v->m_pos.y,next_v->m_pos.z);
			*/
		}

		/*
		_cprintf("Moving vertex %d to %f,%f,%f\n",
		  i,next_v->m_pos.x,next_v->m_pos.y,next_v->m_pos.z);
		  */
	}

	m_mean_vertex_acceleration /= ((float)(m_nVertices));
	m_mean_vertex_velocity /= ((float)(m_nVertices));

	// Swap previous and current arrays
	cDeformableVertex* tmp = m_previousPositions;
	m_previousPositions = m_deformableVertices;
	m_deformableVertices = tmp;

	return true;
}


bool cTeschnerMesh::compute_forces() {
	compute_contact_forces();
	compute_internal_forces();
	return true;
}


void cTeschnerMesh::compute_contact_forces() {

	// Apply external (mouse) forces
	// force_map::iterator external_force_iter;
	force_list::iterator external_force_iter;
	for (external_force_iter = external_forces.begin();
		external_force_iter != external_forces.end();
		external_force_iter++) {
		// external_force* f = (*external_force_iter).second;
		external_force* f = (*external_force_iter);

		// convert this force to local space
		cVector3f local_force;
		cVector3d local_force_d(f->force.x, f->force.y, f->force.z);
		cMatrix3d irot = m_localRot.inv();
		irot.mul(local_force_d);
		local_force = local_force_d;

		for (unsigned int i = 0; i < f->vertices.size(); i++) {
			int vindex = f->vertices[i];
			cDeformableVertex* v = m_deformableVertices + vindex;
			v->m_force.add(local_force);
			/*
			if (i == 0) {
			  _cprintf("external: %s, %s\n",local_force.str(2).c_str(),v->m_force.str(2).c_str());
			}
			*/
		}
	}

	if (m_gravity_enabled || m_floor_force_enabled) {

		cVector3d gravity_dvector(0, (float)(-1.0*m_gravity_force), 0);

		cMatrix3d irot = m_localRot.inv();
		irot.mul(gravity_dvector);
		cVector3f gravity_fvector = gravity_dvector;

		cVector3d global_pos;
		cVector3f local_floor_force;
		cVector3f gravity_force;

		for (register unsigned int i = 0; i < m_nVertices; i++) {
			cDeformableVertex* v = m_deformableVertices + i;

			// if gravity is enabled...
			if (m_gravity_enabled) {

				// Apply gravity
				float factor = (GRAVITY_IS_ACCELERATION) ? v->m_mass : 1.0f;
				gravity_force = gravity_fvector / factor;
				v->m_force.add(gravity_force);
				/*
				if (i == 0) {
				  _cprintf("gravity: %s, %s\n",gravity_force.str(2).c_str(),v->m_force.str(2).c_str());
				}
				*/

#ifdef RENDER_DEBUG_FORCES
				v->m_debug_gravity_force = gravity_force;
#endif

			} // if gravity is enabled...

#ifdef RENDER_DEBUG_FORCES
			else {
				v->m_debug_gravity_force.set(0, 0, 0);
			}
#endif

			// if the floor is enabled...
			if (m_floor_force_enabled) {

				cVector3f floor_fvector = -1.0*gravity_fvector;
				floor_fvector.normalize();
				floor_fvector *= m_kFloor;

				global_pos.set(v->m_pos.x, v->m_pos.y, v->m_pos.z);
				m_localRot.mul(global_pos);
				global_pos += m_localPos;
				if (global_pos.y < m_floor_position) {
					float floor_scale = (float)(m_floor_position - (float)(global_pos.y));
					local_floor_force = floor_fvector;
					local_floor_force *= floor_scale;
					v->m_force.add(local_floor_force);

					/*
					if (i == 0) {
					  _cprintf("floor: %s, %s\n",local_floor_force.str(2).c_str(),v->m_force.str(2).c_str());
					}
					*/
#ifdef RENDER_DEBUG_FORCES
					v->m_debug_floor_force = local_floor_force;
#endif
				} // floor force

#ifdef RENDER_DEBUG_FORCES
				else {
					v->m_debug_floor_force.set(0, 0, 0);
				}
#endif

			} // if the floor is enabled...

		} // for all vertices 

	} // if we have gravity or floor forces

#ifdef RENDER_DEBUG_FORCES
	else {
		for (register unsigned int i = 0; i < m_nVertices; i++) {
			cDeformableVertex* v = m_deformableVertices + i;
			v->m_debug_gravity_force.set(0, 0, 0);
			v->m_debug_floor_force.set(0, 0, 0);
		}
	}
#endif

} // compute_contact_forces


void cTeschnerMesh::compute_internal_forces() {

	cVector3f cur_vertex_pos, opposing_face_normal, volume_force;

	// For each vertex
	for (register unsigned int i = 0; i < m_nVertices; i++) {

		cDeformableVertex* v = m_deformableVertices + i;

		if (m_compute_vertex_normals && (m_use_cmesh_normal_computation == false))
			v->m_normal.set(0, 0, 0);

		cur_vertex_pos = v->m_pos;

		// Compute tet volume forces    

		// For each tet I live in
		const unsigned int n_local_tets = v->m_nTets;
		unsigned int* curtet_index_ptr = m_containingTets + v->m_startingTetIndex;
		int* cur_opposing_face = m_opposingFaces + v->m_startingTetIndex;

		for (register unsigned int j = 0; j < n_local_tets; j++, curtet_index_ptr++, cur_opposing_face++) {

			int containing_tet_index = *curtet_index_ptr;

			// Find the rest and current volume for this tet
			const float rest_volume = m_restTetVolumes[containing_tet_index];
			const float cur_volume = m_tetVolumes[containing_tet_index];

			// Find the triangle I'm across from in this tet and get his normal
			int facing_triangle = *cur_opposing_face;

			volume_force = m_faceNormalsAndAreas[abs(facing_triangle)];
			bool faces_toward_me = (sign(facing_triangle) >= 0);
			volume_force.normalize();

			// Apply a force along or against that triangle's normal
			// to restore tet volume
			float volumek = m_kVolumePreservation;

			if (m_heterogeneous_constants && (m_heterogeneous_constant_flags & (1 << KVOLUME))) {
				volumek = m_hetero_kVolumePreservation[i];
				float maxk = 0.0f;
				bool nonuniform = false;
				for (int m = 0; m < 4; m++) {
					for (int n = 0; n < 4; n++) {
						if (m_hetero_kVolumePreservation[containing_tet_index + m] != m_hetero_kVolumePreservation[containing_tet_index + n]) {
							nonuniform = true;
							float localmax = max(m_hetero_kVolumePreservation[containing_tet_index + m], m_hetero_kVolumePreservation[containing_tet_index + n]);
							maxk = max(maxk, localmax);
						}
					}
				}

				if (nonuniform) volumek = maxk;
			}

			volume_force *= ((cur_volume - rest_volume) * volumek);

			/*
			_cprintf("Face %d (%f,%f,%f) points %s vertex %d (%f,%f,%f)\n",
			  facing_triangle,
			  opposing_face_normal.x,opposing_face_normal.y,opposing_face_normal.z,
			  faces_toward_me?"toward":"away from",i,
			  cur_vertex_pos.x,cur_vertex_pos.y,cur_vertex_pos.z);
			  */

			if (faces_toward_me) volume_force *= -1.0f;

			v->m_force.add(volume_force);

			/*
			if (i == 0) {
			  _cprintf("volume: %s, %s\n",volume_force.str(2).c_str(),v->m_force.str(2).c_str());
			}
			*/

#ifdef RENDER_DEBUG_FORCES
			v->m_debug_volume_force += volume_force;
#endif

		} // for each tet I live in

		// Compute edge forces

		cVector3f opposing_vertex_pos, vector_toward_me, edge_force, damping_force;

		// For each edge I live in
		const unsigned int n_local_edges = v->m_nEdges;
		unsigned int* curedge_index_ptr = m_containingEdges + v->m_startingEdgeIndex;

		for (register unsigned int j = 0; j < n_local_edges; j++, curedge_index_ptr++) {

			unsigned int curedge_index = *curedge_index_ptr;

			// Find the rest and current length for this edge
			const float rest_length = m_restEdgeLengths[curedge_index];
			const float cur_length = m_edgeLengths[curedge_index];

			edge e = m_edges[curedge_index];

			// Find the vertex that isn't me
			unsigned int opposing_vertex_index = (e.v0 == i) ? e.v1 : e.v0;
			if (e.v1 == e.v0) {
				_cprintf("Warning: degenerate edge\n");
			}

			opposing_vertex_pos = m_deformableVertices[opposing_vertex_index].m_pos;

			// Build a vector pointing from the opposing vertex toward this vertex

			edge_force = cur_vertex_pos;
			edge_force.sub(opposing_vertex_pos);
			edge_force.normalize();

			// Apply a force along or against the edge to restore
			// edge length
			float distancek = m_kDistancePreservation;
			if (m_heterogeneous_constants && (m_heterogeneous_constant_flags & (1 << KDISTANCE))) {
				distancek = m_hetero_kDistancePreservation[i];

				// Check to see whether I have a different constant than my neighbor; if so, 
				// use our average, our max, whatever... but make it the same.
				if (m_hetero_kDistancePreservation[i] != m_hetero_kDistancePreservation[opposing_vertex_index]) {
					distancek = max(m_hetero_kDistancePreservation[i], m_hetero_kDistancePreservation[opposing_vertex_index]);
				}

			}
			edge_force *= distancek * (rest_length - cur_length);

			/*
			_cprintf("Vertex %d to %d, force %f,%f,%f on edge %d\n",
			  i,opposing_vertex_index,edge_force.x,edge_force.y,edge_force.z,curedge_index);
			  */

			v->m_force.add(edge_force);

			/*
			if (i == 0) {
			  _cprintf("edge: %s, %s\n",edge_force.str(2).c_str(),v->m_force.str(2).c_str());
			}
			*/

#ifdef RENDER_DEBUG_FORCES
			v->m_debug_edge_force += edge_force;
#endif      

		} // for each edge I live in

		// Compute face forces and update normal

		// For each face I live in
		const unsigned int n_local_faces = v->m_nFaces;
		unsigned int* curface_index_ptr = m_containingFaces + v->m_startingFaceIndex;

		cVector3f cur_face_normal, rest_face_normal, midpoint, other_vertex_positions[2],
			force_vector, projection, perpendicular, area_force, edge;

		for (register unsigned int j = 0; j < n_local_faces; j++, curface_index_ptr++) {

			unsigned int curface_index = *curface_index_ptr;

			// Find the current normal and length for this face 
			cur_face_normal = m_faceNormalsAndAreas[curface_index];
			float current_area = cur_face_normal.length();

			// Find the rest normal and length for this face 
			rest_face_normal = m_restFaceNormalsAndAreas[curface_index];
			float rest_area = rest_face_normal.length();
			// Not used for anything right now...
			// rest_face_normal.normalize();

			// Find the midpoint between the other two vertices for this face
			unsigned int other_vertex_indices[2];
			face f = m_faces[curface_index];
			if (f.v0 == i) {
				other_vertex_indices[0] = f.v1;
				other_vertex_indices[1] = f.v2;
			}
			else if (f.v1 == i) {
				other_vertex_indices[0] = f.v0;
				other_vertex_indices[1] = f.v2;
			}
			else if (f.v2 == i) {
				other_vertex_indices[0] = f.v0;
				other_vertex_indices[1] = f.v1;
			}
			else {
				_cprintf("Oops... I can't find myself in this face...\n");
			}

			other_vertex_positions[0] =
				m_deformableVertices[(other_vertex_indices[0])].m_pos;
			other_vertex_positions[1] =
				m_deformableVertices[(other_vertex_indices[1])].m_pos;

#define PROJECT_ONTO_PERPENDICULAR_EDGE

#ifndef PROJECT_ONTO_PERPENDICULAR_EDGE

			midpoint = other_vertex_positions[0];
			midpoint.add(other_vertex_positions[1]);
			midpoint.mul(0.5f);

			// Just a vector from this edge toward me, not necessarily
			// optimal...
			cVector3f vector_toward_me = cur_vertex_pos - midpoint;
			vector_toward_me.normalize();
			force_vector = vector_toward_me;

#else

			// Find the perpendicular to this edge that passes through this point

			// Project a line from one p0 to this point onto the opposite edge
			edge = other_vertex_positions[1];
			edge.sub(other_vertex_positions[0]);

			vector_toward_me = cur_vertex_pos;
			vector_toward_me.sub(other_vertex_positions[0]);

			projection = edge;
			projection.mul(
				((edge * vector_toward_me) / (edge * edge))
			);

			perpendicular = vector_toward_me;
			perpendicular.sub(projection);

			if (perpendicular.lengthsq() < CHAI_SMALL) {
				// _cprintf("Warning: small perpendicular in area force...\n");
				force_vector = vector_toward_me;
			}
			else {
				force_vector = perpendicular;
			}
			force_vector.normalize();

#endif
			// Apply a force between myself and that midpoint
			area_force = force_vector;
			float areak = m_kAreaPreservation;
			if (m_heterogeneous_constants && (m_heterogeneous_constant_flags & (1 << KAREA))) {
				areak = m_hetero_kAreaPreservation[i];
				if (
					m_hetero_kAreaPreservation[i] != m_hetero_kAreaPreservation[(other_vertex_indices[0])]
					||
					m_hetero_kAreaPreservation[i] != m_hetero_kAreaPreservation[(other_vertex_indices[1])]
					||
					m_hetero_kAreaPreservation[(other_vertex_indices[1])] != m_hetero_kAreaPreservation[(other_vertex_indices[0])]
					) {
					areak = max(
						max(m_hetero_kAreaPreservation[i], m_hetero_kAreaPreservation[(other_vertex_indices[0])]),
						m_hetero_kAreaPreservation[(other_vertex_indices[1])]
					);
				}
			}
			area_force.mul(
				areak * (rest_area - current_area)
			);

			v->m_force.add(area_force);

			/*
			if (i == 0) {
			  _cprintf("area: %s, %s\n",area_force.str(2).c_str(),v->m_force.str(2).c_str());
			}
			*/


#ifdef RENDER_DEBUG_FORCES
			v->m_debug_area_force += area_force;
#endif

			if (m_compute_vertex_normals && (m_use_cmesh_normal_computation == false)) {
				// Add this face's normal to my accumulating normal
				cur_face_normal.normalize();
				v->m_normal.add(cur_face_normal);
			}
		} // for each face that I live in

		// TODO: this doesn't need to happen every sim pass, just every
		// rendering pass...
		if (m_compute_vertex_normals && (m_use_cmesh_normal_computation == false)) {
			// Normalize and store normal
			v->m_normal.normalize();
			if (i == g_debug_vertex_index) {
				_cprintf("Computed normal %s for vertex %d\n", v->m_normal.str(2).c_str(), i);
			}
		}

		// Add velocity-dependent damping of distance-preservation forces
		//
		// TODO: This is actually global damping, not damping the distance
		// force specifically...
		damping_force = v->m_velocity;
		float dampingk = m_kDistanceDamping;
		if (m_heterogeneous_constants && (m_heterogeneous_constant_flags & (1 << KDAMPING)))
			dampingk = m_hetero_kDistanceDamping[i];
		damping_force *= (-1.0f * dampingk);
		v->m_force.add(damping_force);

		/*
		if (i == 0) {
		  _cprintf("damping: %s, %s\n",damping_force.str(2).c_str(),v->m_force.str(2).c_str());
		}
		*/

#ifdef RENDER_DEBUG_FORCES
		v->m_debug_damping_force = damping_force;
#endif

	} // for each vertex

} // compute_forces()


bool cTeschnerMesh::compute_face_areas() {

	// For each face
	//
	// (skip the zero face, since it's invalid)
	for (register unsigned int i = 1; i < m_nFaces; i++) {

		const face* curface = m_faces + i;

		// Find the position of each vertex
		cVector3f positions[3];
		for (register int j = 0; j < 3; j++) {
			positions[j] = m_deformableVertices[((*curface)[j])].m_pos;
		}

		// Compute face area / normal
		positions[1].sub(positions[0]);
		positions[2].sub(positions[0]);
		m_faceNormalsAndAreas[i] = positions[1];
		m_faceNormalsAndAreas[i].cross(positions[2]);
		m_faceNormalsAndAreas[i].mul(0.5f);
		/*
		m_faceNormalsAndAreas[i] =
		  (positions[1]-positions[0]).cross_and_return(positions[2]-positions[0]) / 2.0f;
		  */
		  /*
		  _cprintf("Face normal %d is %f,%f,%f\n",i,
			m_faceNormalsAndAreas[i].x,m_faceNormalsAndAreas[i].y,
			m_faceNormalsAndAreas[i].z);
			*/
	}

	return true;
}


bool cTeschnerMesh::compute_tet_volumes() {

	float total_volume = 0.0;
	float minvol = 10000000.0;
	float maxvol = 0.0;

	unsigned int bad_tets = 0;

	// For each tet
	for (register unsigned int i = 0; i < m_nTets; i++) {

		const unsigned int* curtet = m_tets + i * 4;

		// Find the position of each vertex
		cVector3f positions[4];
		for (register int j = 0; j < 4; j++) {
			positions[j] = m_deformableVertices[(curtet[j])].m_pos;
		}

		// Compute _signed_ tet volume
		m_tetVolumes[i] = (1.0f / 6.0f) *
			(positions[1] - positions[0]).dot(
			((positions[2] - positions[0]).cross_and_return(positions[3] - positions[0]))
			);

		float v = fabs(m_tetVolumes[i]);
		total_volume += v;
		if (v > maxvol) maxvol = v;
		if (v < minvol) minvol = v;
		if (v < SMALL_TET_VOLUME) {
			// _cprintf("Warning: tet %d has volume %f\n",i,v);
			bad_tets++;
		}
	}

	if (m_initialized == false) {
		_cprintf("Total volume: %f, max tet vol: %f, min tet vol: %f\n",
			total_volume, maxvol, minvol);
		return true;
	}

	if (bad_tets > MAX_BAD_TET_COUNT) return false;
	else return true;

}


bool cTeschnerMesh::compute_spring_lengths() {

	// Compute spring lengths for each spring
	// For each spring
	for (register unsigned int i = 0; i < m_nEdges; i++) {
		// Compute spring length
		const edge* e = m_edges + i;
		m_edgeLengths[i] =
			m_deformableVertices[e->v0].m_pos.distance(
				m_deformableVertices[e->v1].m_pos);
	}

	return true;
}


void cTeschnerMesh::applyForce(unsigned int vertex_index, cVector3f& force) {
	if (vertex_index >= m_nVertices) return;
	m_deformableVertices[vertex_index].m_force.add(force);

	if (vertex_index == 0) {
		_cprintf("apply: %s, %s\n", force.str(2).c_str(), m_deformableVertices[vertex_index].m_force.str(2).c_str());
	}
}

void cTeschnerMesh::build_rendering_weights() {

	if (m_initialized == 0 || m_rendering_mesh == 0 || m_nVertexBoundaryMarkers == 0) {
		_cprintf("\n\nCannot build rendering weights...\n\n");
		return;
	}

	_cprintf("Building rendering weights...\n");

#define KD_EPSILON 0.0001f

	m_compute_vertex_normals = true;
	m_weights_per_vertex = WEIGHTS_PER_VERTEX;
	int neighbors_to_find = m_weights_per_vertex + EXTRA_NEIGHBORS_TO_FIND;

	unsigned int n_rendering_vertices = m_rendering_mesh->getNumVertices(true);

	unsigned int n_total_weights = n_rendering_vertices*m_weights_per_vertex;

	m_vertex_weights = new float[n_total_weights];
	m_vertex_effectors = new int[n_total_weights];
	m_vertex_normal_offsets = new float[n_total_weights];
	m_vertex_tangential_offsets = new cVector3f[n_total_weights];
	m_vertex_coordframe_positions = new cVector3f[n_total_weights];
	m_opposite_edge_vertices = new int[n_total_weights];

	// Create a kd-tree
	ANNpoint* kd_points;
	ANNidx* nearest_indices;
	ANNdist* nearest_sq_distances;
	ANNkd_tree* tree;
	ANNpoint querypoint;

	querypoint = annAllocPt(3);
	nearest_indices = new ANNidx[neighbors_to_find];
	nearest_sq_distances = new ANNdist[neighbors_to_find];

	unsigned int i;
	int j;

	std::vector<unsigned int> border_vertex_indices;
	border_vertex_indices.reserve(m_nVertices);

	// Find border indices
	for (i = 0; i < m_nVertices; i++) {
		int* markerpos = m_vertexBoundaryMarkers + (i*m_nVertexBoundaryMarkers);
		if (markerpos[0]) {
			border_vertex_indices.push_back(i);
		}
	}

	unsigned int n_border_vertices = border_vertex_indices.size();

	kd_points = annAllocPts(n_border_vertices, 3);

	// For all of my _border_ vertices 
	for (i = 0; i < n_border_vertices; i++) {

		int index = border_vertex_indices[i];

		// Put this vertex in the tree
		ANNpoint p = kd_points[i];
		for (int k = 0; k < 3; k++) {
			p[k] = m_deformableVertices[index].m_pos[k];
		}
	}

	_cprintf("Building kd tree...\n");
	tree = new ANNkd_tree(kd_points, n_border_vertices, 3);
	_cprintf("Finished building kd tree...\n");

	std::vector<cVertex>* rendering_vertex_vector = m_rendering_mesh->pVerticesNonEmpty();
	cVertex* vertex_array = (cVertex*) &((*rendering_vertex_vector)[0]);
	if (n_rendering_vertices != rendering_vertex_vector->size()) {
		_cprintf("\nWarning: nrv is %d, but vvs is %d\n\n", n_rendering_vertices, rendering_vertex_vector->size());
		_getch();
		n_rendering_vertices = rendering_vertex_vector->size();
	}

	// For each of my rendering mesh's vertices
	for (i = 0; i < n_rendering_vertices; i++) {

		cVector3d p = vertex_array[i].m_localPos;
		cVector3f cur_vertex_pos(p);

		querypoint[0] = p.x;
		querypoint[1] = p.y;
		querypoint[2] = p.z;

		// Find the closest n vertices in the tree (border vertices)
		tree->annkSearch(querypoint, neighbors_to_find, nearest_indices, nearest_sq_distances, KD_EPSILON);

		float total_inverse_squared_distance = 0.0f;

		int valid_weights_found = 0;

		// Look at each of these nearest vertices, stopping when we have enough weights
		for (j = 0; (j < neighbors_to_find) && (valid_weights_found < m_weights_per_vertex); j++) {

			unsigned int weight_offset = i*m_weights_per_vertex + valid_weights_found;

			unsigned int cur_pt_idx_in_border_list = nearest_indices[j];
			unsigned int cur_pt_idx = border_vertex_indices[cur_pt_idx_in_border_list];

			cDeformableVertex* cdv = m_deformableVertices + cur_pt_idx;

			cVector3f cur_pt_pos = cdv->m_pos;
			cVector3f cur_pt_normal = cdv->m_normal;

			bool valid_effector = true;

			// Decide whether this weight is valid
			if (m_use_coordinate_frame_based_skinning) {

				// Does this vertex have a triangle associated with him?
				int nedges = cdv->m_nEdges;
				if (nedges == 0) {
					_cprintf("Skipping dvertex %d for rendering vertex %d: no edges\n", cur_pt_idx, i);
					continue;
				}

				int containing_edge_index = cdv->m_startingEdgeIndex;
				bool found_border_edge = false;

				for (int k = 0; k < nedges; k++, containing_edge_index++) {

					int edge_number = m_containingEdges[containing_edge_index];

					// What vertex is on the other side of this edge?
					int other_pt_index = m_edges[edge_number].v0;
					if (other_pt_index == cur_pt_idx) other_pt_index = m_edges[edge_number].v1;

					// Is he a border vertex?  If so, this is a border edge?
					bool borderedge = 0;

					int* markerpos = m_vertexBoundaryMarkers + (other_pt_index*m_nVertexBoundaryMarkers);
					if (markerpos[0]) borderedge = 1;

					if (borderedge) {
						//_cprintf("Found a border edge from %d to %d\n",cur_pt_idx,other_pt_index);
						// Record this edge as the other side of my axis-defining edge
						m_opposite_edge_vertices[weight_offset] = other_pt_index;
						found_border_edge = true;
						break;
					}
					else {
						//_cprintf("Edge from %d to %d for vertex %d is not a border edge...\n",cur_pt_idx,other_pt_index,i);
						continue;
					}
				} // For each edge this vertex is part of

				if (found_border_edge == false) {
					_cprintf("Skipping dvertex %d for rendering vertex %d: no border edges\n", cur_pt_idx, i);
					continue;
				}

			} // If we need to look for border edges

			// Mark invalid weight positions...
			if (valid_effector == false) {
				m_vertex_effectors[weight_offset] = -1;
				continue;
			}

			valid_weights_found++;

			// Store the index of this point
			m_vertex_effectors[weight_offset] = cur_pt_idx;

			//////////////
			// Normal + offset approach
			//////////////

			// Project this point onto this vector's normal
			cVector3f point_to_vertex = cur_vertex_pos - cur_pt_pos;

			cVector3f projection = (point_to_vertex*cur_pt_normal)*cur_pt_normal;

			// Find projected point
			cVector3f projected_point = cur_pt_pos + projection;

			// Compute and store offset along his normal
			float normal_offset = projection.length();
			m_vertex_normal_offsets[weight_offset] = normal_offset;

			// Compute and store tangential offset
			cVector3f tangential_offset = cur_vertex_pos - projected_point;
			m_vertex_tangential_offsets[weight_offset] = tangential_offset;

			//////////////
			// Coordinate frame approach
			//////////////

			if (m_use_coordinate_frame_based_skinning) {

				cVector3f origin = cur_pt_pos;

				// One axis is the vertex normal (unit)
				cVector3f axis1 = cur_pt_normal;

				// The next will be the first triangle axis, made perpendicular to the normal
				unsigned int other_pt_idx = m_opposite_edge_vertices[weight_offset];
				cVector3f other_pt_pos = m_deformableVertices[other_pt_idx].m_pos;
				cVector3f edge = other_pt_pos - origin;

				// Component parallel to my normal
				cVector3f axis2_parallel = (edge*axis1) * axis1;

				// Component perpendicular to my normal
				cVector3f axis2_perpendicular = edge - axis2_parallel;

				// The normalized perpendicular is my second axis
				cVector3f axis2 = axis2_perpendicular;
				axis2.normalize();

				// The third axis will be their cross-product
				cVector3f axis3 = axis1;
				axis3.cross(axis2);

				// Find and store the components along each axis

				// The vector in world-space
				cVector3f offset_in_world_space = cur_vertex_pos - origin;
				cVector3f offset_in_vertex_space;

				cMatrix3d rot;
				cVector3d coffset_in_world_space;
				cVector3d coffset_in_vertex_space;

				/*
				// Rotate into vertex-space
				for(unsigned int ri=0; ri<3; ri++) {
				  for(unsigned int rj=0; rj<3; rj++) {
					rot.m[rj][ri] =
					  offset_in_world_space[ri]*axis1[rj]+
					  offset_in_world_space[ri]*axis2[rj]+
					  offset_in_world_space[ri]*axis3[rj];
				  }
				}
				*/

				cVector3d caxis1(axis1.x, axis1.y, axis1.z);
				cVector3d caxis2(axis2.x, axis2.y, axis2.z);
				cVector3d caxis3(axis3.x, axis3.y, axis3.z);
				coffset_in_world_space.set(offset_in_world_space.x, offset_in_world_space.y, offset_in_world_space.z);
				rot.setCol(caxis1, caxis2, caxis3);

				rot.invert();
				rot.mulr(coffset_in_world_space, coffset_in_vertex_space);
				offset_in_vertex_space.set(coffset_in_vertex_space);

				/*
				offset_in_vertex_space.set(
				  offset_in_world_space * axis1,
				  offset_in_world_space * axis2,
				  offset_in_world_space * axis3
				  );
				*/

				m_vertex_coordframe_positions[weight_offset] = offset_in_vertex_space;

#ifdef DEBUG_VERTEX_INDEX
				if (i == DEBUG_VERTEX_INDEX) {
					g_debug_vertex_index = m_vertex_effectors[weight_offset];
					_cprintf("Building vertex %d,%d (%d) (wo %d): origin %s, otherpos %s, axis1 %s, axis2 %s, axis3 %s, vspace %s, wspace %s\n",
						i, weight_offset, g_debug_vertex_index, weight_offset,
						origin.str(2).c_str(),
						other_pt_pos.str(2).c_str(),
						axis1.str(2).c_str(), axis2.str(2).c_str(), axis3.str(2).c_str(),
						offset_in_vertex_space.str(2).c_str(),
						offset_in_world_space.str(2).c_str());
					// _getch();
				}
#endif

			} // if we're doing coordinate-based skinning

			// Compute squared distance
			float distancesq = cur_vertex_pos.distancesq(cur_pt_pos);

			float inverse_squared_distance = (float)(1.0 / distancesq);

			// Store weight as inverse_squared_distance
			m_vertex_weights[weight_offset] = inverse_squared_distance;

			// Update total distance
			total_inverse_squared_distance += inverse_squared_distance;

		}

		if (valid_weights_found < m_weights_per_vertex) {
			_cprintf("Warning: vertex %d only has %d weights (instead of %d)\n", i, valid_weights_found, m_weights_per_vertex);
		}

		float normalization_factor = (float)(1.0 / total_inverse_squared_distance);

		// Normalize weights

		// For each weighted vertex
		for (j = 0; j < valid_weights_found; j++) {

			// Update weight to reflect normalization
			m_vertex_weights[i*m_weights_per_vertex + j] *= normalization_factor;

		}

	} // For each vertex in the skinning mesh

	_cprintf("Cleaning up kd tree\n");

	// Clean up
	annDeallocPt(querypoint);
	if (nearest_indices) delete[] nearest_indices;
	if (nearest_sq_distances) delete[] nearest_sq_distances;
	if (kd_points) { annDeallocPts(kd_points); }
	if (tree) delete tree;

	_cprintf("Cleaned up kd tree\n");

	this->addChild(m_rendering_mesh);
}


void cTeschnerMesh::prepare_heterogeneous_constant_rendering() {

	_cprintf("Preparing heterogeneous constant rendering...\n");

	// If heterogeneity is not enabled, return
	if (m_heterogeneous_constants == false) return;

	std::vector<cVertex>* vertex_vector = this->pVerticesNonEmpty();
	cVertex* vertex_array = (cVertex*) &((*vertex_vector)[0]);

	float maxval = 0.0;
	float minval = FLT_MAX;

	for (unsigned int i = 0; i < m_nVertices; i++) {
		if (m_hetero_kDistancePreservation[i] < minval)
			minval = m_hetero_kDistancePreservation[i];
		if (m_hetero_kDistancePreservation[i] > maxval)
			maxval = m_hetero_kDistancePreservation[i];
	}

	float range = maxval - minval;

	_cprintf("Max,min kd values: %f,%f\n", maxval, minval);

	// Map kd's to vertex colors
	for (unsigned int i = 0; i < m_nVertices; i++) {
		float val = m_hetero_kDistancePreservation[i];
		float color = (val - minval) / range;
		/*
		unsigned char r = (unsigned char)(color * 255.0);
		unsigned char g = 50;
		unsigned char b = 50;
		*/
		int colorindex = (int)(color*255.0f);
		if (colorindex < 0) colorindex = 0;
		if (colorindex > 255) colorindex = 255;
		unsigned char r = colormap[colorindex][0] * 255.0f;
		unsigned char g = colormap[colorindex][1] * 255.0f;
		unsigned char b = colormap[colorindex][2] * 255.0f;

		/*
		// For the ground truth dragon plot
		if (colorindex > 128) {
		  r = 255; g = 77; b = 77;
		}
		else {
		  r = 77; g = 77; b = 255;
		}
		*/

		vertex_array[i].m_color.set(r, g, b);
	}

	if (m_rendering_mesh && (m_disable_skinned_mesh == false)) {

		m_rendering_mesh->useMaterial(false, true);
		m_rendering_mesh->useColors(true, true);

		// std::vector<cVertex>* rendering_vertex_vector = m_rendering_mesh->pVerticesNonEmpty();
		std::vector<cVertex>* rendering_vertex_vector = m_rendering_mesh->pVertices();
		cVertex* rendering_vertex_array = (cVertex*) &((*rendering_vertex_vector)[0]);
		unsigned int n_rendering_vertices = rendering_vertex_vector->size();
		cVertex* cur = rendering_vertex_array;
		cVertex* end = rendering_vertex_array + n_rendering_vertices;

		int weight_offset = 0;

		unsigned int vcount = 0;

		// For each of my rendering mesh's vertices
		while (cur != end) {

			vcount++;

			cur->m_color.set(0, 0, 0);

			// For each relevant effector vertex
			for (unsigned int j = 0; j < m_weights_per_vertex; j++) {

				// This will be the "vote" from this weight source
				cColorb tempcolor;

				// Find the effector point
				int effector_index = m_vertex_effectors[weight_offset];

				// A -1 value says "no more effectors"
				if (effector_index < 0) break;
				cVertex* source = vertex_array + effector_index;

				if (m_use_coordinate_frame_based_skinning) {
					tempcolor = source->m_color;
				}

				else {

				}

				tempcolor.m_color[0] = ((float)(tempcolor.m_color[0])) * m_vertex_weights[weight_offset];
				tempcolor.m_color[1] = ((float)(tempcolor.m_color[1])) * m_vertex_weights[weight_offset];
				tempcolor.m_color[2] = ((float)(tempcolor.m_color[2])) * m_vertex_weights[weight_offset];

				cur->m_color.m_color[0] += tempcolor.m_color[0];
				cur->m_color.m_color[1] += tempcolor.m_color[1];
				cur->m_color.m_color[2] += tempcolor.m_color[2];

				weight_offset++;

			} // for each effector

			cur++;

		} // for each vertex in my rendering mesh
	} // if I have a rendering mesh


	// Enable vertex-color rendering and disable material property rendering...
	m_useVertexColors = true;
	m_useMaterialProperty = false;

}