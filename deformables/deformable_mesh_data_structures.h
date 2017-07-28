#ifndef _DEFORMABLE_MESH_DATA_STRUCTURES_H_
#define _DEFORMABLE_MESH_DATA_STRUCTURES_H_

#include "CVector3f.h"

#ifdef _DEBUG
#define RENDER_DEBUG_FORCES
#else
// #define RENDER_DEBUG_FORCES
#endif

/*

when rendering debug forces:

white  == total force
green  == volume force
blue   == area force
red    == edge force
yellow == damping force
purple == floor force
teal   == gravity

*/

struct face {
	unsigned int v0, v1, v2;
	face(const unsigned int& v0, const unsigned int& v1, const unsigned int& v2) {
		this->v0 = v0; this->v1 = v1; this->v2 = v2;
	}
	inline unsigned int operator[] (const unsigned int& index) const {
		return (((unsigned int*)(this))[index]);
	}
	inline unsigned int& operator[] (const unsigned int& index) {
		return (((unsigned int*)(this))[index]);
	}
	face() {}
};

struct edge {
	unsigned int v0, v1;
	edge(const unsigned int& v0, const unsigned int& v1) {
		this->v0 = v0; this->v1 = v1;
	}
	edge() {}
};

struct cDeformableVertex {
	cVector3f m_pos;
	cVector3f m_force;
	cVector3f m_normal;
	cVector3f m_velocity;

#ifdef RENDER_DEBUG_FORCES
	cVector3f m_debug_edge_force;
	cVector3f m_debug_volume_force;
	cVector3f m_debug_area_force;
	cVector3f m_debug_damping_force;
	cVector3f m_debug_floor_force;
	cVector3f m_debug_gravity_force;
	cVector3f m_debug_total_force;
#endif

	// How many position constraints affect this vertex?
	unsigned char position_is_constrained;

	// How many force constraints affect this vertex?
	unsigned char force_is_constrained;
	cVector3f constraint_force;

	float m_mass;

	unsigned int m_nTets;
	unsigned int m_startingTetIndex;
	unsigned int m_nFaces;
	unsigned int m_startingFaceIndex;
	unsigned int m_nEdges;
	unsigned int m_startingEdgeIndex;
};



#endif