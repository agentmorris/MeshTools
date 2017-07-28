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

#if !defined(AFX_voxelizer_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
#define AFX_voxelizer_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

typedef enum {
	MOUSE_BUTTON_RIGHT = 0, MOUSE_BUTTON_LEFT, MOUSE_BUTTON_MIDDLE
} mouse_buttons;

typedef enum {
	OPERATION_VOXELIZE = 0, OPERATION_TETRAHEDRALIZE, OPERATION_TESTONLY
} voxelizer_operations;

#include "resource.h"
#include "CWorld.h"
#include "CViewport.h"
#include "CGenericObject.h"
#include "CPhantom3dofPointer.h"
#include "CPrecisionTimer.h"
#include <vector>
#include "CCollisionAABB.h"
#include "CMeta3dofPointer.h"
#include "CLight.h"

// Controls whether I compile and link with the cpt library
// (which has to be in ../cpt/cpt)
//
// This is off right now, since I just do the cpt explicitly using
// CHAI primitives.
// #define COMPILE_CPT

#ifdef COMPILE_CPT
#include "cpt_c.h"
#endif

// #define STORE_POINT_CLOUD_DISTANCE_INFO

struct simple_point {
	float x, y, z;
	float r, g, b;

#ifdef STORE_POINT_CLOUD_DISTANCE_INFO
	cVector3d closest_point;
	float distance;

	simple_point() {
		x = y = z = r = g = b = distance = 0;
		closest_point.zero();
	}
#endif

};

class CPointCloud : public cGenericObject {

public:
	CPointCloud();
	virtual ~CPointCloud();

	// A map from the corresponding voxel id to the graphical point
	std::map<int, simple_point> points;

	virtual void render(const int a_renderMode);
	HANDLE point_mutex;
};


// A function used to pop up a dialog box and ask the user to select
// a file (for selecting game files) (last five parameters are optional).
int FileBrowse(char* buffer, int length, int save = 0, char* forceExtension = 0,
	char* extension_string = 0, char* title = 0);

class CvoxelizerApp : public CWinApp, public cGenericObject {
public:
	CvoxelizerApp();

	// Call this in place of a destructor to clean up
	void uninitialize();

	void process_ini_file();
	void write_results_file();

	void render(const int a_renderMode);

	virtual int render_loop();

	void voxelize_current_object(int operation = OPERATION_VOXELIZE);
	void cpt_current_object();

	void launch_voxelization();

	char output_filename_root[_MAX_PATH];

	// An object of some kind, to be rendered in the scene
	cMesh* object_to_voxelize;
	std::vector<cMesh*> modifier_objects;
	std::vector<cMesh*> objects_to_subtract;
	cGenericObject* current_voxelization_point;

	// Set by the GUI before a voxelization
	unsigned int m_long_axis_resolution;
	float m_normal_multiplier;
	int seed_triangle_index;
	int m_compute_distance_field;
	int m_write_output_file;

	// Grab relevant options from checkboxes and sliders in the GUI
	void update_options_from_gui();
	void update_gui_from_options();

	char current_texture[_MAX_PATH];

	cMesh* LoadModel(char* filename, int compute_new_offsets = 1);

	int LoadObjectToVoxelize(char* filename);
	int LoadObjectToSubtract(char* filename);
	int LoadModifierObject(char* filename);

	int LoadTexture(char* filename);

	void check_mesh();
	void zoom(int zoom_level);
	// Handles mouse-scroll events (moves or rotates the selected object)
	void scroll(CPoint p, int button = MOUSE_BUTTON_LEFT);
	void select(CPoint p);
	cGenericObject* selected_object;
	cTriangle* selected_tri;

	// The interface to the haptic device...
	cMeta3dofPointer *tool;

	cPrecisionTimer timer;

	int haptics_enabled;
	int haptics_thread_running;

#define TOGGLE_HAPTICS_TOGGLE  -1
#define TOGGLE_HAPTICS_DISABLE  0
#define TOGGLE_HAPTICS_ENABLE   1

	// If the parameter is -1, haptics are toggled on/off
	// If it's 0 haptics are turned off
	// If it's 1 haptics are turned on
	void toggle_haptics(int enable = TOGGLE_HAPTICS_TOGGLE);

	CPointCloud* cloud;

	int quit_voxelizing;
	int multithreaded_voxelizer;
	int render_point_cloud;

	double total_time;
	double distance_time;
	unsigned int found_voxels;
	unsigned int m_nTets;

	// Closet-point transform data
#ifdef COMPILE_CPT
	cpt::number_type* m_distance_array;
	cpt::number_type* m_closest_point_array;
	cpt::number_type* m_gradient_array;
	int* m_closest_face_array;
#endif

	/***

	Voxelization info that is exported with the outgoing model...

	***/

	// While voxelizing, this is the offset we need to add to each coordinate
	// to account for roundoff...
	cVector3d voxel_start_offset;

	// Number of voxels along each axis
	int voxel_resolution[3];

	// Size of a voxel along each axis
	cVector3d voxel_size;

	// The world-space coordinate of the (0,0,0) voxel
	cVector3d zero_coordinate;

	// The object position before re-positioning...
	cVector3d old_object_pos;

	// The scale factor that we multiplied the original
	// model by to get our current model
	double model_scale_factor;

	// The vector by which we offset the original model
	// to get our current model
	cVector3d model_offset;

	/***

	End exported info

	***/


	// virtual world
	cWorld* world;
	cCamera* camera;
	cViewport* viewport;

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CvoxelizerApp)
public:
	virtual BOOL InitInstance();
	virtual int Run();
	//}}AFX_VIRTUAL

  // Implementation

	  //{{AFX_MSG(CvoxelizerApp)
	  //}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


class AABB_distance_computer {

public:
	AABB_distance_computer();
	virtual ~AABB_distance_computer();

	// Find the closest vertex to point p... p can be in global space
	static double closest_vertex(cMesh* mesh,
		cVector3d p, cTriangle* closest_tri = 0, cVector3d* closest_point = 0, const int reinit_mesh_list = 0);

	static double closest_vertex_to_single_mesh(cCollisionAABB* collider,
		cVector3d p, cTriangle* closest_tri = 0, cVector3d* closest_point = 0);

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_voxelizer_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)


