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

#include "stdafx.h"
#include "voxelizer.h"
#include "voxelizerDlg.h"
#include <conio.h>
#include <process.h>
#include <vector>
#include "voxel_file_format.h"
#include <float.h>
#include "CFileLoaderOBJ.h"
#include "CFileLoader3DS.h"
#include <algorithm>
#include "wmldistvec3tri3.h"
#include "meshImporter.h"
#include "mesh_data_structures.h"
#include "cImageLoader.h"

 // Turn off annoying compiler warnings
#pragma warning(disable: 4305) // stl
#pragma warning(disable: 4800) // stl
#pragma warning(disable: 4244) // numeric cast
#pragma warning(disable: 4996) // deprecation

#ifdef COMPILE_CPT
#pragma comment(lib,"libmingwex.a")
#pragma comment(lib,"libmingw32.a")
#pragma comment(lib,"libstdc++.a")
#pragma comment(lib,"libgcc.a")
#pragma comment(lib,"cpt.a")
#endif

// Should we default to the "hard shape" or just the cube?
// #define HARD_SHAPE

#ifndef M_PI
#define M_PI 3.1415926535898
#endif

struct outgoing_vertex_info {
	cVector3d pos;
	bool border;
};

typedef enum {
	// 0: Elements that aren't on the border (red)
	BORDER_TAG_NOT_ON_BORDER = 0,

	// 1: The initial element
	BORDERTAG_INITIAL_ELEMENT,

	// 2: I found that one of my neighbors was outside the
	//    range of valid neighbor indices (blue)
	BORDER_TAG_BAD_NEIGHBOR,

	// 3: I found that one of my neighbors was outside
	//    the surface by casting rays (green)
	BORDER_TAG_NEIGHBOR_COLLISION
};

/********

 CONSTANTS

 Sometimes these need to be tweaked to get a good filling...

*********/

#define INI_FILENAME "voxelizer.ini"
#define RESULTS_FILENAME "voxelizer_out.txt"

// If this is defined, the resulting voxels will be isotropic.  Otherwise
// the same number of voxels will be formed along each axis.
#define ISOTROPIC_VOXELS

#define NORMAL_SOURCE_TRIANGLE_FACE  0
#define NORMAL_SOURCE_VERTEX_NORMALS 1
#define NORMAL_SOURCE_NONE           2

#define NORMAL_SOURCE NORMAL_SOURCE_VERTEX_NORMALS

// The triangle from which flood-filling will start
#define DEFAULT_SEED_TRIANGLE 5

// How much to extend each ray to avoid "just barely" hitting triangles
#define COLLISION_OFFSET_FACTOR 0.00

#define ASCII_EXTENSION "txt"
#define BINARY_EXTENSION "voxels"
#define TETFILE_EXTENSION "tets"
#define ELEFILE_EXTENSION "ele"
#define NODEFILE_EXTENSION "node"
#define FACEFILE_EXTENSION "face"

#define DEFAULT_ROOT_FILENAME "default"

// Valid values: 5, 6
#define TETS_PER_CUBE 5

// Things that can be modified by the .ini file

// These were optimized by experiment...
int g_depth_first_search = 1;

// A value of -1 means "don't exploit spatial coherence"
int g_last_hit_steps = 0; // 2;

char g_filename_to_voxelize[_MAX_PATH];
int g_voxelize_immediately = 0;
int g_immediate_operation = OPERATION_VOXELIZE;

#define USE_PHANTOM_DIRECT_IO 1
// #define USE_MM_TIMER_FOR_HAPTICS

// Deduced roughly from:
//
// http://www.lec.leeds.ac.uk/~jason/Mesh-Maker/doc/MM-algorithm.html#Algorithm-initial-grid
static const int six_tet_cube_corners[6][4] =
{
  {1, 0, 5, 3},
  {4, 7, 6, 2},
  {0, 7, 3, 2},
  {0, 4, 7, 2},
  {0, 4, 5, 7},
  {0, 5, 3, 7}
};

// Deduced roughly from:
//
// http://arxiv.org/PS_cache/cs/pdf/0302/0302027.pdf
static const int five_tet_cube_corners[5][4] =
{
  {0, 1, 3, 5},
  {3, 5, 6, 7},
  {0, 2, 3, 6},
  {0, 4, 5, 6},
  {0, 3, 5, 6}
};


struct voxel_id {
	short i, j, k;
};

typedef enum {

	VOXELMARK_UNMARKED = 0, VOXELMARK_NOTINVOLUME,
	VOXELMARK_INVOLUME_TEXTURED, VOXELMARK_INVOLUME_UNTEXTURED

} voxel_mark_values;


class raw_triangle {

public:
	cVector3d v0;
	cVector3d v1;
	cVector3d v2;

	raw_triangle(cVector3d v0, cVector3d v1, cVector3d v2) {
		this->v0 = v0;
		this->v1 = v1;
		this->v2 = v2;
	}


	inline float compute_area() {

		// From http://mathworld.wolfram.com/TriangleArea.html
		// 
		// A = 0.5 * | u x v |

		cVector3d u = cSub(v1, v0);
		cVector3d v = cSub(v2, v0);

		return (float)(0.5f * (cCross(u, v).length()));
	}

};


struct ltvoxel
{
	bool operator()(const voxelfile_voxel& v1, const voxelfile_voxel& v2)
		const
	{
		if (v1.i < v2.i) return true;
		if (v2.i < v1.i) return false;

		if (v1.j < v2.j) return true;
		if (v2.j < v1.j) return false;

		if (v1.k < v2.k) return true;
		return false;
	}
};



#include <vector>
#include <set>

typedef std::vector<voxel_id> voxel_list;
typedef std::set<voxelfile_voxel, ltvoxel> voxel_set;

#define ALLOCATE_SCOPED_GLOBALS
#include "voxelizer_globals.h"
#undef ALLOCATE_SCOPED_GLOBALS

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CvoxelizerApp, CWinApp)
	//{{AFX_MSG_MAP(CvoxelizerApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// No one really knows why GetConsoleWindow() is not
// properly exported by the MS include files.
extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow();

CvoxelizerApp theApp;

void CvoxelizerApp::process_ini_file() {

	FILE* f = fopen(INI_FILENAME, "r");
	if (f == 0) {
		_cprintf("Could not open ini file %s\n", INI_FILENAME);
		return;
	}

	char buf[1000];

	while (1) {

		char* result = fgets(buf, 1000, f);
		if (result == 0) break;
		chop_newlines(buf);

		char* linestart = buf;
		while (*linestart == ' ' || *linestart == '\t') linestart++;
		if (*linestart == '#' || *linestart == '\n' || *linestart == '\0') continue;

		char* token = strtok(linestart, " ");

		if (strncmp(token, "LAST_HIT_STEPS", strlen("LAST_HIT_STEPS")) == 0) {
			token = strtok(0, " ");
			if (token == 0) {
				_cprintf("Could not read value for option LAST_HIT_STEPS\n");
			}
			int result = sscanf(token, "%d", &g_last_hit_steps);
			if (result == 0) {
				_cprintf("Could not read value for option LAST_HIT_STEPS\n");
			}
			else {
				_cprintf("Read value %d for option LAST_HIT_STEPS\n", g_last_hit_steps);
			}
			continue;
		}

		if (strncmp(token, "LONG_AXIS_RESOLUTION", strlen("LONG_AXIS_RESOLUTION")) == 0) {
			token = strtok(0, " ");
			if (token == 0) {
				_cprintf("Could not read value for option LONG_AXIS_RESOLUTION\n");
			}
			int result = sscanf(token, "%d", &m_long_axis_resolution);
			if (result == 0) {
				_cprintf("Could not read value for option LONG_AXIS_RESOLUTION\n");
			}
			else {
				_cprintf("Read value %d for option LONG_AXIS_RESOLUTION\n", m_long_axis_resolution);
			}
			continue;
		}

		if (strncmp(token, "COMPUTE_DISTANCE_FIELD", strlen("COMPUTE_DISTANCE_FIELD")) == 0) {
			token = strtok(0, " ");
			if (token == 0) {
				_cprintf("Could not read value for option COMPUTE_DISTANCE_FIELD\n");
			}
			int result = sscanf(token, "%d", &m_compute_distance_field);
			if (result == 0) {
				_cprintf("Could not read value for option COMPUTE_DISTANCE_FIELD\n");
			}
			else {
				_cprintf("Read value %d for option COMPUTE_DISTANCE_FIELD\n", m_compute_distance_field);
			}
			continue;
		}

		if (strncmp(token, "DEPTH_FIRST_SEARCH", strlen("DEPTH_FIRST_SEARCH")) == 0) {
			token = strtok(0, " ");
			if (token == 0) {
				_cprintf("Could not read value for option DEPTH_FIRST_SEARCH\n");
			}
			int result = sscanf(token, "%d", &g_depth_first_search);
			if (result == 0) {
				_cprintf("Could not read value for option DEPTH_FIRST_SEARCH\n");
			}
			else {
				_cprintf("Read value %d for option DEPTH_FIRST_SEARCH\n", g_depth_first_search);
			}
			continue;
		}

		if (strncmp(token, "VOXELIZE_IMMEDIATELY", strlen("VOXELIZE_IMMEDIATELY")) == 0) {
			token = strtok(0, " ");
			if (token == 0) {
				_cprintf("Could not read value for option VOXELIZE_IMMEDIATELY\n");
			}
			int result = sscanf(token, "%d", &g_voxelize_immediately);
			if (result == 0) {
				_cprintf("Could not read value for option VOXELIZE_IMMEDIATELY\n");
			}
			else {
				_cprintf("Read value %d for option VOXELIZE_IMMEDIATELY\n", g_voxelize_immediately);
			}
			continue;
		}

		if (strncmp(token, "WRITE_OUTPUT_FILE", strlen("WRITE_OUTPUT_FILE")) == 0) {
			token = strtok(0, " ");
			if (token == 0) {
				_cprintf("Could not read value for option WRITE_OUTPUT_FILE\n");
			}
			int result = sscanf(token, "%d", &m_write_output_file);
			if (result == 0) {
				_cprintf("Could not read value for option WRITE_OUTPUT_FILE\n");
			}
			else {
				_cprintf("Read value %d for option WRITE_OUTPUT_FILE\n", m_write_output_file);
			}
			continue;
		}

		if (strncmp(token, "IMMEDIATE_OPERATION", strlen("IMMEDIATE_OPERATION")) == 0) {
			token = strtok(0, " ");
			if (token == 0) {
				_cprintf("Could not read value for option IMMEDIATE_OPERATION\n");
			}
			int result = sscanf(token, "%d", &g_immediate_operation);
			if (result == 0) {
				_cprintf("Could not read value for option IMMEDIATE_OPERATION\n");
			}
			else {
				_cprintf("Read value %d for option IMMEDIATE_OPERATION\n", g_immediate_operation);
			}
			continue;
		}

		if (strncmp(token, "SEED_TRIANGLE", strlen("SEED_TRIANGLE")) == 0) {
			token = strtok(0, " ");
			if (token == 0) {
				_cprintf("Could not read value for option SEED_TRIANGLE\n");
			}
			int result = sscanf(token, "%d", &seed_triangle_index);
			if (result == 0) {
				_cprintf("Could not read value for option SEED_TRIANGLE\n");
			}
			else {
				_cprintf("Read value %d for option SEED_TRIANGLE\n", seed_triangle_index);
			}
			continue;
		}

		if (strncmp(token, "OBJECT_TO_VOXELIZE", strlen("OBJECT_TO_VOXELIZE")) == 0) {
			token = strtok(0, " ");
			if (token == 0) {
				_cprintf("Could not read value for option OBJECT_TO_VOXELIZE\n");
			}
			strcpy(g_filename_to_voxelize, token);
			_cprintf("Read value %s for option OBJECT_TO_VOXELIZE\n", g_filename_to_voxelize);
			continue;
		}

		_cprintf("Unrecognized token: %s\n", token);

	} // for each line in the file

	fclose(f);

	update_gui_from_options();
}


void CvoxelizerApp::write_results_file() {

	FILE* f = fopen(RESULTS_FILENAME, "w");
	if (f == 0) {
		_cprintf("Could not open results file %s\n", RESULTS_FILENAME);
		return;
	}

	fprintf(f, "# xres yres zres total_time distance_time voxels triangles tets_produced\n");
	fprintf(f, "%d %d %d %lf %lf %d %d %d\n",
		voxel_resolution[0], voxel_resolution[1], voxel_resolution[2],
		total_time, distance_time, found_voxels, object_to_voxelize->getNumTriangles(true), m_nTets);
	fprintf(f, "%s\n", g_filename_to_voxelize);

	fclose(f);
}

CvoxelizerApp::CvoxelizerApp() {

	m_write_output_file = 1;
	m_compute_distance_field = 0;
	seed_triangle_index = DEFAULT_SEED_TRIANGLE;
	g_filename_to_voxelize[0] = '\0';
	multithreaded_voxelizer = 0;
	render_point_cloud = 0;

	total_time = 0.0;
	distance_time = 0.0;

	m_long_axis_resolution = DEFAULT_VOXEL_RESOLUTION;
	m_normal_multiplier = DEFAULT_NORMAL_MULTIPLIER;

#ifdef COMPILE_CPT
	m_distance_array = 0;
	m_closest_point_array = 0;
	m_gradient_array = 0;
	m_closest_face_array = 0;
#endif

	current_texture[0] = '\0';

	cloud = new CPointCloud();

	object_to_voxelize = 0;

	strcpy(output_filename_root, DEFAULT_ROOT_FILENAME);

	current_voxelization_point = new cGenericObject();
	current_voxelization_point->setPos(0, 0, 0);
	current_voxelization_point->setShowFrame(1, 0);
	current_voxelization_point->setFrameSize(1.0);

	selected_object = 0;
	selected_tri = 0;

	haptics_enabled = 0;

	AllocConsole();

	HWND con_wnd = GetConsoleWindow();

	// We want the console to pop up over any non-TG2 windows, although
	// we'll put the main dialog window above it.
	::SetForegroundWindow(con_wnd);

	SetWindowPos(con_wnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);

	g_main_app = this;

	model_offset.set(0, 0, 0);
	model_scale_factor = 1.0;

}


void CvoxelizerApp::uninitialize() {

	toggle_haptics(TOGGLE_HAPTICS_DISABLE);
	world->removeChild(this);
	delete world;
	delete viewport;

}

// A function to quickly stick a cube in the given mesh
void createCube(cMesh *mesh, float edge);
void createChallengingShape(cMesh *mesh, float edge);


BOOL CvoxelizerApp::InitInstance() {

	AfxEnableControlContainer();

	g_main_dlg = new CvoxelizerDlg;
	m_pMainWnd = g_main_dlg;

	g_main_dlg->Create(IDD_voxelizer_DIALOG, NULL);

	// Now we should have a display context to work with...

	world = new cWorld();
	world->addChild(this);

	// set background color
	world->setBackgroundColor(0.0f, 0.0f, 0.0f);

	cLight* light = new cLight(world);

	// Turn on one light...
	light->setEnabled(true);
	light->setPos(cVector3d(240, 200, 200));
	light->setDir(cVector3d(-1.4, -1.0, -1.0));

	// Create a camera
	camera = new cCamera(world);
	world->addChild(camera);

	// set camera position and orientation, preferably looking down
	// the negative z axis
	int result = camera->set(cVector3d(0, 0, 4.0),   // position of camera
		cVector3d(0.0, 0.0, 0.0),   // camera looking at origin
		cVector3d(0.0, 1.0, 0.0));  // orientation of camera (standing up)

	// Create a display for graphic rendering
	viewport = new cViewport(g_main_dlg->m_gl_area_hwnd, camera, false);

	// Create a mesh - we will build a pyramid manually, and later let the
	// user load 3d models
	object_to_voxelize = new cMesh(world);
	world->addChild(object_to_voxelize);

	float cube_size = 1.0;

#ifdef HARD_SHAPE  
	createChallengingShape(object_to_voxelize, cube_size);
#else
	// Create a nice little cube
	createCube(object_to_voxelize, cube_size);
#endif

	// Set object settings.  The parameters tell the object
	// to apply this alpha level to his textures and to his
	// children (of course he has neither right now).
	// object_to_voxelize->setTransparencyLevel(0.5, true, true);

	// Give him some material properties...
	cMaterial material;

	// This sets the _haptic_ stiffness of the material
	material.setStiffness(0.0);

	material.m_ambient.set(0.4, 0.2, 0.2, 1.0);
	material.m_diffuse.set(0.8, 0.6, 0.6, 1.0);
	material.m_specular.set(0.9, 0.9, 0.9, 1.0);
	material.setShininess(100);
	object_to_voxelize->m_material = material;

	// These two calls are useful for debugging, but not
	// really important...

	// Tells the object to compute a boundary box around
	// himself and all his children.  We do this here just so
	// he can render his bounding box for your viewing
	// pleasure.
	object_to_voxelize->computeBoundaryBox(true);

	// Set the size of the coordinate frame that we're
	// going to gratuitously render
	object_to_voxelize->setFrameSize(1.0, 1.0, false);

	object_to_voxelize->computeGlobalPositions();

	object_to_voxelize->createAABBCollisionDetector(true, false);

	// Operations I do during debugging only

	// object_to_voxelize->createSphereTreeCollisionDetector(true,true);
	// object_to_voxelize->showCollisionTree(1,1);
	// object_to_voxelize->computeAllNormals();

	update_options_from_gui();

	return TRUE;
}


int CvoxelizerApp::Run() {

	MSG msg;

	// Position the console window so it looks nice...
	HWND con_wnd = GetConsoleWindow();
	RECT r;
	::GetWindowRect(con_wnd, &r);

	// Move the real window up to the front
	::SetWindowPos(g_main_dlg->m_hWnd, HWND_TOP, 0, r.bottom, 0, 0, SWP_NOSIZE);
	::SetForegroundWindow(g_main_dlg->m_hWnd);

	//LoadObjectToVoxelize("C:\\users\\dmorris\\voxelizer\\bin\\ape_capped.3ds");
	//voxelize_current_object();
	// OPERATION_TESTONLY

	// Open voxel .ini file
	process_ini_file();

	int loadresult = -1;
	if (g_filename_to_voxelize[0]) {
		loadresult = LoadObjectToVoxelize(g_filename_to_voxelize);
	}

	// If we're supposed to process a file immediately 
	if (loadresult >= 0 && g_voxelize_immediately) {
		voxelize_current_object(g_immediate_operation);
		write_results_file();
		// _getch();
		exit(0);
	}

	// Loop forever looking for messages and rendering...
	while (1) {

		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) == TRUE) {

			if (GetMessage(&msg, NULL, 0, 0)) {

				int message_processed = PreTranslateMessage(&msg);

				if (message_processed == 0) TranslateMessage(&msg);

				if (msg.message == WM_KEYDOWN) {

				}

				else if (msg.message == WM_KEYUP) {

				}

				if (message_processed == 0) DispatchMessage(&msg);

			}

			// Quit if GetMessage(...) fails
			else return TRUE;

		}

		render_loop();

		// Sleep to be extra well-behaved, not necessary for
		// a game-like app
		// Sleep(1);

	}

} // Run()


int CvoxelizerApp::render_loop() {

	// Print the distance from the haptic probe to a modifier object
	/*
	if (tool && modifier_object) {
	  cVector3d p = tool->m_deviceGlobalPos; // tool->getPos();
	  cVector3d closest_point;
	  double d = AABB_distance_computer::closest_vertex(
		modifier_object,p,0,&closest_point,1);
	  _cprintf("Distance from (%3.3f,%3.3f,%3.3f) to (%3.3f,%3.3f,%3.3f) is %lf\n",
		(float)p.x,(float)p.y,(float)p.z,
		(float)closest_point.x,(float)closest_point.y,(float)closest_point.z,
		d);
	}*/

	viewport->render();

	return 0;

}

void CvoxelizerApp::update_gui_from_options() {
	g_main_dlg->m_voxelres_edit.Set_Number(m_long_axis_resolution);
	g_main_dlg->m_distance_check = m_compute_distance_field;
	g_main_dlg->UpdateData(false);
}

void CvoxelizerApp::update_options_from_gui() {

	// The second parameter in each case transfers options to
	// any children the object has...

	if (object_to_voxelize) {
		object_to_voxelize->setWireMode(g_main_dlg->m_usewireframe_check, true);
		object_to_voxelize->setShowFrame(g_main_dlg->m_showframe_check, false);
		object_to_voxelize->showNormals(g_main_dlg->m_shownormals_check, true);

		// object_to_voxelize->setShowBox(g_main_dlg->m_showbox_check, true);
		object_to_voxelize->showCollisionTree(g_main_dlg->m_showbox_check, true);

		object_to_voxelize->getCollisionDetector()->setDisplayDepth(-5);

		object_to_voxelize->useTexture(g_main_dlg->m_usetexture_check, true);
		object_to_voxelize->useColors(g_main_dlg->m_usecolors_check, true);
		object_to_voxelize->useMaterial(g_main_dlg->m_material_check, true);
		object_to_voxelize->setShow(g_main_dlg->m_rendermesh_check, true);
		if (cloud) cloud->setShow(true, true);
	}

	if (objects_to_subtract.size() > 0) {

		for (unsigned int i = 0; i < objects_to_subtract.size(); i++) {
			cMesh* object_to_subtract = objects_to_subtract[i];
			object_to_subtract->setWireMode(g_main_dlg->m_usewireframe_check, true);
			object_to_subtract->setShowFrame(g_main_dlg->m_showframe_check, false);
			object_to_subtract->showNormals(g_main_dlg->m_shownormals_check, true);
			object_to_subtract->setShowBox(g_main_dlg->m_showbox_check, true);
			object_to_subtract->useTexture(g_main_dlg->m_usetexture_check, true);
			object_to_subtract->useColors(g_main_dlg->m_usecolors_check, true);
			object_to_subtract->useMaterial(g_main_dlg->m_material_check, true);
		}
	}

	if (modifier_objects.size() > 0) {

		for (unsigned int i = 0; i < modifier_objects.size(); i++) {
			cMesh* modifier_object = modifier_objects[i];
			modifier_object->setWireMode(g_main_dlg->m_usewireframe_check, true);
			modifier_object->setShowFrame(g_main_dlg->m_showframe_check, false);
			modifier_object->showNormals(g_main_dlg->m_shownormals_check, true);
			modifier_object->setShowBox(g_main_dlg->m_showbox_check, true);
			modifier_object->useTexture(g_main_dlg->m_usetexture_check, true);
			modifier_object->useColors(g_main_dlg->m_usecolors_check, true);
			modifier_object->useMaterial(g_main_dlg->m_material_check, true);
		}
	}

	world->setShowFrame(g_main_dlg->m_showframe_check, false);

}


// A function used to pop up a dialog box and ask the user to select
// a file (for selecting game files) (last five parameters are optional).
int FileBrowse(char* buffer, int length, int save, char* forceExtension,
	char* extension_string, char* title) {

	CFileDialog cfd(1 - save, 0, 0, 0, extension_string, g_main_dlg);

	if (forceExtension != 0) {
		cfd.m_ofn.lpstrDefExt = forceExtension;
	}

	if (title != 0) {
		cfd.m_ofn.lpstrTitle = title;
	}

	if (save) {
		cfd.m_ofn.Flags |= OFN_OVERWRITEPROMPT;
	}

	// We don't want to change the working path
	cfd.m_ofn.Flags |= OFN_NOCHANGEDIR;

	if (cfd.DoModal() != IDOK) return -1;

	CString result = cfd.GetPathName();

	// This should never happen, since I always pass in _MAX_PATH
	// for the length.
	if (result.GetLength() > length) {
		_cprintf("Warning: truncating filename to %d characters\n", length);
	}

	strncpy(buffer, (LPCSTR)(result), length);

	// strncpy needs help with the null termination
	buffer[length - 1] = '\0';

	return 0;
}


int CvoxelizerApp::LoadObjectToVoxelize(char* filename) {

	cMesh* new_mesh = LoadModel(filename, 1);

	if (new_mesh == 0) return -1;

	if (object_to_voxelize) {

		bool result1, result2;
		result1 = object_to_voxelize->removeChild(current_voxelization_point);
		result2 = object_to_voxelize->removeChild(cloud);

		world->removeChild(object_to_voxelize);
		delete object_to_voxelize;

		selected_object = 0;
		selected_tri = 0;

	}

	strcpy(output_filename_root, filename);
	_cprintf("Filename root is %s\n", output_filename_root);

	object_to_voxelize = new_mesh;
	world->addChild(object_to_voxelize);

	char* str = new char[strlen(filename) + 1];
	strcpy(str, filename);
	object_to_voxelize->m_userData = str;

	update_options_from_gui();

	return 0;

}


int CvoxelizerApp::LoadObjectToSubtract(char* filename) {

	cMesh* new_mesh = LoadModel(filename, 0);
	if (new_mesh == 0) return -1;

	cMesh* object_to_subtract = new_mesh;

	char* str = new char[strlen(filename) + 1];
	find_filename(str, filename, true);
	object_to_subtract->m_userData = str;

	objects_to_subtract.push_back(object_to_subtract);
	world->addChild(object_to_subtract);
	object_to_subtract->computeGlobalPositions(false);

	update_options_from_gui();

	return 0;
}


int CvoxelizerApp::LoadModifierObject(char* filename) {

	cMesh* new_mesh = LoadModel(filename, 0);
	if (new_mesh == 0) return -1;

	cMesh* modifier_object = new_mesh;

	modifier_objects.push_back(modifier_object);
	world->addChild(modifier_object);
	modifier_object->computeGlobalPositions(false);

	char* str = new char[strlen(filename) + 1];
	find_filename(str, filename, true);
	modifier_object->m_userData = str;

	update_options_from_gui();

	return 0;
}


cMesh* CvoxelizerApp::LoadModel(char* filename, int compute_new_offsets) {

	transform_operation transform_to_apply;
	mesh_xform_information xform_data;

	if (compute_new_offsets) {
		transform_to_apply = XFORMOP_AUTO;
	}
	else {
		transform_to_apply = XFORMOP_USESUPPLIED;
		xform_data.model_offset = model_offset;
		xform_data.model_scale_factor = model_scale_factor;
		_cprintf("Transforming by %s,%f\n", model_offset.str(2).c_str(), model_scale_factor);
	}
	cMesh* m;
	cLabelPanel* lp = 0;
	bool result = importModel(filename, m, world, true, true, 0, 0,
		transform_to_apply, &xform_data);

	// Remember the transformations that were applied if we're computing
	// new offsets
	if (compute_new_offsets == 1) {
		_cprintf("Transformed by %s,%f\n", model_offset.str(2).c_str(), model_scale_factor);
		model_offset = xform_data.model_offset;
		model_scale_factor = xform_data.model_scale_factor;
	}

	if (lp) delete lp;
	if (result == false) return 0;

	return m;
}


int CvoxelizerApp::LoadTexture(char* filename) {

	cTexture2D *newTexture = world->newTexture();
	int result = newTexture->loadFromFile(filename);

	if (result == 0) {
		_cprintf("Could not load texture file %s\n", filename);
		delete newTexture;
		return -1;
	}

	object_to_voxelize->setTexture(newTexture, 1);
	object_to_voxelize->useTexture(1, 1);

	char buf[_MAX_PATH];
	char* final_filename;
	GetFullPathName(filename, _MAX_PATH, buf, &final_filename);
	strcpy(current_texture, final_filename);

	return 0;
}

void CvoxelizerApp::zoom(int zoom_level) {

	//camera->setPos(p.x,((float)(zoom_level))/100.0*10.0,p.z);
	camera->setFieldViewAngle(((float)(zoom_level)));

}


void CvoxelizerApp::scroll(CPoint p, int button) {

	int shift_pressed = GetKeyState(VK_SHIFT) & (1 << 15);

	// Get the current camera vectors
	cVector3d up = camera->getUpVector();
	cVector3d right = camera->getRightVector();
	cVector3d look = camera->getLookVector();

	if (button == MOUSE_BUTTON_RIGHT) {

		cVector3d motion;

		if (shift_pressed) {
			// In/out motion based on vertical mouse movement
			motion = ((double)p.y) / 50.0 * look +
				((double)p.x) / -50.0 * right;
		}
		else {
			// Up/down motion based on vertical mouse movement
			motion = ((double)p.y) / 50.0 * up +
				((double)p.x) / -50.0 * right;
		}
		camera->translate(motion);
		world->computeGlobalPositions(true);
		return;
	}

	// Middle button
	if (button == MOUSE_BUTTON_MIDDLE) {

		cMatrix3d rot;
		rot.identity();

		// Map horizontal mouse motion to motion around the up vector
		rot.rotate(up, (float)p.x / 300.0);

		// Map vertical mouse motion to motion around the right vector
		rot.rotate(right, (float)p.y / 300.0);

		// Build a new rotation matrix for the camera
		cVector3d new_look = rot * look;
		cVector3d new_up = rot * up;
		new_look.normalize();
		new_up.normalize();
		cVector3d new_right = cCross(new_up, new_look);

		rot.setCol0(new_look);
		rot.setCol1(new_right);
		rot.setCol2(new_up);

		camera->setRot(rot);
		world->computeGlobalPositions(true);
		return;
	}

	if (selected_object == 0) return;

	// If the user hasn't clicked on any objects, we don't
	// have to move or rotate anyone
	cGenericObject* object_to_move = selected_object;

	// We want to move the _parent_ mesh around, so let's
	// walk up the tree, looking for the highest-level object
	// that's still a cMesh...
	//
	// The reason we do this is that maybe I've loaded, say,
	// a model of a person from a file, that might contain
	// separate meshes for each arm.  If I click on that mesh,
	// CHAI will indeed find that mesh, but really I don't
	// want the user to pull the model's arm off, I want to move
	// the larger model around.
	cGenericObject* tmp = object_to_move;

	// Walk up the tree...
	while (1) {

		tmp = tmp->getParent();

		// If there was no parent, we hit the highest-level mesh
		if (tmp == 0) break;

		// If the parent was not a cMesh, we hit the highest-level mesh
		cMesh* mesh = dynamic_cast<cMesh*>(tmp);
		if (mesh == 0) break;

		// Okay, this is a mesh...
		object_to_move = tmp;

	}

	if (button == MOUSE_BUTTON_LEFT) {

		if (shift_pressed) {
			cVector3d translation_vector =
				(-1.0*(float)p.y / 100.0) * camera->getUpVector() +
				(1.0*(float)p.x / 100.0) * camera->getRightVector();
			object_to_move->translate(translation_vector);
		}

		else {

			// These vectors come from the (unusual) definition of the CHAI
			// camera's rotation matrix:
			//
			// column 0: look
			// column 2: up
			// column 1: look x up

			// Rotation around the horizontal camera axis
			cVector3d axis1(0, 1, 0);
			camera->getRot().mul(axis1);
			object_to_move->rotate(axis1, 1.0*(float)p.y / 50.0);

			// Rotation around the vertical camera axis
			cVector3d axis2(0, 0, 1);
			camera->getRot().mul(axis2);
			object_to_move->rotate(axis2, (float)p.x / 50.0);
		}

		object_to_move->computeGlobalPositions(true);
		// object_to_move->computeBoundaryBox(true);    
	}

}


void CvoxelizerApp::select(CPoint p) {

	if (viewport->select(p.x, p.y, false)) {
		selected_object = viewport->getLastSelectedObject();
		selected_tri = viewport->getLastSelectedTriangle();
		_cprintf("Selected triangle %d\n", selected_tri);
	}
	else {
		selected_object = 0;
		selected_tri = 0;
	}

}

// Our haptic loop... just computes forces on the 
// phantom every iteration, until haptics are disabled
// in the supplied CvoxelizerApp

// A single iteration through the loop...
void voxelizer_haptic_iteration(void* param) {

	CvoxelizerApp* app = (CvoxelizerApp*)(param);

	app->tool->updatePose();
	app->tool->computeForces();
	app->tool->applyForces();

}


// This loop is used only in the threaded version of this
// application...
DWORD voxelizer_haptic_loop(void* param) {

	CvoxelizerApp* app = (CvoxelizerApp*)(param);

	while (app->haptics_enabled) {

		voxelizer_haptic_iteration(param);

	}

	app->haptics_thread_running = 0;

	return 0;
}


void CvoxelizerApp::toggle_haptics(int enable) {

	if (enable == TOGGLE_HAPTICS_TOGGLE) {

		if (haptics_enabled) toggle_haptics(TOGGLE_HAPTICS_DISABLE);
		else toggle_haptics(TOGGLE_HAPTICS_ENABLE);

	}

	else if (enable == TOGGLE_HAPTICS_ENABLE) {

		if (haptics_enabled) return;

		haptics_enabled = 1;

		// create a phantom tool with its graphical representation
		//
		// Use device zero, and use either the gstEffect or direct 
		// i/o communication mode, depending on the USE_PHANTOM_DIRECT_IO
		// constant
		if (tool == 0) {

			tool = new cMeta3dofPointer(world, 0, USE_PHANTOM_DIRECT_IO);

			// The tool is rendered as a child of the camera.
			//
			// This has the nice property of always aligning the Phantom's
			// axes with the camera's axes.

			// camera ->addChild(tool);

			// Actually, that doesn't work well... need to sort out why cd 
			// doesn't operate in the right frame if the tool is somebody's
			// child.
			world->addChild(tool);

			// set up a nice-looking workspace for the phantom so 
			// it fits nicely with our models
			tool->setWorkspace(2.0, 2.0, 2.0);

			// Rotate the tool so its axes align with our opengl-like axes
			tool->rotate(cVector3d(0, 0, 1), -90.0*M_PI / 180.0);
			tool->rotate(cVector3d(1, 0, 0), -90.0*M_PI / 180.0);
			tool->setRadius(0.05);

		}

		// set up the device
		tool->initialize();

		// open communication to the device
		tool->start();

		// update initial orientation and position of device
		tool->updatePose();

		// tell the tool to show his coordinate frame so you
		// can see tool rotation
		tool->visualizeFrames(true);

		// I need to call this so the tool can update its internal
		// transformations before performing collision detection, etc.
		tool->computeGlobalPositions();
		tool->setForcesON();

		// Enable the "dynamic proxy", which will handle moving objects
		cProxyPointForceAlgo* proxy = tool->getProxy();

		proxy->enableDynamicProxy(1);

#ifdef USE_MM_TIMER_FOR_HAPTICS

		// start the mm timer to run the haptic loop
		timer.set(0, voxelizer_haptic_iteration, this);

#else

		// start haptic thread
		haptics_thread_running = 1;

		DWORD thread_id;
		::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)(voxelizer_haptic_loop), this, 0, &thread_id);

		// Boost thread and process priority
		::SetThreadPriority(&thread_id, THREAD_PRIORITY_ABOVE_NORMAL);
		//::SetPriorityClass(GetCurrentProcess(),ABOVE_NORMAL_PRIORITY_CLASS);

#endif

	} // enabling

	else if (enable == TOGGLE_HAPTICS_DISABLE) {

		if (haptics_enabled == 0) return;

		// tell the haptic thread to quit
		haptics_enabled = 0;

#ifdef USE_MM_TIMER_FOR_HAPTICS

		timer.stop();

#else

		// wait for the haptic thread to quit
		while (haptics_thread_running) Sleep(1);

#endif

		// Stop the haptic device...
		tool->setForcesOFF();
		tool->stop();

		// SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);    

	} // disabling

} // toggle_haptics()


// A global function for sticking a cube in the given mesh
// 
// Manually creates the 12 triangles (two per face) required to
// model a cube
void createCube(cMesh *mesh, float edge) {

	// I define the cube's "radius" to be half the edge size
	float radius = edge / 2.0f;
	int n;
	int cur_index = 0;
	int start_index = 0;

	// +x face
	mesh->newVertex(radius, radius, -radius);
	mesh->newVertex(radius, radius, radius);
	mesh->newVertex(radius, -radius, -radius);
	mesh->newTriangle(cur_index, cur_index + 1, cur_index + 2);
	cur_index += 3;

	mesh->newVertex(radius, -radius, -radius);
	mesh->newVertex(radius, radius, radius);
	mesh->newVertex(radius, -radius, radius);
	mesh->newTriangle(cur_index, cur_index + 1, cur_index + 2);
	cur_index += 3;

	for (n = start_index; n < cur_index; n++) {
		cVertex* curVertex = mesh->getVertex(n);
		curVertex->setTexCoord(
			(curVertex->getPos().y + radius) / (2.0 * radius),
			(curVertex->getPos().z + radius) / (2.0 * radius)
		);
		curVertex->setNormal(1, 0, 0);
	}

	start_index += 6;

	// -x face
	mesh->newVertex(-radius, radius, radius);
	mesh->newVertex(-radius, radius, -radius);
	mesh->newVertex(-radius, -radius, -radius);
	mesh->newTriangle(cur_index, cur_index + 1, cur_index + 2);
	cur_index += 3;

	mesh->newVertex(-radius, radius, radius);
	mesh->newVertex(-radius, -radius, -radius);
	mesh->newVertex(-radius, -radius, radius);
	mesh->newTriangle(cur_index, cur_index + 1, cur_index + 2);
	cur_index += 3;

	for (n = start_index; n < cur_index; n++) {
		cVertex* curVertex = mesh->getVertex(n);
		curVertex->setTexCoord(
			(curVertex->getPos().y + radius) / (2.0 * radius),
			(curVertex->getPos().z + radius) / (2.0 * radius)
		);
		curVertex->setNormal(-1, 0, 0);
	}

	start_index += 6;

	// +y face
	mesh->newVertex(radius, radius, radius);
	mesh->newVertex(radius, radius, -radius);
	mesh->newVertex(-radius, radius, -radius);
	mesh->newTriangle(cur_index, cur_index + 1, cur_index + 2);
	cur_index += 3;

	mesh->newVertex(radius, radius, radius);
	mesh->newVertex(-radius, radius, -radius);
	mesh->newVertex(-radius, radius, radius);
	mesh->newTriangle(cur_index, cur_index + 1, cur_index + 2);
	cur_index += 3;

	for (n = start_index; n < cur_index; n++) {
		cVertex* curVertex = mesh->getVertex(n);
		curVertex->setTexCoord(
			(curVertex->getPos().x + radius) / (2.0 * radius),
			(curVertex->getPos().z + radius) / (2.0 * radius)
		);
		curVertex->setNormal(0, 1, 0);
	}

	start_index += 6;

	// +y face
	mesh->newVertex(radius, -radius, radius);
	mesh->newVertex(-radius, -radius, -radius);
	mesh->newVertex(radius, -radius, -radius);
	mesh->newTriangle(cur_index, cur_index + 1, cur_index + 2);
	cur_index += 3;

	mesh->newVertex(-radius, -radius, -radius);
	mesh->newVertex(radius, -radius, radius);
	mesh->newVertex(-radius, -radius, radius);
	mesh->newTriangle(cur_index, cur_index + 1, cur_index + 2);
	cur_index += 3;

	for (n = start_index; n < cur_index; n++) {
		cVertex* curVertex = mesh->getVertex(n);
		curVertex->setTexCoord(
			(curVertex->getPos().x + radius) / (2.0 * radius),
			(curVertex->getPos().z + radius) / (2.0 * radius)
		);
		curVertex->setNormal(0, -1, 0);
	}

	start_index += 6;

	// -z face
	mesh->newVertex(-radius, -radius, -radius);
	mesh->newVertex(radius, radius, -radius);
	mesh->newVertex(radius, -radius, -radius);
	mesh->newTriangle(cur_index, cur_index + 1, cur_index + 2);
	cur_index += 3;

	mesh->newVertex(radius, radius, -radius);
	mesh->newVertex(-radius, -radius, -radius);
	mesh->newVertex(-radius, radius, -radius);
	mesh->newTriangle(cur_index, cur_index + 1, cur_index + 2);
	cur_index += 3;

	for (n = start_index; n < cur_index; n++) {
		cVertex* curVertex = mesh->getVertex(n);
		curVertex->setTexCoord(
			(curVertex->getPos().x + radius) / (2.0 * radius),
			(curVertex->getPos().y + radius) / (2.0 * radius)
		);
		curVertex->setNormal(0, 0, -1);
	}

	start_index += 6;

	// +z face
	mesh->newVertex(-radius, -radius, radius);
	mesh->newVertex(radius, -radius, radius);
	mesh->newVertex(radius, radius, radius);
	mesh->newTriangle(cur_index, cur_index + 1, cur_index + 2);
	cur_index += 3;

	mesh->newVertex(-radius, -radius, radius);
	mesh->newVertex(radius, radius, radius);
	mesh->newVertex(-radius, radius, radius);
	mesh->newTriangle(cur_index, cur_index + 1, cur_index + 2);
	cur_index += 3;

	for (n = start_index; n < cur_index; n++) {
		cVertex* curVertex = mesh->getVertex(n);
		curVertex->setTexCoord(
			(curVertex->getPos().x + radius) / (2.0 * radius),
			(curVertex->getPos().y + radius) / (2.0 * radius)
		);
		curVertex->setNormal(0, 0, 1);
	}

	start_index += 6;

	mesh->computeAllNormals(true);
}


void createChallengingShape(cMesh *mesh, float edge) {

	unsigned int i;

	edge /= 2.0;

	mesh->newVertex(-edge, -edge, edge);
	mesh->newVertex(edge, -edge, edge);
	mesh->newVertex(edge, edge, edge);
	mesh->newVertex(-edge, edge, edge);
	mesh->newVertex(edge, -edge, -edge);
	mesh->newVertex(-edge*0., -edge*0., -edge*0.);
	mesh->newVertex(-edge, edge, -edge);
	mesh->newVertex(edge, edge, -edge);

	mesh->newTriangle(0, 1, 2);
	mesh->newTriangle(0, 2, 3);
	mesh->newTriangle(4, 5, 6);
	mesh->newTriangle(4, 6, 7);
	mesh->newTriangle(5, 0, 3);
	mesh->newTriangle(5, 3, 6);
	mesh->newTriangle(1, 4, 7);
	mesh->newTriangle(1, 7, 2);
	mesh->newTriangle(5, 4, 1);
	mesh->newTriangle(5, 1, 0);
	mesh->newTriangle(3, 2, 7);
	mesh->newTriangle(3, 7, 6);

	for (i = 0; i < mesh->getNumVertices(); i++) {
		cVertex* v = mesh->getVertex(i);
		float c1, c2;

		c1 = v->getPos().x;
		c2 = v->getPos().y;

		v->setTexCoord(
			(v->getPos().x + edge) / (2.0*edge),
			(c2 + edge) / (2.0*edge)
		);
	}

	for (i = 0; i < mesh->getNumTriangles(); i++) {
		cTriangle* tri = mesh->getTriangle(i);
		tri->m_tag = i;
	}

	// Give an arbitrary color to each vertex
	for (i = 0; i < mesh->getNumVertices(); i++) {

		cVertex* nextVertex = mesh->getVertex(i);

		cColorb color;
		color.set(
			GLuint(0xff * (edge + nextVertex->getPos().x) / (2.0 * edge)),
			GLuint(0xff * (edge + nextVertex->getPos().y) / (2.0 * edge)),
			GLuint(0xff * nextVertex->getPos().z / 2 * edge)
		);

		nextVertex->setColor(color);
	}

	mesh->computeAllNormals(true);
}


#define VECTOR_EQUAL_EPSILON 0.0001

/****
 Note: this is not a stable operator.  It works because
 I know my points won't be all that close relative to
 VECTOR_EQUAL_EPSILON.
****/
struct lt_cVector3d {
	inline bool operator()(const cVector3d& v1, const cVector3d& v2) const
	{

		// First check for approximate equality
		if (v1.equals(v2, VECTOR_EQUAL_EPSILON)) return false;

		// Now sort
		if (v1[0] < v2[0]) return true;
		else if (v1[0] > v2[0]) return false;
		else if (v1[1] < v2[1]) return true;
		else if (v1[1] > v2[1]) return false;
		else if (v1[2] < v2[2]) return true;
		else return false;
	}
};


struct lt_outgoing_vertex_info {
	inline bool operator()(const outgoing_vertex_info& v1, const outgoing_vertex_info& v2) const
	{
		lt_cVector3d compare;
		return compare(v1.pos, v2.pos);
	}
};


inline int voxel_index(const int* voxel_resolution, const voxel_id& id) {

	return
		id.i * (voxel_resolution[1] * voxel_resolution[2]) +
		id.j*voxel_resolution[2] +
		id.k;
}


inline voxel_id voxel_coords(const int* voxel_resolution, unsigned int index) {

	voxel_id id;

	id.k = index % voxel_resolution[2];
	index /= voxel_resolution[2];
	id.j = index % voxel_resolution[1];
	index /= voxel_resolution[1];
	id.i = index;

	return id;

}


inline void compute_voxel_coordinate(const voxel_id& id, cVector3d& coord,
	const cVector3d& voxel_size,
	const cVector3d& voxel_start_offset) {

	coord.set(
		((float)id.i)*voxel_size.x + voxel_size.x / 2.0,
		((float)id.j)*voxel_size.y + voxel_size.y / 2.0,
		((float)id.k)*voxel_size.z + voxel_size.z / 2.0
	);

	coord.add(voxel_start_offset);
}


inline voxel_id compute_voxel_id(const cVector3d& coord,
	const cVector3d& voxel_size,
	const cVector3d& voxel_start_offset) {

	voxel_id id;
	cVector3d p = coord;
	p.sub(voxel_start_offset);
	id.i = (coord.x - voxel_size.x / 2.0) / voxel_size.x;
	id.j = (coord.y - voxel_size.y / 2.0) / voxel_size.y;
	id.k = (coord.z - voxel_size.z / 2.0) / voxel_size.z;

	return id;
}

double g_smallest_distance = DBL_MAX;
double g_largest_distance = -DBL_MAX;

void CvoxelizerApp::voxelize_current_object(int operation) {

	m_nTets = 0;

	g_smallest_distance = DBL_MAX;
	g_largest_distance = -DBL_MAX;

	if (object_to_voxelize->containsChild(current_voxelization_point, true) == 0)
		object_to_voxelize->addChild(current_voxelization_point);
	if (object_to_voxelize->containsChild(cloud, true) == 0)
		object_to_voxelize->addChild(cloud);

	WaitForSingleObject(cloud->point_mutex, INFINITE);
	cloud->points.clear();
	ReleaseMutex(cloud->point_mutex);

	// Align the object with the world-space axes, to avoid rotation issues
	cMatrix3d new_rot;
	new_rot.identity();

	// Make sure the object doesn't have any rotation applied to
	// it...
	object_to_voxelize->setRot(new_rot);

	// Re-center the object, in case the user moved it...
	object_to_voxelize->setPos(0, 0, 0);

	// Find the boundary box of the object
	cVector3d object_min = object_to_voxelize->getBoundaryMin();
	cVector3d object_max = object_to_voxelize->getBoundaryMax();
	cVector3d span = cSub(object_max, object_min);

	// Increase the effective span by just a little, to make sure we leave room
	// for all voxels...
  // TODO: this is undefined just for debugging
  // #define INCREASE_EFFECTIVE_SPAN

#ifdef INCREASE_EFFECTIVE_SPAN
	cVector3d spacer = cMul(0.005, span);
	object_min.sub(spacer);
	object_max.add(spacer);
	span = cSub(object_max, object_min);
#endif

	_cprintf("max min span (%lf,%lf,%lf) (%lf,%lf,%lf) (%lf,%lf,%lf)\n",
		object_max.x, object_max.y, object_max.z,
		object_min.x, object_min.y, object_min.z,
		span.x, span.y, span.z);

	// Move the object to put its bounding box's minimum coordinate at (0,0,0)
	// 
	// That is, move it by (the opposite of its position + the opposite of its minimum)
	old_object_pos = object_to_voxelize->getPos();
	object_to_voxelize->translate(cMul(-1.0, old_object_pos));
	object_to_voxelize->translate(cMul(-1.0, object_min));

	_cprintf("Moving object from (%lf,%lf,%lf) to (%lf,%lf,%lf)\n",
		old_object_pos.x, old_object_pos.y, old_object_pos.z,
		object_to_voxelize->getPos().x, object_to_voxelize->getPos().y, object_to_voxelize->getPos().z);

	_cprintf("Initializing subtractors...\n");

	if (objects_to_subtract.size() > 0) {
		for (unsigned int i = 0; i < objects_to_subtract.size(); i++) {
			cMesh* object_to_subtract = objects_to_subtract[i];

			// This works because the same transformations were applied to
			// all objects at load-time
			object_to_subtract->setRot(new_rot);
			object_to_subtract->setPos(0, 0, 0);
			object_to_subtract->translate(cMul(-1.0, old_object_pos));
			object_to_subtract->translate(cMul(-1.0, object_min));
			object_to_subtract->computeGlobalPositions(false);

		}
	}

	_cprintf("Initializing modifiers...\n");

	if (modifier_objects.size() > 0) {
		for (unsigned int i = 0; i < modifier_objects.size(); i++) {
			cMesh* modifier_object = modifier_objects[i];

			// This works because the same transformations were applied to
			// all objects at load-time
			modifier_object->setRot(new_rot);
			modifier_object->setPos(0, 0, 0);
			modifier_object->translate(cMul(-1.0, old_object_pos));
			modifier_object->translate(cMul(-1.0, object_min));
			modifier_object->computeGlobalPositions(false);
		}
	}

	// Man did this get me for a while...
	//
	// Note that I'm doing a computeGlobal for every vertex
	_cprintf("Computing global vertex positions...\n");
	object_to_voxelize->computeGlobalPositions(false);
	_cprintf("Done computing vertex positions...\n");

	object_to_voxelize->computeBoundaryBox(true);

	// #define PRINT_ALL_VOXELS

#ifdef ISOTROPIC_VOXELS

  // The resolution along the largest axis
#define MAX_VOXEL_RESOLUTION VOXEL_RESOLUTION

// The largest axis of this object
	float max_span = (float)(max(span.x, max(span.y, span.z)));

	// This is the isotropic voxel size
	float voxel_edge = max_span / ((float)m_long_axis_resolution);

	// Compute the resolution of each axis
	voxel_resolution[0] = (int)ceil(span.x / voxel_edge);
	voxel_resolution[1] = (int)ceil(span.y / voxel_edge);
	voxel_resolution[2] = (int)ceil(span.z / voxel_edge);

	// Compute the size of each voxel
	voxel_size.x = voxel_size.y = voxel_size.z = voxel_edge;

	// Compute the total number of voxels
	unsigned int total_num_voxels =
		voxel_resolution[0] * voxel_resolution[1] * voxel_resolution[2];

	// Anisotropic voxels
#else

  // Compute the resolution of each axis
	voxel_resolution[0] = voxel_resolution[1] = voxel_resolution[2] = VOXEL_RESOLUTION;

	// Compute the size of each voxel
	voxel_size.x = span.x / ((double)(voxel_resolution[0]));
	voxel_size.y = span.y / ((double)(voxel_resolution[1]));
	voxel_size.z = span.z / ((double)(voxel_resolution[2]));

	// Compute the total number of voxels
	unsigned int total_num_voxels =
		voxel_resolution[0] * voxel_resolution[1] * voxel_resolution[2];

#endif

	_cprintf("Voxel size is %lf,%lf,%lf\n", voxel_size.x, voxel_size.y, voxel_size.z);
	_cprintf("Voxel resolution is %d x %d x %d\n", voxel_resolution[0], voxel_resolution[1], voxel_resolution[2]);

	// Compute the closest-point-transform...
	// cpt_current_object();

	// Create an array to hold information about whether we've already
	// processed a voxel
	unsigned char* voxel_marks = new unsigned char[total_num_voxels];
	memset(voxel_marks, 0, total_num_voxels);

	// Create a stack of voxels that we still want to process
	voxel_list voxel_stack;

	cVector3d start_point;

	cMesh* seed_object = object_to_voxelize;
	bool use_recursive_index = true;

	if (selected_tri) {
		seed_triangle_index = selected_tri->m_index;
		seed_object = (cMesh*)selected_object;
		use_recursive_index = false;
	}

	_cprintf("Using triangle seed %d\n", seed_triangle_index);

	// Find a good starting point that we know is inside the object
	//{
	  // cVector3d start_point(0.5,0.5,0.5);

	  // Grab a triangle from the model
	cTriangle* tri = seed_object->getTriangle(seed_triangle_index, use_recursive_index);

	if (tri == 0) {
		_cprintf("Could not find a seed triangle...\n");
		delete[] voxel_marks;
		return;
	}

	// Find his vertices    
	cVector3d p0 = tri->getVertex(0)->getPos();
	cVector3d p1 = tri->getVertex(1)->getPos();
	cVector3d p2 = tri->getVertex(2)->getPos();

	_cprintf("Vertices at %lf,%lf,%lf %lf,%lf,%lf %lf,%lf,%lf\n",
		p0.x, p0.y, p0.z,
		p1.x, p1.y, p1.z,
		p2.x, p2.y, p2.z);

	// Find the centroid of this triangle
	start_point = cDiv(3.0, cAdd(p0, cAdd(p1, p2)));

	_cprintf("Centroid at %lf,%lf,%lf\n", start_point.x, start_point.y, start_point.z);

	// Find the normal to the triangle
	cVector3d n = cCross(cSub(p1, p0), cSub(p2, p0));

	_cprintf("Normal is %lf,%lf,%lf\n", n.x, n.y, n.z);

	// Move off the triangle by a fraction of the normal
	start_point.add(cMul(m_normal_multiplier, n));

	_cprintf("Shifted centroid at %lf,%lf,%lf\n", start_point.x, start_point.y, start_point.z);

	_cprintf("Object position is %lf,%lf,%lf\n",
		object_to_voxelize->getPos().x,
		object_to_voxelize->getPos().y,
		object_to_voxelize->getPos().z);

	// Offset to global coordinates
	start_point.add(object_to_voxelize->getPos());

	_cprintf("Global centroid at %lf,%lf,%lf\n", start_point.x, start_point.y, start_point.z);

	//}

	// start_point.set(0.5,0.5,0.5);

	// Put the known-inside voxel on the stack
	voxel_id start_voxel;
	start_voxel.i = start_point.x / voxel_size.x;
	start_voxel.j = start_point.y / voxel_size.y;
	start_voxel.k = start_point.z / voxel_size.z;

	_cprintf("Start voxel is %lf,%lf,%lf %d,%d,%d\n",
		start_point.x, start_point.y, start_point.z,
		start_voxel.i, start_voxel.j, start_voxel.k);

	voxel_start_offset.zero();
	cVector3d rounded_start_point;
	compute_voxel_coordinate(start_voxel, rounded_start_point, voxel_size, voxel_start_offset);

	// We need to add this much to get the right coordinate back every time...
	voxel_start_offset = cSub(start_point, rounded_start_point);

	// Compute the world-space zero point, for exporting
	voxel_id zero_id = { 0,0,0 };
	compute_voxel_coordinate(zero_id, zero_coordinate, voxel_size, voxel_start_offset);

	_cprintf("Voxel size is %lf %lf %lf\n", voxel_size.x, voxel_size.y, voxel_size.z);

	int start_index = voxel_index(voxel_resolution, start_voxel);
	voxel_marks[start_index] = VOXELMARK_INVOLUME_TEXTURED;

	voxel_stack.push_back(start_voxel);

	int voxels_processed = 0;
	int dot_interval = (float)total_num_voxels / 10000.0;
	if (dot_interval == 0) dot_interval = 10;

	_cprintf("Each dot will be %d voxels (of %d total)\n", dot_interval, total_num_voxels);

	// Number of dots per time interval
	int time_report_interval = 15;
	int dots_since_report = 0;

	total_time = distance_time = 0.0;

	cPrecisionClock dot_timer;
	double start_time = dot_timer.getCPUtime();

	voxel_set textured_voxels;

	// Put this voxel on the output list with his texture coordinates
	voxelfile_voxel file_voxel(start_voxel.i, start_voxel.j, start_voxel.k);

	// TODO: interpolate here (I do it for all the other triangles)
	file_voxel.has_texture = 1;
	file_voxel.is_on_border = BORDERTAG_INITIAL_ELEMENT;
	file_voxel.u = tri->getVertex(0)->getTexCoord().x;
	file_voxel.v = tri->getVertex(0)->getTexCoord().y;

	file_voxel.num_modifiers = 0;

	// voxels_to_write.insert(file_voxel);
	textured_voxels.insert(file_voxel);

	double floodfill_start_time = dot_timer.getCPUtime();

	// As long as the stack is not empty
	while (voxel_stack.empty() == 0 && quit_voxelizing == 0) {

		//_getch();
		// Sleep(100);

		// Grab the first voxel from the stack, he must be inside
		voxel_id cur_voxel = voxel_stack.back();
		voxel_stack.pop_back();

		// Compute the coordinates of this voxel
		cVector3d voxel_coordinates;
		compute_voxel_coordinate(cur_voxel, voxel_coordinates, voxel_size, voxel_start_offset);

		// TODO: sanity check
		// See whether this point is even in the bounding box of the object
		cMatrix3d object_inverse_rot;
		object_to_voxelize->getGlobalRot().transr(object_inverse_rot);
		cVector3d vox_local_coordinates = voxel_coordinates;
		vox_local_coordinates.sub(object_to_voxelize->getGlobalPos());
		object_inverse_rot.mul(vox_local_coordinates);

		bool object_in_box = cBoxContains(vox_local_coordinates,
			object_to_voxelize->getBoundaryMin(),
			object_to_voxelize->getBoundaryMax());

		if (object_in_box == false) {
			_cprintf("Voxel isn't in box...\n");
			continue;
		}

		current_voxelization_point->setPos(cSub(voxel_coordinates, object_to_voxelize->getPos()));

		if ((g_voxelize_immediately == 0) && (voxels_processed % dot_interval == 0)) {

			_cprintf(".");

			if (dots_since_report == time_report_interval) {
				dots_since_report = 0;

				double t = dot_timer.getCPUtime() - start_time;
				_cprintf("Time: %lfs\n", t);
				start_time = dot_timer.getCPUtime();
			}

			else {
				dots_since_report++;
			}

			// Rendering was taking too long...
			if (multithreaded_voxelizer == 0) {
				//render_loop();
				//Sleep(0);
			}

		}

		voxels_processed++;

#ifdef PRINT_ALL_VOXELS

		_cprintf("Processing voxel %d %d %d (%lf,%lf,%lf) (%d)\n",
			cur_voxel.i, cur_voxel.j, cur_voxel.k,
			voxel_coordinates.x, voxel_coordinates.y, voxel_coordinates.z,
			voxels_processed
		);

#endif

		// _getch();

		bool voxel_out_of_bounds = false;

		// For each of his voxel neighbors
		for (int di = -1; di <= 1 && (voxel_out_of_bounds == false); di++) {
			for (int dj = -1; dj <= 1 && (voxel_out_of_bounds == false); dj++) {
				for (int dk = -1; dk <= 1 && (voxel_out_of_bounds == false); dk++) {

					// We only do 6-connected searching
					if (!((di == 0 && dj == 0) || (dj == 0 && dk == 0) || (di == 0 && dk == 0))) continue;

					// Ignore the voxel himself
					if (di == 0 && dj == 0 && dk == 0) continue;

					// Compute the indices of this neighbor
					voxel_id neighbor = cur_voxel;
					neighbor.i += di;
					neighbor.j += dj;
					neighbor.k += dk;

					// Compute the coordinates of this neighbor
					cVector3d neighbor_coordinates;
					compute_voxel_coordinate(neighbor, neighbor_coordinates, voxel_size, voxel_start_offset);

					// Compute the index of this neighbor
					int neighbor_index = voxel_index(voxel_resolution, neighbor);


					// TODO: unnecessary to recompute every time
					// Put these neighbor coordinates in the object's frame

					// TODO: I should use the expanded bounding box here
					cMatrix3d object_inverse_rot;
					object_to_voxelize->getGlobalRot().transr(object_inverse_rot);
					cVector3d neighbor_local_coordinates = neighbor_coordinates;
					neighbor_local_coordinates.sub(object_to_voxelize->getGlobalPos());
					object_inverse_rot.mul(neighbor_local_coordinates);

					bool neighbor_in_box = cBoxContains(neighbor_local_coordinates,
						object_to_voxelize->getBoundaryMin(),
						object_to_voxelize->getBoundaryMax());

					/*
					_cprintf("Neighbor at %s, result %d (voxel %s)\n",
					  neighbor_local_coordinates.str().c_str(),neighbor_in_box,
					  voxel_coordinates.str().c_str());
					*/

					// neighbor_in_box = true;

					// Make sure this neighbor is in the bounding box of the
					// object... if not, _this_ voxel must be on the border
					if (
						neighbor.i < 0 || neighbor.i >= voxel_resolution[0] ||
						neighbor.j < 0 || neighbor.j >= voxel_resolution[1] ||
						neighbor.k < 0 || neighbor.k >= voxel_resolution[2]
						|| (neighbor_in_box == false)
						) {

						// Find the record for the current voxel
						voxelfile_voxel key(cur_voxel.i, cur_voxel.j, cur_voxel.k);
						voxel_set::iterator iter = textured_voxels.find(key);
						if (iter == textured_voxels.end()) {
							_cprintf("That's strange... I couldn't find the current voxel.\n");
							continue;
						}

						//_cprintf("Re-assigned a voxel to the border...\n");
						voxelfile_voxel v = (*iter);

						// A note-to-self that this voxel was _re-assigned_ to the border
						v.is_on_border = BORDER_TAG_BAD_NEIGHBOR;
						textured_voxels.erase(iter);
						textured_voxels.insert(v);

						if (render_point_cloud) {

							cVector3d cloud_point = cSub(voxel_coordinates, object_to_voxelize->getPos());
							simple_point p;
							p.x = cloud_point.x;
							p.y = cloud_point.y;
							p.z = cloud_point.z;
							p.r = 0.0;
							p.g = 0.0;
							p.b = 1.0;

							// On the border of the entire region, distances not computed
							WaitForSingleObject(cloud->point_mutex, INFINITE);
							cloud->points[voxel_index(voxel_resolution, cur_voxel)] = p;
							ReleaseMutex(cloud->point_mutex);

						}

#ifdef PRINT_ALL_VOXELS     
						_cprintf("Voxel %d,%d,%d out of bounds\n",
							neighbor.i, neighbor.j, neighbor.k);
#endif
						voxel_out_of_bounds = true;
						continue;

					} // if this neighbor is outside the bounding box

					// Extend the ray a little to avoid "just barely" hitting triangles
					cVector3d offset(
						COLLISION_OFFSET_FACTOR*voxel_size.x*(float)di,
						COLLISION_OFFSET_FACTOR*voxel_size.y*(float)dj,
						COLLISION_OFFSET_FACTOR*voxel_size.z*(float)dk
					);

					// If this neighbor has been marked already, skip him
					if (voxel_marks[neighbor_index] != VOXELMARK_UNMARKED) {

#ifdef PRINT_ALL_VOXELS              
						/*
					  _cprintf("Voxel %d,%d,%d (%lf,%lf,%lf) already processed\n",
						  neighbor.i,neighbor.j,neighbor.k,
						  neighbor_coordinates.x,neighbor_coordinates.y,neighbor_coordinates.z);
						  */
#endif
						continue;
					}

					cGenericObject* colObject;
					cTriangle*      colTriangle;
					cVector3d       colPoint;
					double          colSquareDistance = DBL_MAX;

					// See if this ray enters the "cut zone"
					if (objects_to_subtract.size() > 0) {

						int found_cut_zone = 0;

						for (unsigned int subtractor_index = 0; subtractor_index < objects_to_subtract.size(); subtractor_index++) {

							cMesh* object_to_subtract = objects_to_subtract[subtractor_index];

							// See if this ray enters the "cut zone"
							int result = object_to_subtract->computeCollisionDetection(

								// Create a ray from the voxel to this neighbor
								voxel_coordinates, cAdd(neighbor_coordinates, offset),

								// Stuff the cd wants to return
								colObject, colTriangle, colPoint, colSquareDistance,

								0, 1);

							// If it does, mark this neighbor as outside the volume and continue
							//
							// Note that if we cared to be thoroughly consistent, we might
							// want to consider this voxel as _inside_ the volume and compute
							// surface information for him, like we do for voxels that are
							// found to lie just outside the object.
							if (result) {
								found_cut_zone = 1;
								voxel_marks[neighbor_index] = VOXELMARK_NOTINVOLUME;
							}

							if (found_cut_zone) break;

						} // for each object

						if (found_cut_zone) continue;

					} // if we have subtractors

					colSquareDistance = DBL_MAX;

					cVector3d start_point = voxel_coordinates;
					cVector3d end_point;
					neighbor_coordinates.addr(offset, end_point);

					// See if this ray leaves the object
					int result = object_to_voxelize->computeCollisionDetection(

						// Create a ray from the voxel to this neighbor
						start_point, end_point,

						// Stuff the cd wants to return
						colObject, colTriangle, colPoint, colSquareDistance,

						0, 1);

					/*
					_cprintf("Test from %s to %s... result %d\n",
					  voxel_coordinates.str().c_str(),end_point.str().c_str(),(int)result);
					*/

					// If it does, mark this neighbor as on the border and continue
					if (result) {

						voxel_marks[neighbor_index] = VOXELMARK_INVOLUME_TEXTURED;

#ifdef PRINT_ALL_VOXELS            
						_cprintf("Voxel %d,%d,%d (%lf,%lf,%lf) is not in the volume\n",
							neighbor.i, neighbor.j, neighbor.k,
							neighbor_coordinates.x, neighbor_coordinates.y, neighbor_coordinates.z);
#endif

						// File-writing stuff
						// {

						// In order to interpolate between vertices, we'll have
						// to find barycentric coordinates for the intersection point

						// From :
						//
						// http://kurtm.flipcode.com/devlog-VoxelMeshCreationandRendering.shtml

						/*
						a = area(Source Triangle)

						bu = area([T1, T2, PT]) / a
						bv = area([T2, T0, PT]) / a
						bw = area([T0, T1, PT]) / a
						*/

						// These points are all in object local space...
						cVector3d p0 = colTriangle->getVertex(0)->getPos();
						cVector3d p1 = colTriangle->getVertex(1)->getPos();
						cVector3d p2 = colTriangle->getVertex(2)->getPos();

						// So we need to convert this point to object local space also...
						colPoint.sub(object_to_voxelize->getPos());

						raw_triangle source_triangle(p0, p1, p2);

						float a = source_triangle.compute_area();

						raw_triangle t1t2pt(p1, p2, colPoint);
						raw_triangle t2t0pt(p2, p0, colPoint);
						raw_triangle t0t1pt(p0, p1, colPoint);

						float a0 = t1t2pt.compute_area();
						float a1 = t2t0pt.compute_area();
						float a2 = t0t1pt.compute_area();

						float bu = a0 / a;
						float bv = a1 / a;
						float bw = a2 / a;

						/*
						TU = (bu * T0.TU) + (bv * T1.TU) + (bw * T2.TU);
						TV = (bv * T0.TV) + (bv * T1.TV) + (bw * T2.TV);
						*/

						// Interpolate in barycentric coordinates
						float u = bu * colTriangle->getVertex(0)->getTexCoord().x +
							bv * colTriangle->getVertex(1)->getTexCoord().x +
							bw * colTriangle->getVertex(2)->getTexCoord().x;

						float v = bu * colTriangle->getVertex(0)->getTexCoord().y +
							bv * colTriangle->getVertex(1)->getTexCoord().y +
							bw * colTriangle->getVertex(2)->getTexCoord().y;

						// A simpler scheme, for debugging...
						/*
						u = colTriangle->getVertex(0)->getTexCoord().x +
							colTriangle->getVertex(1)->getTexCoord().x +
							colTriangle->getVertex(2)->getTexCoord().x;
						u/=3.0;

						v = colTriangle->getVertex(0)->getTexCoord().y +
							colTriangle->getVertex(1)->getTexCoord().y +
							colTriangle->getVertex(2)->getTexCoord().y;
						v/=3.0;
						*/


						// Insert this voxel in the output set of "interesting" voxels
						voxelfile_voxel file_voxel(neighbor.i, neighbor.j, neighbor.k);
						file_voxel.u = u;
						file_voxel.v = v;
						file_voxel.has_texture = 1;
						file_voxel.is_on_border = BORDER_TAG_NEIGHBOR_COLLISION;
						file_voxel.has_normal = 1;

						cVector3d n;

						static int printed_normal_status = 0;

#if (NORMAL_SOURCE == NORMAL_SOURCE_TRIANGLE_FACE)

						// Insert a normal for this voxel also, based on this triangle's face normal...
						cVector3d nv1 = cSub(p1, p0);
						cVector3d nv2 = cSub(p2, p0);
						n = cCross(nv1, nv2);
						n.normalize();

						if (printed_normal_status == 0) {
							_cprintf("Using face normals...\n");
							printed_normal_status = 1;
						}

#elif (NORMAL_SOURCE == NORMAL_SOURCE_VERTEX_NORMALS)

						if (printed_normal_status == 0) {
							_cprintf("Using vertex normals...\n");
							printed_normal_status = 1;
						}

						// Insert a normal for this voxel also, based on vertex normals...
						cVector3d v0n = colTriangle->getVertex(0)->getNormal();
						cVector3d v1n = colTriangle->getVertex(1)->getNormal();
						cVector3d v2n = colTriangle->getVertex(2)->getNormal();

						double d0 = cDistance(colPoint, colTriangle->getVertex(0)->getPos());
						double d1 = cDistance(colPoint, colTriangle->getVertex(1)->getPos());
						double d2 = cDistance(colPoint, colTriangle->getVertex(2)->getPos());

						double totald = d0 + d1 + d2;
						n = (v0n * (totald - d0)) + (v1n * (totald - d1)) + (v2n * (totald - d2));
						n.normalize();

#elif (NORMAL_SOURCE == NORMAL_SOURCE_NONE)

						file_voxel.has_normal = 0;

#else 
						_cprintf("Unrecognized normal computation scheme...\n");
#endif            

						file_voxel.normal[0] = n.x;
						file_voxel.normal[1] = n.y;
						file_voxel.normal[2] = n.z;

						if (m_compute_distance_field) {

							double t1 = dot_timer.getCPUtime();

							file_voxel.has_distance = 1;

							cVector3d closest_point;
							file_voxel.distance_to_surface =
								AABB_distance_computer::closest_vertex(
									object_to_voxelize, neighbor_coordinates, 0, &closest_point);
							cVector3d gradient = cSub(closest_point, neighbor_coordinates);
							gradient.normalize();

							// This corrects for the fact that this center is
							// _outside_ the object
							gradient.mul(-1.0);
							file_voxel.distance_gradient[0] = gradient.x;
							file_voxel.distance_gradient[1] = gradient.y;
							file_voxel.distance_gradient[2] = gradient.z;

							double elapsed = dot_timer.getCPUtime() - t1;
							distance_time += elapsed;

						}

						// TODO: average over the relevant triangles (right now
						// only the _first_ triangle to get hit for this voxel
						// affects the final output)

						// voxels_to_write.insert(file_voxel);

						file_voxel.num_modifiers = 0;

						textured_voxels.insert(file_voxel);

						// _Don't_ put him on the stack of voxels to process

						// } // file-writing stuff

						if (render_point_cloud) {

							cVector3d cloud_point = cSub(neighbor_coordinates, object_to_voxelize->getPos());
							simple_point p;
							p.x = cloud_point.x;
							p.y = cloud_point.y;
							p.z = cloud_point.z;
							p.r = 0.0;
							p.g = 1.0;
							p.b = 0.0;

#ifdef STORE_POINT_CLOUD_DISTANCE_INFO
							// We hit a triangle, so we have distance values
							p.distance = file_voxel.distance_to_surface;
							p.closest_point = closest_point;
#endif
							WaitForSingleObject(cloud->point_mutex, INFINITE);
							cloud->points[neighbor_index] = p;
							ReleaseMutex(cloud->point_mutex);

						}
						continue;

					} // if we hit a triangle

					// If it doesn't hit a triangle, he's in the volume

					// Mark him as in the volume, untextured by default
					voxel_marks[neighbor_index] = VOXELMARK_INVOLUME_UNTEXTURED;

#ifdef PRINT_ALL_VOXELS
					_cprintf("Voxel %d,%d,%d (%lf,%lf,%lf) is in the volume\n",
						neighbor.i, neighbor.j, neighbor.k,
						neighbor_coordinates.x, neighbor_coordinates.y, neighbor_coordinates.z);
#endif

					// Put him on the list of voxels to write out
					voxelfile_voxel file_voxel(neighbor.i, neighbor.j, neighbor.k);

					if (m_compute_distance_field) {

						double t1 = dot_timer.getCPUtime();

						file_voxel.has_distance = 1;

						cVector3d closest_point;
						file_voxel.distance_to_surface =
							AABB_distance_computer::closest_vertex(
								object_to_voxelize, neighbor_coordinates, 0, &closest_point);

						cVector3d gradient = cSub(closest_point, neighbor_coordinates);
						gradient.normalize();
						file_voxel.distance_gradient[0] = gradient.x;
						file_voxel.distance_gradient[1] = gradient.y;
						file_voxel.distance_gradient[2] = gradient.z;

						double elapsed = dot_timer.getCPUtime() - t1;
						distance_time += elapsed;

					}

					if (render_point_cloud) {

						cVector3d cloud_point = cSub(neighbor_coordinates, object_to_voxelize->getPos());
						simple_point p;
						p.x = cloud_point.x;
						p.y = cloud_point.y;
						p.z = cloud_point.z;
						p.r = 1.0;
						p.g = 0.0;
						p.b = 0.0;


#ifdef STORE_POINT_CLOUD_DISTANCE_INFO
						// We are an internal voxel, so we have distance values
						p.distance = file_voxel.distance_to_surface;
						p.closest_point = closest_point;
#endif
						WaitForSingleObject(cloud->point_mutex, INFINITE);
						cloud->points[neighbor_index] = p;
						ReleaseMutex(cloud->point_mutex);

					}

					file_voxel.is_on_border = BORDER_TAG_NOT_ON_BORDER;
					textured_voxels.insert(file_voxel);

					// Put him on the stack of voxels to process
					voxel_stack.push_back(neighbor);

				} // for all neighboring k's

			} // for all neighboring j's

		} // for all neighboring i's

	} // while the stack isn't empty

	found_voxels = 0;
	for (unsigned int i = 0; i < total_num_voxels; i++) {
		if (voxel_marks[i] > VOXELMARK_NOTINVOLUME) found_voxels++;
	}

	total_time = dot_timer.getCPUtime() - floodfill_start_time;

	_cprintf("\n\nFound %d voxels after processing %d voxels, total time %f\n", found_voxels, voxels_processed, total_time);

	if (m_compute_distance_field)
		_cprintf("Largest and smallest distances to bounding object: %3.3lf %3.3lf, df time %lf\n", g_largest_distance, g_smallest_distance, distance_time);

	// Do the tetrahedralization based on these voxels
	if (operation == OPERATION_TETRAHEDRALIZE) {

		// A searchable list of tets
		std::set<indexed_tet, ltindexed_tet> tets;

		// A searchable list of exterior faces
		std::set<indexed_face, ltindexed_face> faces;

		// a map from coordinates to vertex id's
		std::map<outgoing_vertex_info, unsigned int, lt_outgoing_vertex_info> coordinate_to_vertexid_map;

		// a list of coordinates, in vertex id order
		std::vector<outgoing_vertex_info> coordinates_in_vertex_order;

		// The unique vertex id we'll assign to the next new vertex we find
		unsigned int current_vertex_id = 0;

		voxel_set::iterator iter = textured_voxels.begin();

		_cprintf("\nTetrahedralizing...\n");

		int voxels_processed = 0;
		int dot_interval = (float)textured_voxels.size() / 10000.0;
		if (dot_interval == 0) dot_interval = 10;

		_cprintf("Each dot will be %d textured voxels (of %d total)\n", dot_interval, textured_voxels.size());

		// For each voxel on the list
		while (iter != textured_voxels.end()) {

			voxelfile_voxel v = *iter;
			voxel_id vid = { v.i,v.j,v.k };
			cVector3d voxelpos;
			compute_voxel_coordinate(vid, voxelpos, voxel_size, voxel_start_offset);

			int border_voxel = (v.is_on_border != 0) ? 1 : 0;

			// Now build eight vertices from this voxel to make a cube
			cVector3d cube_vertices[8];

			// Are these "border vertices?"
			bool cube_vertices_on_border[8];
			memset(cube_vertices_on_border, 0, 8 * sizeof(bool));

			int curcorner = 0;
			for (int i = -1; i <= 1; i += 2) {
				for (int j = -1; j <= 1; j += 2) {
					for (int k = -1; k <= 1; k += 2) {
						cube_vertices[curcorner].set(
							voxelpos.x + (double)(i)*voxel_size.x / 2.0,
							voxelpos.y + (double)(j)*voxel_size.y / 2.0,
							voxelpos.z + (double)(k)*voxel_size.z / 2.0
						);

						// If this is a border voxel, check whether these are border
						// vertices
						if (border_voxel) {

							// Do any of my 6-connected neighbors not exist?
							voxelfile_voxel key_vf(v.i, v.j, v.k);

							voxel_set::iterator search_iter;

							key_vf.i = v.i + i; key_vf.j = v.j; key_vf.k = v.k;
							search_iter = textured_voxels.find(key_vf);
							if (search_iter == textured_voxels.end()) {
								//_cprintf("I'm %d,%d,%d and %d,%d,%d isn't there; I'm on the border...\n",
								//  v.i,v.j,v.k,key_vf.i,key_vf.j,key_vf.k);
								cube_vertices_on_border[curcorner] = true;
							}

							key_vf.i = v.i; key_vf.j = v.j + j; key_vf.k = v.k;
							search_iter = textured_voxels.find(key_vf);
							if (search_iter == textured_voxels.end()) {
								//_cprintf("I'm %d,%d,%d and %d,%d,%d isn't there; I'm on the border...\n",
								//  v.i,v.j,v.k,key_vf.i,key_vf.j,key_vf.k);
								cube_vertices_on_border[curcorner] = true;
							}

							key_vf.i = v.i; key_vf.j = v.j; key_vf.k = v.k + k;
							search_iter = textured_voxels.find(key_vf);
							if (search_iter == textured_voxels.end()) {
								//_cprintf("I'm %d,%d,%d and %d,%d,%d isn't there; I'm on the border...\n",
								//  v.i,v.j,v.k,key_vf.i,key_vf.j,key_vf.k);
								cube_vertices_on_border[curcorner] = true;
							}

						}
						curcorner++;
					}
				}
			}

			unsigned int cube_vertex_ids[8];
			// Assign or retrieve vertex id's for each vertex
			for (int i = 0; i < 8; i++) {

				cVector3d cur_vertex_pos = cube_vertices[i];
				outgoing_vertex_info ovi;
				ovi.pos = cur_vertex_pos;
				ovi.border = cube_vertices_on_border[i];

				// Does this guy already have a vertex id?
				std::map<outgoing_vertex_info, unsigned int, lt_outgoing_vertex_info>::iterator iter =
					coordinate_to_vertexid_map.find(ovi);

				// This is a new vertex
				if (iter == coordinate_to_vertexid_map.end()) {

					cube_vertex_ids[i] = current_vertex_id;
					coordinate_to_vertexid_map[ovi] = current_vertex_id;

					// _cprintf("Adding vertex %d (%s)\n",current_vertex_id,cur_vertex_pos.str(2).c_str());

					current_vertex_id++;

					// If we assign a new vertex id, put this coordinate
					// on the output list
					coordinates_in_vertex_order.push_back(ovi);
				}

				// We've seen this vertex before
				else {
					cube_vertex_ids[i] = (*iter).second;
					outgoing_vertex_info old_ovi = (*iter).first;

					// Is this a change in border status?
					if (old_ovi.border != ovi.border) {

						// If I'm changing him to say that he _is_ on the border,
						// update the list.
						if (ovi.border) {
							coordinates_in_vertex_order[(*iter).second] = ovi;
						}
						//_cprintf("Warning: vertex %d (%s) has old border %d, new border %d\n",
						  //(*iter).second,cur_vertex_pos.str(2).c_str(),(int)(old_ovi.border),(int)(ovi.border));
					}
				}
			} // for each cube vertex

			// Build tets from these vertices

			for (int curtet = 0; curtet < TETS_PER_CUBE; curtet++) {

				indexed_tet t;
				for (int i = 0; i < 4; i++) {
					int index;
					if (TETS_PER_CUBE == 5)
						index = five_tet_cube_corners[curtet][i];
					else
						index = six_tet_cube_corners[curtet][i];
					t.vertices[i] = cube_vertex_ids[index];
				}

				indexed_tet sorted_tet = t;

				// Sort this tet
				sort_indexed_tet(sorted_tet);

				// Confirm that this tet is not degenerate
				if (isDegenerateTet(sorted_tet)) {
					_cprintf("Oops... voxel %d, tet %d is degenerate...\n",
						voxels_processed, curtet);
					continue;
				}

				// Confirm that we haven't build this tet already
				std::set<indexed_tet, ltindexed_tet>::iterator tetsearch_iter =
					tets.find(sorted_tet);

				bool tet_exists = (tetsearch_iter != tets.end());

				if (tet_exists) {
					_cprintf("Oops... voxel %d, tet %d already exists (%d,%d,%d,%d)...\n",
						voxels_processed, curtet,
						sorted_tet.vertices[0], sorted_tet.vertices[1],
						sorted_tet.vertices[2], sorted_tet.vertices[3]);
					continue;
				}

				// Record boundary information for this tet
				sorted_tet.attribute = v.is_on_border;

				// Okay, we appear to have a new tet
				tets.insert(sorted_tet);

				// Build faces if this is a border voxel
				if (v.is_on_border) {

					// TODO: this approach creates many subsurface faces; it only
					// tests whether the center voxel is on the border 
					for (int curface = 0; curface < 4; curface++) {
						indexed_face f;
						f.vertices[0] = sorted_tet.vertices[(tet_triangle_faces[curface][0])];
						f.vertices[1] = sorted_tet.vertices[(tet_triangle_faces[curface][1])];
						f.vertices[2] = sorted_tet.vertices[(tet_triangle_faces[curface][2])];

						sort_indexed_face(f);

						// Strictly speaking I could just add the face and let the STL
						// avoid duplication, but I leave the code this way in case I
						// want to do some processing on each new face later, and as a
						// sanity check...

						// Is this face already on the list?
						std::set<indexed_face, ltindexed_face>::iterator face_search =
							faces.find(f);

						// Ignore this face if we've already created it
						if (face_search != faces.end()) continue;

						// Put this face on our face list
						faces.insert(f);
					} // for each face we can make from this tet

				} // if this voxel is on the border

			} // for each tet we made from this voxel

			iter++;
			voxels_processed++;
			if ((g_voxelize_immediately == 0) && ((voxels_processed % dot_interval) == 0)) _cprintf(".");

		} // for each voxel

		m_nTets = tets.size();

		// Update the total time to include tetrahedralization
		total_time = dot_timer.getCPUtime() - floodfill_start_time;

		if (m_write_output_file) {

			// generate a name for the output file

			// Open up and write out the output files
			char node_filename[_MAX_PATH];
			char ele_filename[_MAX_PATH];
			char face_filename[_MAX_PATH];

			// Build nice descriptive filenames...
			char file_details_str[1000];
			file_details_str[0] = '\0';

			for (unsigned int i = 0; i < objects_to_subtract.size(); i++) {
				char* filename = (char*)(objects_to_subtract[i]->m_userData);
				char buf[_MAX_PATH];
				sprintf(buf, ".s_%s", filename);
				strcat(file_details_str, buf);
			}

			sprintf(node_filename, "%s%s.v%d.%s", output_filename_root, file_details_str,
				m_long_axis_resolution, NODEFILE_EXTENSION);

			sprintf(ele_filename, "%s%s.v%d.%s", output_filename_root, file_details_str,
				m_long_axis_resolution, ELEFILE_EXTENSION);

			sprintf(face_filename, "%s%s.v%d.%s", output_filename_root, file_details_str,
				m_long_axis_resolution, FACEFILE_EXTENSION);

			_cprintf("\nWriting %d vertices and %d tets\n", current_vertex_id, tets.size());
			_cprintf("Output filenames are:\n%s\n%s\n%s\n\n", ele_filename, node_filename, face_filename);

			// open the output files
			FILE* nodef = fopen(node_filename, "wb");
			if (nodef == 0) {
				_cprintf("After all this work, I couldn't open the file %s\n", node_filename);
			}
			else {
				// Write out the node file header
				int ndim = 3;
				int nboundary_markers = 1;
				int nattributes = 0;
				unsigned int npoints = current_vertex_id;

				fprintf(nodef, "# Generated by voxelizer ( http://cs.stanford.edu/~dmorris/voxelizer )\n");
				fprintf(nodef, "# SCALE %f\n", model_scale_factor);
				fprintf(nodef, "# OFFSET %f %f %f\n", (float)(model_offset.x), (float)(model_offset.y), (float)(model_offset.z));
				fprintf(nodef, "# ZERO %f %f %f\n", (float)zero_coordinate.x, (float)zero_coordinate.y, (float)zero_coordinate.z);
				fprintf(nodef, "%u %d %d %d\n", npoints, ndim, nattributes, nboundary_markers);
			}

			FILE* elef = fopen(ele_filename, "wb");
			if (elef == 0) {
				_cprintf("After all this work, I couldn't open the file %s\n", ele_filename);
			}
			else {
				// Write out the ele file header
				int nodes_per_tet = 4;
				int nattributes = 1;

				fprintf(elef, "%u %d %d\n", m_nTets, nodes_per_tet, nattributes);
			}

			FILE* facef = fopen(face_filename, "wb");
			if (facef == 0) {
				_cprintf("After all this work, I couldn't open the file %s\n", face_filename);
			}
			else {
				// Write out the face file header
				unsigned int nfaces = faces.size();
				int nattributes = 0;

				fprintf(facef, "%u %d\n", nfaces, nattributes);
			}

			if (nodef && elef && facef) {

				// write all the vertex id's and positions out

				unsigned int vertex_id = 0;
				std::vector<outgoing_vertex_info>::iterator coord_iter = coordinates_in_vertex_order.begin();

				int n_border_vertices = 0;

				// for each voxel that was assigned a vertex id
				while (coord_iter != coordinates_in_vertex_order.end()) {

					outgoing_vertex_info cur_ovi = (*coord_iter);
					cVector3d pos = cur_ovi.pos;
					bool border = cur_ovi.border;

					if (border) n_border_vertices++;

					int boundary_marker = border ? 1 : 0;

					// write out the vertex index and position
					fprintf(nodef, "%u %3.3lf %3.3lf %3.3lf %d\n",
						vertex_id, pos.x, pos.y, pos.z, boundary_marker);

					vertex_id++;
					coord_iter++;
				}

				_cprintf("Wrote %d total vertices, %d border vertices\n",
					coordinates_in_vertex_order.size(), n_border_vertices);

				// go through the list of tets
				std::set<indexed_tet, ltindexed_tet>::iterator tetiter = tets.begin();
				unsigned int tet_id = 0;
				while (tetiter != tets.end()) {

					indexed_tet t = (*tetiter);

					fprintf(elef, "%u", tet_id);

					// write out each tet with _vertex_ id's
					for (int i = 0; i < 4; i++) {
						fprintf(elef, " %u", t.vertices[i]);
					}
					fprintf(elef, " %d", t.attribute);

					fprintf(elef, "\n");

					tet_id++;
					tetiter++;
				}

				// go through the list of faces
				std::set<indexed_face, ltindexed_face>::iterator faceiter = faces.begin();
				unsigned int face_id = 0;
				while (faceiter != faces.end()) {
					indexed_face f = (*faceiter);
					fprintf(facef, "%d %d %d %d\n", face_id, f.vertices[0], f.vertices[1], f.vertices[2]);
					face_id++;
					faceiter++;
				}

				// close the file
				if (nodef) fclose(nodef);
				if (elef) fclose(elef);
				if (facef) fclose(facef);

			} // if we were able to open the files

			_cprintf("Finished writing tets to file...\n");

		} // if we're writing output

	} // if we're generating tetrahedra

	else if (operation == OPERATION_VOXELIZE) {

		// Open up and write out the binary output file
		char binary_filename[_MAX_PATH];

		// Build a nice descriptive filename...
		char file_details_str[1000];
		file_details_str[0] = '\0';

		for (unsigned int i = 0; i < objects_to_subtract.size(); i++) {
			char* filename = (char*)(objects_to_subtract[i]->m_userData);
			char buf[_MAX_PATH];
			sprintf(buf, ".s_%s", filename);
			strcat(file_details_str, buf);
		}

		for (unsigned int i = 0; i < modifier_objects.size(); i++) {
			char* filename = (char*)(modifier_objects[i]->m_userData);
			char buf[_MAX_PATH];
			sprintf(buf, ".m_%s", filename);
			strcat(file_details_str, buf);
		}

		sprintf(binary_filename, "%s%s.v%d.%s", output_filename_root, file_details_str,
			m_long_axis_resolution, BINARY_EXTENSION);

		_cprintf("Output filename is %s\n", binary_filename);

		// If necessary, compute the distance from each voxel that we're
		// going to output to the modifier mesh...
		if (modifier_objects.size() > 0) {

			unsigned int num_modifiers = modifier_objects.size();

			for (unsigned int modifier_index = 0; modifier_index < num_modifiers; modifier_index++) {

				_cprintf("\nComputing distances to modifier object... %d\n", modifier_index);

				if (modifier_index >= MAX_NUM_MODIFIERS) {
					_cprintf("Warning: only %d modifiers are supported\n", MAX_NUM_MODIFIERS);
					break;
				}

				cMesh* modifier_object = modifier_objects[modifier_index];
				_cprintf("Object has %d triangles\n", modifier_object->getNumTriangles(true));

				// Force a reset for the distance computation on the first pass
				int first_pass = 1;

				// The modifier mesh has already been transformed to match
				// the transform we applied to the main mesh

				int total_voxels = textured_voxels.size();
				int dot_interval = ((float)total_voxels) / 1000.0;
				_cprintf("Computing %d voxel distances (%d per dot)\n",
					total_voxels, dot_interval);

				int voxels_processed = 0;

				double min_distance = DBL_MAX;
				double max_distance = 0.0;

				voxel_set::iterator iter = textured_voxels.begin();

				// For each voxel
				while (iter != textured_voxels.end()) {

					voxels_processed++;

					if ((voxels_processed % dot_interval) == 0) _cprintf(".");

					// I think this should let us modify the data in-place
					voxelfile_voxel* v = (voxelfile_voxel*)(&(*iter));

					voxel_id vi = { v->i,v->j,v->k };

					cVector3d voxel_coordinates;
					compute_voxel_coordinate(vi, voxel_coordinates, voxel_size, voxel_start_offset);

					cVector3d p;
					double d = AABB_distance_computer::closest_vertex(
						modifier_object, voxel_coordinates, 0, &p, first_pass);

					if (d < min_distance) min_distance = d;
					if (d > max_distance) max_distance = d;

					v->distance_to_modifier[modifier_index] = d;

					if (first_pass) _cprintf("Initializing distance variables\n");
					first_pass = 0;

					cVector3d gradient = cSub(p, voxel_coordinates);

					// Sanity check the returned value
					double err = fabs(gradient.length() - v->distance_to_modifier[modifier_index]);

					if (err > 0.001) _cprintf("Distance error %lf\n", err);

					gradient.normalize();

					v->num_modifiers++;
					v->modifier_gradient[modifier_index][0] = gradient.x;
					v->modifier_gradient[modifier_index][1] = gradient.y;
					v->modifier_gradient[modifier_index][2] = gradient.z;

					// Replace the old voxel
					// 
					// Now I'm modifying in-place...
					// textured_voxels.insert(v);

					iter++;

				} // for each voxel

				_cprintf("Finished computing distances to modifier object %d...\n", modifier_index);
				_cprintf("Min and max distances to modifier were %3.3lf, %3.3lf\n", min_distance, max_distance);

			} // for each modifier object

		} // if there's a modifier object

		if (m_write_output_file) {

			FILE* binary_f = fopen(binary_filename, "wb");

			if (binary_f == 0) {

				_cprintf("Could not open binary output file %s\n", binary_filename);

			}

			else {

				// Write out binary file header and object header
				voxelfile_file_header file_hdr;
				file_hdr.header_size = sizeof(file_hdr);
				file_hdr.object_header_size = sizeof(voxelfile_object_header);
				file_hdr.voxel_struct_size = sizeof(voxelfile_voxel);
				file_hdr.num_objects = 1;

				fwrite(&file_hdr, sizeof(file_hdr), 1, binary_f);

				_cprintf("File header is %d bytes\n", file_hdr.header_size);
				_cprintf("Object header is %d bytes\n", file_hdr.object_header_size);
				_cprintf("Voxel struct is %d bytes\n", file_hdr.voxel_struct_size);

				voxelfile_object_header object_hdr;

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

				_cprintf("\nWriting %d voxels (%d textured voxels) to the binary output file\n",
					object_hdr.num_voxels, textured_voxels.size());

				_cprintf("\nResolution %d,%d,%d, size %f,%f,%f\n",
					object_hdr.voxel_resolution[0], object_hdr.voxel_resolution[1], object_hdr.voxel_resolution[2],
					object_hdr.voxel_size[0], object_hdr.voxel_size[1], object_hdr.voxel_size[2]);

				object_hdr.has_texture = 1;

				strcpy(object_hdr.texture_filename, current_texture);
				_cprintf("Storing texture %s\n", current_texture);

				fwrite(&object_hdr, sizeof(object_hdr), 1, binary_f);

				for (int i = 0; i < voxel_resolution[0]; i++) {
					for (int j = 0; j < voxel_resolution[1]; j++) {
						for (int k = 0; k < voxel_resolution[2]; k++) {

							voxel_id vi = { i,j,k };

							int index = voxel_index(voxel_resolution, vi);

							int mark = voxel_marks[index];

							if (mark == VOXELMARK_NOTINVOLUME) fputc(ASCIIVOXEL_NOVOXEL, binary_f);

							/*
							else if (mark == VOXELMARK_INVOLUME_UNTEXTURED) fputc(ASCIIVOXEL_INTERNAL_VOXEL,binary_f);

							else if (mark == VOXELMARK_INVOLUME_TEXTURED) {
							*/

							else if (mark == VOXELMARK_INVOLUME_TEXTURED || mark == VOXELMARK_INVOLUME_UNTEXTURED) {

								voxelfile_voxel cur(i, j, k);
								voxel_set::iterator iter = textured_voxels.find(cur);

								if (iter == textured_voxels.end()) {
									_cprintf("Could not find voxel %d,%d,%d on the textured voxel list\n",
										i, j, k);
									fputc(ASCIIVOXEL_INTERNAL_VOXEL, binary_f);
								}

								else {

									fputc(ASCIIVOXEL_TEXTURED_VOXEL, binary_f);
									voxelfile_voxel out = *iter;
									fwrite(&out, sizeof(voxelfile_voxel), 1, binary_f);

								}

							}

							else {

								// unmarked
								fputc(ASCIIVOXEL_NOVOXEL, binary_f);

							}
						}
					}
				}

				fclose(binary_f);
			}

		} // if we're writing output

	}  // if we're generating voxels instead of tetrahedra

	else if (operation == OPERATION_TESTONLY) {

	}

	else {
		_cprintf("Unrecognized voxelization operation\n");
	}

	if (current_voxelization_point)
		current_voxelization_point->setShowFrame(0, 0);

	if (render_point_cloud) {

		_cprintf("Preparing point cloud\n");

		std::map<int, simple_point>::iterator iter;
		float minx = 100;
		for (iter = cloud->points.begin(); iter != cloud->points.end(); iter++) {
			if ((*iter).second.x < minx) minx = (*iter).second.x;
		}

		_cprintf("Min x value is %f (%f global)\n", minx, minx + object_to_voxelize->getGlobalPos().x);

	}

	_cprintf("Done.\n\n");

	// Delete our voxel mark array
	delete[] voxel_marks;


}


CPointCloud::CPointCloud() : cGenericObject() {

	point_mutex = CreateMutex(0, 0, "point_mutex");

	if (point_mutex == 0) {
		_cprintf("Could not create point mutex\n");
	}

}


CPointCloud::~CPointCloud() {

}


void CPointCloud::render(int a_renderMode) {

	glPointSize(4.0);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	WaitForSingleObject(point_mutex, INFINITE);

	simple_point* vertexp = (simple_point*)(&(points[0]));

	glBegin(GL_POINTS);

	std::map<int, simple_point>::iterator pointiter = points.begin();
	for (; pointiter != points.end(); pointiter++) {
		simple_point& sp = (*pointiter).second;
		glColor3f(sp.r, sp.g, sp.b);
		glVertex3f(sp.x, sp.y, sp.z);
	}

	glEnd();

	ReleaseMutex(point_mutex);

}

void CvoxelizerApp::render(const int a_renderMode) {

#ifdef STORE_POINT_CLOUD_DISTANCE_INFO
	if (g_main_app->haptics_enabled && cloud) {
		cVector3d pos = g_main_app->tool->m_deviceGlobalPos;
		glPushMatrix();
		glTranslated(pos.x, pos.y, pos.z);
		cDrawSphere(0.1);
		glPopMatrix();

		// pos.sub(object_to_voxelize->getPos());

		voxel_id current_voxel_id = compute_voxel_id(pos, voxel_size, voxel_start_offset);

		int index = voxel_index(voxel_resolution, current_voxel_id);

		std::map<int, simple_point>::iterator iter = cloud->points.find(index);

		if (iter == cloud->points.end()) {
			_cprintf("No point found for index %d (%d,%d,%d)...\n", index,
				(int)current_voxel_id.i, (int)current_voxel_id.j, (int)current_voxel_id.k);
		}
		else {
			simple_point p = (*iter).second;
			_cprintf("Distance is %f\n", p.distance);

			glPushMatrix();
			glTranslated(pos.x, pos.y, pos.z);
			glColor3f(0, 1, 0);
			cDrawSphere(0.1);
			glPopMatrix();
		}
	}
#endif

}

AABB_distance_computer::AABB_distance_computer() {

}

AABB_distance_computer::~AABB_distance_computer() {

}

// Compute distances from a given point to each vertex of a box
void compute_sq_distances(const cCollisionAABBBox& box, const cVector3d& p, double sq_distances[8]) {

	cVector3d v[8];
	register unsigned char i;

	for (i = 0; i < 8; i++) {

		v[i] = box.m_min;

		if (i & (1 << 0)) v[i].x = box.m_max.x;
		if (i & (1 << 1)) v[i].y = box.m_max.y;
		if (i & (1 << 2)) v[i].z = box.m_max.z;

	}

	for (i = 0; i < 8; i++) {
		sq_distances[i] = v[i].distancesq(p);
	}
}

// Compute the best-case and worst-case squared distances from this point to this box
void compute_best_worst_sq_distances(
	const cCollisionAABBBox& box, const cVector3d& p, double& best_sq, double& worst_sq) {

	// Best and worst _squared_ distances
	best_sq = 0;
	worst_sq = 0;

	double best_dist = 0;
	double worst_dist = 0;

	// Temporary variable to simplify squarings
	// double d;

	//// Check x distances 

	for (register unsigned int k = 0; k < 3; k++) {

		// If I'm below the x range, my best x distance uses
		// the minimum x, and my worst uses the maximum x
		if (p[k] < box.m_min[k]) {
			best_dist += box.m_min[k] - p[k];
			// best_sq += d*d;

			worst_dist += box.m_max[k] - p[k];
			// worst_sq += d*d;
		}

		// If I'm above the x range, my best x distance uses
		// the maximum x, and my worst uses the minimum x
		else if (p[k] > box.m_max[k]) {
			best_dist += p[k] - box.m_max[k];
			// best_sq += d*d;

			worst_dist += p[k] - box.m_min[k];
			// worst_sq += d*d;
		}

		// If I'm _in_ the x range, x doesn't affect my best
		// distance, and my worst-case distance goes to the
		// _farther_ of the two x distances
		else {
			// best_sq += 0;
			double dmin = fabs(box.m_min[k] - p[k]);
			double dmax = fabs(box.m_max[k] - p[k]);
			double d_worst = (dmin > dmax) ? dmin : dmax;
			worst_dist += d_worst;
			// worst_sq += d_worst*d_worst;
		}

	}

	best_sq = best_dist * best_dist;
	worst_sq = worst_dist * worst_dist;

}

int g_mesh_list_initialized = 0;
std::list<cCollisionAABBNode*> active_boxes;
int g_vector_initialized = 0;

cCollisionAABBNode* last_hit = 0;

int g_iteration_number = 0;

double AABB_distance_computer::closest_vertex(cMesh* mesh,
	cVector3d p, cTriangle* closest_tri, cVector3d* closest_point, const int reinit_mesh_list) {

	double closest_sq_distance = DBL_MAX;

	// Convert the point to object space  
	p.sub(mesh->getPos());

	// This will hold all the parents we're still searching...
	std::list<cMesh*> meshes_to_descend;

	static std::list<cCollisionAABB*> colliders;

	if (g_mesh_list_initialized == 0 || reinit_mesh_list) {

		g_mesh_list_initialized = 1;
		g_vector_initialized = 0;
		active_boxes.clear();
		last_hit = 0;
		colliders.clear();

		meshes_to_descend.push_front(mesh);

		// While there are still parent meshes to process
		while (meshes_to_descend.empty() == 0) {

			// Grab the next parent
			cMesh* cur_mesh = meshes_to_descend.front();
			meshes_to_descend.pop_front();

			cCollisionAABB* collider = (cCollisionAABB*)(cur_mesh->getCollisionDetector());
			if (collider) colliders.push_front(collider);

			// Put all his children on the list of parents to process
			for (unsigned int i = 0; i < cur_mesh->getNumChildren(); i++) {

				cGenericObject* cur_object = cur_mesh->getChild(i);

				// Only process cMesh children
				cMesh* child_mesh = dynamic_cast<cMesh*>(cur_object);
				if (child_mesh) meshes_to_descend.push_back(child_mesh);
			}
		}
	}

	std::list<cCollisionAABB*>::iterator collider_iter;
	for (collider_iter = colliders.begin(); collider_iter != colliders.end();
		collider_iter++) {
		cCollisionAABB* collider = (*collider_iter);
		double dsq = closest_vertex_to_single_mesh(collider, p, closest_tri, closest_point);
		if (dsq < closest_sq_distance) closest_sq_distance = dsq;
	}

	if (closest_sq_distance < g_smallest_distance) g_smallest_distance = closest_sq_distance;
	if (closest_sq_distance > g_largest_distance)  g_largest_distance = closest_sq_distance;

	// Convert back to global space
	if (closest_point) {
		(*closest_point).add(mesh->getPos());
	}

	return sqrt(closest_sq_distance);

}

double AABB_distance_computer::closest_vertex_to_single_mesh(cCollisionAABB* collider,
	cVector3d p, cTriangle* closest_tri, cVector3d* closest_point) {

	g_iteration_number++;

	cTriangle local_closest_tri(0, 0, 0, 0);
	if (closest_tri == 0) closest_tri = &local_closest_tri;

	// Convert the point to object space
	//
	// Now done at the highest level...
	// p.sub(mesh->getPos());

	double best_sq_vertex_distance = DBL_MAX;

	// The best worst-case that I've encountered for any box so far
	// (squared)
	double best_worst_case_sq = DBL_MAX;

	if (g_vector_initialized == 0) {
		//g_vector_initialized = 1;
		//active_boxes.reserve(5000);
	}

	cCollisionAABBNode* root = collider->getRoot();

	if (root == 0) return DBL_MAX;

	active_boxes.push_front(root);

	// This should end up in front of the root on the list; 
	// we want to prune away as many of the root's descendants as
	// we can.
	if (last_hit && g_last_hit_steps >= 0) {

		for (register int i = 0; i < g_last_hit_steps; i++) {
			cCollisionAABBNode* tmp = last_hit->m_parent;
			if (tmp == 0) break;
			last_hit = last_hit->m_parent;
		}

		active_boxes.push_front(last_hit);

		last_hit = 0;
	}

	while (active_boxes.empty() == 0) {

		cCollisionAABBNode* cur_node = active_boxes.front();
		active_boxes.pop_front();

		// Do we care to process this node?
		// cCollisionAABBBox bbox = cur_node->m_bbox;

		// The best and worst _squared_ distances
		double worst_case_sq = 0;
		double best_case_sq = DBL_MAX;

		// Compute the best and worst cases for this box
		compute_best_worst_sq_distances(cur_node->m_bbox, p, best_case_sq, worst_case_sq);

		// If my best case is already worse than a known worst case, skip me...
		if (best_case_sq > best_worst_case_sq) continue;

		// Okay, I'm valid

		// Check whether I'm the new 'best worst case'
		if (worst_case_sq < best_worst_case_sq) best_worst_case_sq = worst_case_sq;

		// If I'm a leaf, check my vertices for closest points...
		cCollisionAABBLeaf* leaf = dynamic_cast<cCollisionAABBLeaf*>(cur_node);

		if (leaf) {

			// Find the closest point on this triangle...
			Wml::Vector3<double> v(p.x, p.y, p.z);

			cVector3d v0 = leaf->m_triangle->getVertex(0)->getPos();
			cVector3d v1 = leaf->m_triangle->getVertex(1)->getPos();
			cVector3d v2 = leaf->m_triangle->getVertex(2)->getPos();

			Wml::Triangle3<double> tri;
			Wml::Vector3<double> p0(v0.x, v0.y, v0.z);
			Wml::Vector3<double> e0(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
			Wml::Vector3<double> e1(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

			tri.Origin() = p0;
			tri.Edge0() = e0;
			tri.Edge1() = e1;

			double s, t;

			double dsq = Wml::SqrDistance<double>(v, tri, &s, &t);

			// Now the squared distance is in dsq, and s and t are parameters
			// interpolating between p0 and the hit point...

			// Is this the best hit we've had so far?
			if (dsq < best_sq_vertex_distance) {

				last_hit = leaf;
				best_sq_vertex_distance = dsq;

				// All worst-case distances should have to beat me from now on...
				best_worst_case_sq = dsq;

				// Record the closest point so far if we were asked
				// to do that...
				if (closest_point) {
					closest_point->x = p0.X() + s*e0.X() + t*e1.X();
					closest_point->y = p0.Y() + s*e0.Y() + t*e1.Y();
					closest_point->z = p0.Z() + s*e0.Z() + t*e1.Z();
				}

				*closest_tri = *leaf->m_triangle;

			}

		}

		// Otherwise cue up my children
		else {

			cCollisionAABBInternal* internal = dynamic_cast<cCollisionAABBInternal*>(cur_node);

			if (g_depth_first_search) {
				if (internal->m_leftSubTree) active_boxes.push_front(internal->m_leftSubTree);
				if (internal->m_rightSubTree) active_boxes.push_front(internal->m_rightSubTree);
			}
			else {
				if (internal->m_leftSubTree) active_boxes.push_back(internal->m_leftSubTree);
				if (internal->m_rightSubTree) active_boxes.push_back(internal->m_rightSubTree);
			}

		}

	}

	return best_sq_vertex_distance;

}


void CvoxelizerApp::check_mesh() {

	std::list<cMesh*> mesh_stack;

	mesh_stack.push_back(object_to_voxelize);

	while (mesh_stack.empty() == 0) {

		cMesh* curmesh = mesh_stack.front();
		mesh_stack.pop_front();

		if (curmesh->getParentWorld() == 0) {
			_cprintf("oops\n");
		}

		for (unsigned int i = 0; i < curmesh->getNumChildren(); i++) {
			cMesh* child = (cMesh*)(curmesh->getChild(i));
			mesh_stack.push_back(child);
		}

	}

}