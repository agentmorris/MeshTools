#ifndef _CONSTRAINTS_H_
#define _CONSTRAINTS_H_

// #define BOOST_NO_AUTO_LINK

#define BOOST_LIB_DIAGNOSTIC
#define BOOST_LIB_TOOLSET "vc71"
#define BOOST_LIB_THREAD_OPT "-mt"
#ifdef _DEBUG
#define BOOST_LIB_RT_OPT "-gd"
#else
#define BOOST_LIB_RT_OPT
#endif

/*
#ifdef _DEBUG
#define DEBUG_BUILD 1
#else
#define DEBUG_BUILD 0
#endif
#undef _DEBUG
#undef __MSVC_RUNTIME_CHECKS
*/

// #define BOOST_NO_WREGEX
// #undef BOOST_REGEX_HAS_MS_STACK_GUARD
// #include <boost/regex.hpp>

/*
#if DEBUG_BUILD
#define _DEBUG
#define __MSVC_RUNTIME_CHECKS
#endif
*/

// using namespace boost;

#include "CVector3f.h"
#include "deformable_mesh_data_structures.h"
#include <list>
#include <set>
#include <map>
#include <vector>

typedef enum {
	CONSTRAINT_UNINITIALIZED = 0,
	CONSTRAINT_FORCE_CONSTANT,
	CONSTRAINT_FORCE_IMPULSE,
	CONSTRAINT_POSITION_CONSTANT,
	CONSTRAINT_POSITION_IMPULSE,
	CONSTRAINT_CONTACT,
	NUM_AVAILABLE_CONSTRAINT_TYPES
} constraint_type;

class constraint {

public:
	// From the above enumeration
	int type;

	// An arbitrary tag
	int tag;

	// Start and end times
	float start_time;
	float end_time;

	virtual ~constraint() {}

	// Which vertices does this apply to?
	bool affects_all_vertices;
	std::vector<unsigned int> vertices;

	constraint() {
		affects_all_vertices = false;
		tag = -1;
		type = CONSTRAINT_UNINITIALIZED;
		start_time = -1.0;
		end_time = -1.0;
	}
};

class constraint_force_constant : public constraint {
public:
	cVector3f force;
};

class constraint_position_constant : public constraint {
public:
	// If 'lock' is true, each vertex is set to whatever
	// position it was in when the constraint was enabled
	// and isn't moved again.  In this case, 'position'
	// isn't meaningful.
	bool lock;
	cVector3f position;

};

struct lt_constraint_start
{
	bool operator()(const constraint* c1, const constraint* c2) const
	{
		return (c1->start_time < c2->start_time);
	}
};

struct lt_constraint_end
{
	bool operator()(const constraint* c1, const constraint* c2) const
	{
		return (c1->end_time < c2->end_time);
	}
};

class constraint_set {

public:
	void add_constraint(constraint* c);
	bool add_constraint(const char* constraint_str);

	void reset();
	void clear();

	// Call these until they return zero to get constraints that are
	// starting and ending

	// Returns constraints that should have started now
	constraint* get_next_constraint_start(float curtime);

	// Returns constraints that should have ended now
	constraint* get_next_constraint_end(float curtime);

	// Read-only... use to get the list of active constraints
	std::map<unsigned int, constraint*> active_constraints;
	std::multiset<constraint*, lt_constraint_start> constraints_by_start_time;
	std::multiset<constraint*, lt_constraint_end> constraints_by_end_time;

	// If you just want the list of active constraints to be 
	// maintained, use this...
	void idle(float curtime);

	virtual ~constraint_set();

protected:
	std::multiset<constraint*, lt_constraint_start>::iterator next_constraint_start;
	std::multiset<constraint*, lt_constraint_end>::iterator next_constraint_end;

};


#endif