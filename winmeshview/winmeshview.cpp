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

#define ALLOCATE_SCOPED_GLOBALS
#include "winmeshview_globals.h"
#undef ALLOCATE_SCOPED_GLOBALS

#include "winmeshview.h"
#include "winmeshviewDlg.h"
#include <conio.h>
#include <process.h>
#include "tetgen_loader.h"
#include "cTetMesh.h"
#include "meshImporter.h"
#include "CCollisionBrute.h"
#include "CImageLoader.h"
#include "CPhantom3dofPointer.h"
#include "meshExporter.h"

#ifndef M_PI
#define M_PI 3.1415926535898
#endif

// Turn off annoying compiler warnings
#pragma warning(disable: 4244)
#pragma warning(disable: 4305)
#pragma warning(disable: 4800)

#define DEFAULT_MODEL_FILENAME "test_models/can.obj"

// How far is the camera from its rotation point?
#define CAMERA_FOCAL_DISTANCE 4.0

// See CHAI object_loader example for details
// #define USE_MM_TIMER_FOR_HAPTICS
#define USE_PHANTOM_DIRECT_IO 1

/*** object animation variables ***/

// Default initial position for the object that will appear in the main window
cVector3d g_initial_object_pos(1.5, 0, -1.2);

// The translational and rotational velocities (gl units/s and radians/s)
// at which the object is animated when the 'toggle animation' button gets
// clicked
#define INITIAL_OBJECT_X_VELOCITY -0.5
#define OBJECT_R_VELOCITY (M_PI / 4.0)

// This is the point where the object will "turn around"
// when he's being animated...
#define MAXIMUM_ANIMATION_XVAL 1.0

/*** ***/

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CwinmeshviewApp, CWinApp)
	//{{AFX_MSG_MAP(CwinmeshviewApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

#ifdef COMPILING_WINMESHVIEW
CwinmeshviewApp theApp;
#endif

// Load the model - if any - that was requested at the command line
void CwinmeshviewApp::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast) {
	if (m_current_param_index == 0) LoadModel((char*)pszParam);
	m_current_param_index++;
}

// Constructor
CwinmeshviewApp::CwinmeshviewApp() {

	m_render_on_gui_thread = true;
	m_loaded_mesh_filename[0] = '\0';

	current_mesh_transform.model_offset.set(0, 0, 0);
	current_mesh_transform.model_scale_factor = 1.0;

	m_current_param_index = 0;
	m_current_cut_plane = CUT_PLANE_NONE;
	m_invert_clip_plane = false;
	cutplane = 0;
	m_last_animation_time = -1.0;
	moving_object = 0;
	selected_object = 0;
	haptics_enabled = 0;
	tool = 0;
	label_panel = 0;

	AllocConsole();
	/*
	HWND con_wnd = GetConsoleWindow();
	// We want the console to pop up over any non-wmv windows, although
	// we'll put the main dialog window above it.
	::SetForegroundWindow(con_wnd);
	SetWindowPos(con_wnd,HWND_TOP,0,0,0,0,SWP_NOSIZE);
	*/

	_cprintf("Created winmeshview application...\n");
	g_main_app = this;

}


// Called by the destructor
void CwinmeshviewApp::uninitialize() {
	toggle_haptics(TOGGLE_HAPTICS_DISABLE);
	delete world;
	delete viewport;
}

int g_exiting_app = 0;

int CwinmeshviewApp::ExitInstance() {

	_cprintf("Exiting winmeshview instance...\n");

	if (g_main_dlg) {
		// ((CwinmeshviewDlg*)(g_main_dlg))->shutdown();    
		((CwinmeshviewDlg*)(g_main_dlg))->CloseWindow();
		g_exiting_app = 1;
		// Run();
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			_cprintf("Handling a message...\n");
			handle_message(msg);
		}
		_cprintf("Closed window...\n");

		delete g_main_dlg;
		g_main_dlg = 0;
	}

	FreeConsole();
	_cprintf("Exited winmeshview instance...\n");
	// _getch();
	return CWinApp::ExitInstance();
}


BOOL CwinmeshviewApp::InitInstance() {

	_cprintf("Initializing winmeshview instance...\n");

	memset(keys_to_handle, 0, sizeof(keys_to_handle));

#ifdef COMPILING_WINMESHVIEW
	AfxEnableControlContainer();
	CwinmeshviewDlg* dlg = new CwinmeshviewDlg;
	g_main_dlg = dlg;
	m_pMainWnd = dlg;
	dlg->Create(IDD_winmeshview_DIALOG, NULL);
#endif

	// Now we should have a display context to work with...

	world = new cWorld();

	// set background color
	world->setBackgroundColor(0.0f, 0.0f, 0.0f);
	//world->setBackgroundColor(1.0f,1.0f,1.0f);

	cutplane = new cPanel(world);
	cutplane->setDisplayEdges(true);
	cutplane->setDisplayRect(true);
	cutplane->enableTransparency(true, true);
	cutplane->m_rectColor.set(0.2, 0.2, 0.2, 0.1);
	cutplane->m_edgeColor.set(0.9, 0.9, 0.9, 1.0);
	cutplane->setSize(cVector3d(2.5, 2.5, 0.0));
	cutplane->setPos(0, 0, 0);

	// Create a camera
	camera = new cCamera(world);

	// This is not strictly necessary; it enables rendering the camera
	// (and children of the camera) as a visible object...
	world->addChild(camera);

	resetCamera();

#ifdef COMPILING_WINMESHVIEW
	// Create a display for graphic rendering
	viewport = new cViewport(g_main_dlg->m_gl_area_hwnd, camera, false);
	viewport->setPostRenderCallback(this);
#else
	viewport = 0;
#endif

	// Create a light source
	light = new cLight(world);


	// This causes the light to follow the camera...
	camera->addChild(light);

	light->setEnabled(true);

	// light->m_diffuse.set(0.3,0.3,0.3);
	// light->m_ambient.set(0.2,0.2,0.2);

	// A purely directional light, pointing into the monitor
	// and a little to the left... note that to accomplish
	// that, I "place" the light off to the left and behind the
	// origin (positive z relative to the origin).
	light->setDirectionalLight(true);
	light->setPos(cVector3d(-1, 0, 1));

	object = 0;

	m_factory_mesh = new cVBOMesh(world);

#ifdef COMPILING_WINMESHVIEW

	loadCommandLineModel();
	if (object == 0) createDefaultModel();
	if (object == 0) _cprintf("Could not load command line or default object...\n");

#endif

	_cprintf("Initialized winmeshview instance...\n");

	return true;
}


bool CwinmeshviewApp::loadCommandLineModel() {

	// Create a mesh - we will build a cube manually, and later let the
	// user load 3d models
	if (m_lpCmdLine[0] != '\0') {

		// This will load a model if the command line says so...
		ParseCommandLine(*this);
		if (object) return true;
	}

	return false;

}


bool CwinmeshviewApp::createDefaultModel() {

	// Load a default model...  
	if (object == 0) {
		bool result = LoadModel(DEFAULT_MODEL_FILENAME);
		if (result == false) {
			_cprintf("Could not load default model %s\n", DEFAULT_MODEL_FILENAME);
		}

	}

	if (object == 0) {
		object = new cMesh(world);
		world->addChild(object);
		double size = 1.0;
		// Create a nice little cube
		createCube(object, size);

	}

	// Useful for testing the panel functionality...
	/*
	cTetGenLoader tgl;
	cLabelPanel* panel = new cLabelPanel(world);
	label_panel = panel;
	camera->m_front_2Dscene.addChild(label_panel);
	panel->addLabel("Hello",tgl.default_materials[0]);
	panel->layout();
	*/

	world->computeGlobalPositions(false);

	// This will set rendering and haptic options as they're defined in
	// the GUI...
	if (g_main_dlg) g_main_dlg->update_rendering_options_from_gui(this);

	return true;
}


void CwinmeshviewApp::resetCamera() {

	// set camera position and orientation
	// 
	// We choose to put it out on the positive z axis, so things appear
	// the way OpenGL users expect them to appear, with z going in and
	// out of the plane of the screen (-z inward).
	int result = camera->set(
		cVector3d(0, 0, CAMERA_FOCAL_DISTANCE),           // position of camera
		cVector3d(0.0, 0.0, 0.0),   // camera looking at origin
		cVector3d(0.0, 1.0, 0.0)  // orientation of camera (standing up)
	);

	if (result == 0) {
		_cprintf("Could not create camera...\n");
	}

	world->computeGlobalPositions(true);
}


void CwinmeshviewApp::handle_message(MSG& msg) {

	int message_processed = PreTranslateMessage(&msg);

	if (message_processed == 0) TranslateMessage(&msg);

	// Record any keystrokes that need to be handled by the rendering
	// or haptics loops...
	if (msg.message == WM_KEYDOWN) {
		keys_to_handle[msg.wParam] = 1;
	}

	else if (msg.message == WM_KEYUP) {
		keys_to_handle[msg.wParam] = 0;
	}

	else if (msg.message == MESSAGE_UPDATE) {
		((CwinmeshviewDlg*)(g_main_dlg))->UpdateData(false);
	}

	if (message_processed == 0) DispatchMessage(&msg);

	/*
	if (msg.message == WM_MOUSELEAVE || msg.message == WM_NCMOUSELEAVE) {
	//g_main_dlg->OnRButtonUp(0,CPoint(0,0));
	//g_main_dlg->OnLButtonUp(0,CPoint(0,0));
	//g_main_dlg->OnMButtonUp(0,CPoint(0,0));
	g_main_dlg->m_trackingMouse = false;
	//_cprintf("Mouse leave\n");
	}
	*/
}


#define INI_FILENAME "winmeshview.ini"

void CwinmeshviewApp::process_ini_file() {

	FILE* f = fopen(INI_FILENAME, "r");
	if (f == 0) {
		_cprintf("Could not open ini file %s\n", INI_FILENAME);
		return;
	}

	char buf[5000];

	while (1) {

		char* result = fgets(buf, 5000, f);
		if (result == 0) break;
		chop_newlines(buf);

		char* linestart = buf;
		while (*linestart == ' ' || *linestart == '\t') linestart++;
		if (*linestart == '#' || *linestart == '\n' || *linestart == '\0') continue;

		char* token = strtok(linestart, " ");

		if (strncmp(token, "LOAD", strlen("LOAD")) == 0) {
			token = strtok(0, " ");
			if (token == 0) {
				_cprintf("Could not read value for option LOAD\n");
				continue;
			}
			char* fname = token;
			LoadModel(fname);
			continue;
		}

		if (strncmp(token, "CONVERT", strlen("CONVERT")) == 0) {
			token = strtok(0, " ");
			if (token == 0) {
				_cprintf("Could not read conversion input filename\n");
				continue;
			}
			char* fname_in = token;

			token = strtok(0, " ");
			if (token == 0) {
				_cprintf("Could not read conversion output filename\n");
				continue;
			}
			char* fname_out = token;

			LoadModel(fname_in);
			int result =
				ExportModel(g_main_app->object, fname_out, &(g_main_app->current_mesh_transform));

			if (result >= 0)
				_cprintf("Converted %s to %s\n", fname_in, fname_out);
			else
				_cprintf("Error converting %s to %s\n", fname_in, fname_out);

			continue;
		}

		_cprintf("Unrecognized token: %s\n", token);

	} // for each line in the file

	fclose(f);

}



int CwinmeshviewApp::Run() {

	MSG msg;

	// Position the console window so it looks nice...
	HWND con_wnd = GetConsoleWindow();
	RECT r;
	::GetWindowRect(con_wnd, &r);

	// Move the real window up to the front
	::SetWindowPos(m_pMainWnd->m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE);
	::SetForegroundWindow(m_pMainWnd->m_hWnd);

	process_ini_file();

	// Loop forever looking for messages and rendering...
	while (1) {

		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) == TRUE) {

			if (GetMessage(&msg, NULL, 0, 0)) {

				handle_message(msg);

			}

			// Quit if GetMessage(...) fails
			else return TRUE;

		}

		if (m_render_on_gui_thread && (g_exiting_app == 0)) render_loop();

		// We can sleep (yield) here to be extra well-behaved, not necessary
		// for a game-like app that is happy to eat CPU
		// Sleep(1);

	}

} // Run()


void CwinmeshviewApp::render(const int a_renderMode) {

}


// Our main drawing loop...
int CwinmeshviewApp::render_loop() {

	// Perform animation if the haptics thread isn't running but
	// we're supposed to be moving the object
	if (haptics_thread_running == 0 && moving_object) {

		animate();

	}

	int old_culling = object->getCullingEnabled();

	// Turn on cut planes if necessary...
	if (m_current_cut_plane != CUT_PLANE_NONE) {

		cutplane->setPos(0, 0, 0);
		cutplane->setRot(cIdentity3d());

		object->useCulling(false, true);
		double peqn[4] = { 0,0,0,0 };

		double plane_position = m_current_cut_plane_position;

		switch (m_current_cut_plane) {
		case CUT_PLANE_XY:
			peqn[2] = -1.0;
			plane_position *= 5.0;
			plane_position -= 2.0;
			peqn[3] = plane_position;
			if (m_invert_clip_plane) {
				peqn[3] *= -1.0;
				peqn[2] *= -1.0;
			}

			// cutplane is already at default rotation
			cutplane->setPos(0, 0, plane_position);

			break;
		case CUT_PLANE_XZ:
			peqn[1] = -1.0;
			peqn[3] = plane_position;
			if (m_invert_clip_plane) {
				peqn[3] *= -1.0;
				peqn[1] *= -1.0;
			}

			cutplane->rotate(cVector3d(1, 0, 0), -M_PI / 2.0);
			cutplane->setPos(0, plane_position, 0);

			break;
		case CUT_PLANE_YZ:
			plane_position *= 2.0;
			peqn[0] = -1.0;
			peqn[3] = plane_position;
			if (m_invert_clip_plane) {
				peqn[3] *= -1.0;
				peqn[0] *= -1.0;
			}

			cutplane->rotate(cVector3d(0, 1, 0), -M_PI / 2.0);
			cutplane->setPos(plane_position, 0, 0);

			break;
		}

#define USE_CHAI_CLIP_PLANE
		//#define USE_LOCAL_CLIP_PLANE

#define LOCAL_CLIP_PLANE_NUMBER 0
#define CHAI_CLIP_PLANE_NUMBER 0

#ifdef USE_CHAI_CLIP_PLANE
		camera->enableClipPlane(CHAI_CLIP_PLANE_NUMBER, 1, peqn);
#endif
#ifdef USE_LOCAL_CLIP_PLANE
		camera->enableClipPlane(LOCAL_CLIP_PLANE_NUMBER, -1, peqn);
		glEnable(GL_CLIP_PLANE0 + LOCAL_CLIP_PLANE_NUMBER);
		glClipPlane(GL_CLIP_PLANE0 + LOCAL_CLIP_PLANE_NUMBER, peqn);
#endif

		if (world->containsChild(cutplane, false) == false) {
			world->addChild(cutplane);
		}
	}
	else {

#ifdef USE_CHAI_CLIP_PLANE
		camera->enableClipPlane(CHAI_CLIP_PLANE_NUMBER, 0);
#endif
#ifdef USE_LOCAL_CLIP_PLANE
		camera->enableClipPlane(LOCAL_CLIP_PLANE_NUMBER, -1);
		glDisable(GL_CLIP_PLANE0 + LOCAL_CLIP_PLANE_NUMBER);
#endif

		world->removeChild(cutplane);
	}

	// If the viewport is rendering in stereo, it renders a full pair here...
	viewport->render();

	if (m_current_cut_plane != CUT_PLANE_NONE)
		object->useCulling(old_culling, true);

	return 0;

}


// Called by the GUI when the user clicks the "load model"
// button.  Opens the specified file and displays the model (graphically
// and haptically).
bool CwinmeshviewApp::LoadModel(const char* filename,
	bool build_collision_detector, bool finalize, bool delete_old_model) {

	cMesh* new_object = 0;
	cLabelPanel* new_label_panel = 0;

	char selected_filename[MAX_PATH];

	bool result = importModel(filename, new_object, world,
		build_collision_detector, finalize, m_factory_mesh,
		&new_label_panel, XFORMOP_AUTO, &current_mesh_transform, selected_filename);

	if (result == false) return false;

	strcpy(m_loaded_mesh_filename, selected_filename);

	// Replace the old object we're displaying with the
	// new one
	//
	// Really I might synchronize this operation with the
	// haptic thread - if there is one - but that's beyond the scope
	// of this demo.
	if (object) {
		world->removeChild(object);
		if (delete_old_model) delete object;
		else _cprintf("Not deleting old model...\n");
	}

	object = new_object;
	world->addChild(object);

	// Copy relevant rendering variables back to the GUI
	if (g_main_dlg) g_main_dlg->copy_rendering_options_to_gui(this);

	// This will set up default rendering options and haptic properties
	//
	// Don't over-write things we loaded from the file...
	if (g_main_dlg) g_main_dlg->update_rendering_options_from_gui(this, 1);

	if (label_panel) {
		camera->m_front_2Dscene.removeChild(label_panel);
		delete label_panel;
	}
	label_panel = 0;

	if (new_label_panel) {
		camera->m_front_2Dscene.addChild(new_label_panel);
		label_panel = new_label_panel;
		label_panel->setBorders(10, 10, 10, 10, 10);
		label_panel->layoutImmediately();
		cVector3d s = label_panel->getSize();
		s /= 2.0;
		s += cVector3d(10, 10, 0);
		label_panel->setPos(s);
	}

	// This fixes a problem with loading models while a cut plane
	// is enabled.
	world->removeChild(cutplane);
	glDisable(GL_CLIP_PLANE0);

	return true;

}


// Applies a texture - loaded from the specified file -
// to the current model, if texture coordinates are
// defined.
int CwinmeshviewApp::LoadTexture(char* filename) {

	cTexture2D *newTexture = world->newTexture();
	int result = newTexture->loadFromFile(filename);

	if (result == 0) {
		_cprintf("Could not load texture file %s\n", filename);
		delete newTexture;
		return -1;
	}

	object->setTexture(newTexture, 1);
	object->useTexture(1, 1);

	return 0;
}


// Called when the camera zoom slider changes
void CwinmeshviewApp::zoom(int zoom_level) {

	//camera->setPos(p.x,((float)(zoom_level))/100.0*10.0,p.z);
	camera->setFieldViewAngle(((float)(zoom_level)));

}


// Called when the user moves the mouse in the main window
// while a button is held down
void CwinmeshviewApp::scroll(CPoint p, int button) {

	int shift_pressed = GetAsyncKeyState(VK_SHIFT) & (1 << 15);

	// Get the current camera vectors
	cVector3d up = camera->getUpVector();
	cVector3d right = camera->getRightVector();
	cVector3d look = camera->getLookVector();
	cVector3d camp = camera->getPos();
	cVector3d lookp = camp - CAMERA_FOCAL_DISTANCE*look;

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

		// If shift is pressed, rotate around the camera's current
		// position    
		if (shift_pressed) {

		}

		// Move the camera too if we're not rotating around the
		// camera center
		else {
			cVector3d new_camp = lookp + new_look*CAMERA_FOCAL_DISTANCE;
			camera->setPos(new_camp);
		}

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

		object->computeGlobalPositions(true);
		// object->computeBoundaryBox(true);    
	}

}


// Called when the user clicks his mouse in the main window
//
// Lets CHAI figure out which object was clicked on.
void CwinmeshviewApp::select(CPoint p) {

	if (viewport->select(p.x, p.y, true)) {
		selected_object = viewport->getLastSelectedObject();
		selected_triangle = viewport->getLastSelectedTriangle();
	}
	else {
		selected_object = 0;
		selected_triangle = 0;
	}

}


// Our haptic loop... just computes forces on the 
// phantom every iteration, until haptics are disabled
// in the supplied CwinmeshviewApp

// A single iteration through the loop...
void winmeshview_haptic_iteration(void* param) {

	CwinmeshviewApp* app = (CwinmeshviewApp*)(param);

	app->animate();

	app->tool->updatePose();
	app->tool->computeForces();
	app->tool->applyForces();

}


// We use this in our "animation" routine...
inline double sgn(const double& a) {
	if (a >= 0) return 1.0;
	return -1.0;
}


void CwinmeshviewApp::animate() {

	if (object == 0) return;

	// The A and D keys can be used to move the current object around
	if (keys_to_handle['A']) {
		object->translate(-0.02, 0, 0);
		object->computeGlobalPositions(1);
		keys_to_handle['A'] = 0;
	}
	if (keys_to_handle['D']) {
		object->translate(0.02, 0, 0);
		object->computeGlobalPositions(1);
		keys_to_handle['D'] = 0;
	}

	cPrecisionClock clock;
	double curtime = clock.getCPUtime();

	if (moving_object && m_last_animation_time >= 0) {

		double elapsed = curtime - m_last_animation_time;

		// Move the object...
		cVector3d delta = cMul(elapsed, m_animation_velocity);
		object->translate(delta);

		double r_delta = elapsed * OBJECT_R_VELOCITY;
		object->rotate(cVector3d(0, 1, 0), r_delta);

		// This is necessary to allow collision detection to work
		// after object movement
		object->computeGlobalPositions(1);
		// object->computeBoundaryBox(true);

		// Turn our velocity around if we reach the end of the animation
		// space...
		if (
			(fabs(object->getPos().x) > MAXIMUM_ANIMATION_XVAL)
			&&
			(sgn(m_animation_velocity.x) == sgn(object->getPos().x))
			)
		{

			m_animation_velocity[0] = m_animation_velocity[0] * -1.0;

		} // if we need to turn around

	} // if we're animating our object

	m_last_animation_time = curtime;

}


void CwinmeshviewApp::toggle_animation() {

	if (object == 0) return;

	// Disable animation
	if (moving_object) {
		moving_object = 0;
		object->setPos(g_initial_object_pos);
		object->computeGlobalPositions(1);
	}

	// Enable animation
	else {
		moving_object = 1;
		m_last_animation_time = -1.0;

		// Put the object at the zero position...
		object->setPos(g_initial_object_pos);

		// And reset the animation velocity to its initial
		// default...
		m_animation_velocity = cVector3d(INITIAL_OBJECT_X_VELOCITY, 0, 0);
	}

}


// This loop is used only in the threaded version of this
// application... all it does is call the main haptic
// iteration loop, which is called directly from a timer
// callback if USE_MM_TIMER_FOR_HAPTICS is defined
DWORD winmeshview_haptic_loop(void* param) {

	CwinmeshviewApp* app = (CwinmeshviewApp*)(param);

	while (app->haptics_enabled) {

		winmeshview_haptic_iteration(param);

	}

	app->haptics_thread_running = 0;

	return 0;
}


void CwinmeshviewApp::reinitialize_viewport(int stereo_enabled) {

	// TODO: this doesn't account for the possibility of a non-existant GUI
	if (viewport) delete viewport;

	viewport = new cViewport(g_main_dlg->m_gl_area_hwnd, camera, stereo_enabled);

	_cprintf("Stereo rendering is %s\n", viewport->getStereoOn() ? "enabled" : "disabled");

}


/***

  Enable or disable haptics; called when the user clicks
  the enable/disable haptics button.  The "enable" parameter
  is one of :

  #define TOGGLE_HAPTICS_TOGGLE  -1
  #define TOGGLE_HAPTICS_DISABLE  0
  #define TOGGLE_HAPTICS_ENABLE   1

***/
void CwinmeshviewApp::toggle_haptics(int enable) {

	if (enable == TOGGLE_HAPTICS_TOGGLE) {

		if (haptics_enabled) toggle_haptics(TOGGLE_HAPTICS_DISABLE);
		else toggle_haptics(TOGGLE_HAPTICS_ENABLE);

	}

	else if (enable == TOGGLE_HAPTICS_ENABLE) {

		if (haptics_enabled) return;

		// Build a collision detector if I haven't already
		cGenericCollision* cd = object->getCollisionDetector();
		if (cd == 0 || (dynamic_cast<cCollisionBrute*>(cd))) {
			object->deleteCollisionDetector();
			_cprintf("Building collision detector...\n");
			object->createAABBCollisionDetector(true, true);
			_cprintf("Finished building collision detector...\n");
		}
		haptics_enabled = 1;

		// create a phantom tool with its graphical representation
		//
		// Use device zero, and use either the gstEffect or direct 
		// i/o communication mode, depending on the USE_PHANTOM_DIRECT_IO
		// constant
		if (tool == 0) {

			tool = new cPhantom3dofPointer(world, 0, USE_PHANTOM_DIRECT_IO);

			// The tool is rendered as a child of the camera.
			//
			// This has the nice property of always aligning the Phantom's
			// axes with the camera's axes.
			camera->addChild(tool);
			tool->setPos(-4.0, 0.0, 0.0);

			// This is what we would do if we _didn't_ want the tool to
			// move around as a child of the camera
			//world->addChild(tool);
			// Rotate the tool so its axes align with our opengl-like axes
			//tool->rotate(cVector3d(0,0,1),-90.0*M_PI/180.0);
			//tool->rotate(cVector3d(1,0,0),-90.0*M_PI/180.0);

			// set up a nice-looking workspace for the phantom so 
			// it fits nicely with our models
			tool->setWorkspace(2.0, 2.0, 2.0);
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

		// Make sure the haptic device knows where he is in the camera frame
		world->computeGlobalPositions(true);

#ifdef USE_MM_TIMER_FOR_HAPTICS

		// start the mm timer to run the haptic loop
		timer.set(0, winmeshview_haptic_iteration, this);

#else

		// start haptic thread
		haptics_thread_running = 1;

		DWORD thread_id;
		::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)(winmeshview_haptic_loop), this, 0, &thread_id);

		// Boost thread and process priority
		::SetThreadPriority(&thread_id, THREAD_PRIORITY_ABOVE_NORMAL);
		//::SetPriorityClass(GetCurrentProcess(),ABOVE_NORMAL_PRIORITY_CLASS);

#endif

	} // enabling

	else if (enable == TOGGLE_HAPTICS_DISABLE) {

		// Don't do anything if haptics are already off
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
	float radius = edge / 2.0;
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

	// Give a color to each vertex
	for (unsigned int i = 0; i < mesh->getNumVertices(); i++) {

		cVertex* nextVertex = mesh->getVertex(i);

		cColorb color;
		color.set(
			GLuint(0xff * (edge + nextVertex->getPos().x) / (2.0 * edge)),
			GLuint(0xff * (edge + nextVertex->getPos().y) / (2.0 * edge)),
			GLuint(0xff * nextVertex->getPos().z / 2 * edge)
		);

		nextVertex->setColor(color);
	}

	// Set object settings.  The parameters tell the object
	// to apply this alpha level to his textures and to his
	// children (of course he has neither right now).
	// object->setTransparencyLevel(0.5, true, true);

	// Give him some material properties...
	cMaterial material;

	material.m_ambient.set(0.6, 0.3, 0.3, 1.0);
	material.m_diffuse.set(0.8, 0.6, 0.6, 1.0);
	material.m_specular.set(0.9, 0.9, 0.9, 1.0);
	material.setShininess(100);
	mesh->m_material = material;

	cMesh* object = mesh;

	// Move the object over some so the Phantom will not initially be
	// inside the object.
	object->translate(g_initial_object_pos);
	object->rotate(cVector3d(0, 1, 0), 45.0 * 3.14159 / 180.0);

	object->computeGlobalPositions();
	object->computeBoundaryBox(true);

	// Note that I don't bother to build a fancy collision detector
	// for this cube, since the brute force collision detector -
	// which is the default - works fine for using the haptic proxy
	// on a simple shape with only a few triangles.
	//
	// I will create a fancy collision detector later when I load
	// a model from a file.

	// These two calls are useful for debugging, but not
	// really important...

	// Tells the object to compute a boundary box around
	// himself and all his children.  We do this here just so
	// he can render his bounding box for your viewing
	// pleasure.
	object->computeBoundaryBox(true);

	// Set the size of the coordinate frame that we're
	// going to gratuitously render
	object->setFrameSize(1.0, false);

	// Compute normals for each triangle
	// object->computeAllNormals();  
}
