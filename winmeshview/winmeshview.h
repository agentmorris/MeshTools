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

#if !defined(AFX_winmeshview_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
#define AFX_winmeshview_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "CWorld.h"
#include "CViewport.h"
#include "CGenericObject.h"
#include "CGeneric3dofPointer.h"
#include "CPrecisionTimer.h"
#include "CLight.h"
#include "CVBOMesh.h"
#include "CPanel.h"
#include "cLabelPanel.h"
#include "meshImporter.h"

typedef enum {
	MOUSE_BUTTON_RIGHT = 0, MOUSE_BUTTON_LEFT, MOUSE_BUTTON_MIDDLE
} mouse_buttons;

typedef enum {
	CUT_PLANE_NONE = 0,
	CUT_PLANE_XY,
	CUT_PLANE_XZ,
	CUT_PLANE_YZ
} AVAIALBLE_CUT_PLANES;

// A global function for sticking a cube in the given mesh
// 
// Manually creates the 12 triangles (two per face) required to
// model a cube
void createCube(cMesh *mesh, float edge);


class CwinmeshviewApp : public CWinApp, public CCommandLineInfo, public cGenericObject {
public:
	CwinmeshviewApp();

	// See whether the user specified a model on the command line;
	// load it if they did.
	virtual bool loadCommandLineModel();

	// Load the default model file if we can find it, otherwise create
	// a simple cube.
	virtual bool createDefaultModel();

	// Reset the camera to its initial positions
	void resetCamera();

	virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);
	int m_current_param_index;

	int m_current_cut_plane;
	float m_current_cut_plane_position;
	bool m_invert_clip_plane;

	// Call this in place of a destructor to clean up
	void uninitialize();

	virtual int render_loop();

	virtual void render(const int a_renderMode = CHAI_RENDER_MODE_RENDER_ALL);

	void process_ini_file();

	// An object of some kind, to be rendered in the scene
	cMesh* object;

	char m_loaded_mesh_filename[_MAX_PATH];

	// What xform was applied to this mesh when it was loaded
	mesh_xform_information current_mesh_transform;

	// Used to generate new meshes
	cMesh* m_factory_mesh;

	// The visual representation of the (optional) cutting plane
	cPanel* cutplane;

	// Light source
	cLight* light;

	// Move the on-screen object if necessary
	//
	// This function is run on the haptics thread if it's running,
	// on the graphics thread otherwise
	void animate();

	// Loads the specified model from a supported 3d model format file
	//
	// Returns true for success, false for failure.
	virtual bool LoadModel(const char* filename,
		bool build_collision_detector = false,
		bool finalize = true, bool delete_old_model = true);

	// Loads a texture (image) file and applies it to the current
	// object
	int LoadTexture(char* filename);

	// Sets the camera zoom level
	void zoom(int zoom_level);

	// Handles mouse-scroll events (moves or rotates the selected object)
	virtual void scroll(CPoint p, int button = MOUSE_BUTTON_LEFT);

	// Handles mouse clicks (marks the front-most clicked object as 'selected')
	virtual void select(CPoint p);

	// The currently selected object (or zero when there's no selection)
	cGenericObject* selected_object;
	cTriangle* selected_triangle;

	// The interface to the haptic device...
	cGeneric3dofPointer *tool;

	// The high-precision timer that's used (optionally) to run
	// the haptic loop
	cPrecisionTimer timer;

	// A flag that indicates whether haptics are currently enabled
	int haptics_enabled;

	// A flag that indicates whether the haptics thread is currently running
	//
	// This flag does not get set when the haptic callback is driven from a 
	// multimedia timer 
	int haptics_thread_running;

	// A flag that indicates whether we're currently moving the selected object
	int moving_object;

	// A flag that indicates whether we should render on the GUI thread
	bool m_render_on_gui_thread;

#define TOGGLE_HAPTICS_TOGGLE  -1
#define TOGGLE_HAPTICS_DISABLE  0
#define TOGGLE_HAPTICS_ENABLE   1

	// If the parameter is -1, haptics are toggled on/off
	// If it's 0 haptics are turned off
	// If it's 1 haptics are turned on
	void toggle_haptics(int enable = TOGGLE_HAPTICS_TOGGLE);

	// Enables or disables the background animation of the current object
	void toggle_animation();

	// Keep track of the time that elapsed between iterations of
	// the animate() function, and the wall-clock time at which the
	// last iteration ran
	double m_last_animation_time;

	// The current velocity at which the object is moving (units/s)
	cVector3d m_animation_velocity;

	// Reinitializes the CHAI viewport
	void reinitialize_viewport(int stereo_enabled = 0);

	// virtual world
	cWorld* world;
	cCamera* camera;
	cViewport* viewport;
	cLabelPanel* label_panel;

	int keys_to_handle[1024];

	void handle_message(MSG& msg);

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CwinmeshviewApp)
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run();
	//}}AFX_VIRTUAL

  // Implementation

	  //{{AFX_MSG(CwinmeshviewApp)
	  //}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_winmeshview_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
