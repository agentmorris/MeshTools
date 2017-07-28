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

#ifndef MESH_DATA_STRUCTURES_H
#define MESH_DATA_STRUCTURES_H

#include <algorithm>

struct indexed_face {
	unsigned int vertices[3];
};

struct ltindexed_face {
	inline bool operator()(const indexed_face& f1, const indexed_face& f2) const
	{
		if (f1.vertices[0] < f2.vertices[0]) return true;
		else if (f1.vertices[0] > f2.vertices[0]) return false;
		else if (f1.vertices[1] < f2.vertices[1]) return true;
		else if (f1.vertices[1] > f2.vertices[1]) return false;
		else if (f1.vertices[2] < f2.vertices[2]) return true;
		return false;
	}
};

inline void sort_indexed_face(indexed_face& f) {
	std::sort(f.vertices, f.vertices + 3);
};

inline bool isDegenerateFace(indexed_face& f) {
	if (
		f.vertices[0] == f.vertices[1] ||
		f.vertices[0] == f.vertices[2] ||
		f.vertices[1] == f.vertices[2]
		) return true;
	return false;
}


struct indexed_tet {
	unsigned int vertices[4];
	unsigned int attribute;
};

struct ltindexed_tet {
	inline bool operator()(const indexed_tet& t1, const indexed_tet& t2) const
	{
		if (t1.vertices[0] < t2.vertices[0]) return true;
		else if (t1.vertices[0] > t2.vertices[0]) return false;
		else if (t1.vertices[1] < t2.vertices[1]) return true;
		else if (t1.vertices[1] > t2.vertices[1]) return false;
		else if (t1.vertices[2] < t2.vertices[2]) return true;
		else if (t1.vertices[2] > t2.vertices[2]) return false;
		else if (t1.vertices[3] < t2.vertices[3]) return true;
		return false;
	}
};

inline void sort_indexed_tet(indexed_tet& t) {
	std::sort(t.vertices, t.vertices + 4);
};

inline bool isDegenerateTet(indexed_tet& t) {
	if (t.vertices[0] == t.vertices[1] ||
		t.vertices[0] == t.vertices[2] ||
		t.vertices[0] == t.vertices[3] ||
		t.vertices[1] == t.vertices[2] ||
		t.vertices[1] == t.vertices[3] ||
		t.vertices[2] == t.vertices[3])
		return true;
	return false;
}


#endif