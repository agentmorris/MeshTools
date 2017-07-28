//===========================================================================
/*
    This file is part of the CHAI 3D visualization and haptics libraries.
    Copyright (C) 2003-2004 by CHAI 3D. All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License("GPL") version 2
    as published by the Free Software Foundation.

    For using the CHAI 3D libraries with software that can not be combined
    with the GNU GPL, and for taking advantage of the additional benefits
    of our support services, please contact CHAI 3D about acquiring a
    Professional Edition License.

    \author:    <http://www.chai3d.org>
    \author:    Dan Morris
    \version    1.0
    \date       06/2004
*/
//===========================================================================

#include "stdafx.h"
#include "mass_springs.h"
#include "mass_springsDlg.h"
#include <conio.h>
#include <process.h>

#ifndef M_PI
#define M_PI 3.1415926535898
#endif

// See the previous CHAI examples for information about
// these constants
#define USE_MM_TIMER_FOR_HAPTICS
#define USE_PHANTOM_DIRECT_IO 1

#define ALLOCATE_SCOPED_GLOBALS
#include "mass_springs_globals.h"
#undef ALLOCATE_SCOPED_GLOBALS

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Turn off annoying compiler warnings
#pragma warning(disable: 4305)
#pragma warning(disable: 4800)

BEGIN_MESSAGE_MAP(Cmass_springsApp, CWinApp)
	//{{AFX_MSG_MAP(Cmass_springsApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// No one really knows why GetConsoleWindow() is not
// properly exported by the MS include files.
extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow (); 

Cmass_springsApp theApp;

Cmass_springsApp::Cmass_springsApp() {

  addball_pending = removeball_pending = 0;

  m_floor = 0;
  m_floor_spring_constant = DEFAULT_FLOOR_SPRING_CONSTANT;

  selected_object = 0;

  haptics_enabled = 0;

  tool = 0;

  AllocConsole();

  HWND con_wnd = GetConsoleWindow();

  // We want the console to pop up over any non-CHAI windows, although
  // we'll put the main dialog window above it.
  ::SetForegroundWindow(con_wnd);
  
  SetWindowPos(con_wnd,HWND_TOP,0,0,0,0,SWP_NOSIZE);

  g_main_app = this;
}


void Cmass_springsApp::uninitialize() {

  toggle_haptics(TOGGLE_HAPTICS_DISABLE);
  delete world;
  delete viewport;
  
}


BOOL Cmass_springsApp::InitInstance() {

	AfxEnableControlContainer();

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	g_main_dlg = new Cmass_springsDlg;
  m_pMainWnd = g_main_dlg;

  g_main_dlg->Create(IDD_mass_springs_DIALOG,NULL);
    
  // Now we should have a display context to work with...

  world = new cWorld();

  // set background color
  world->setBackgroundColor(0.0f,0.0f,0.0f);

  // Create a camera
  camera = new cCamera(world);

  // Turn on one light...
  light = new cLight(world);

  light->setEnabled(true);

  // Put the light somewhere that looks nice
  light->setDirectionalLight(true);
  light->setPos(cVector3d(-1, 1, 1));

  //light->setDir(-1,-1,1);
  // We don't want a spotlight...
  //light->setCutOffAngle(180.0);


  
  // set camera position and orientation
  // 
  // We choose to put it out on the positive z axis, so things appear
  // the way OpenGL users expect them to appear, with z going in and
  // out of the plane of the screen.
  int result = camera->set(
              cVector3d(0,0,4),           // position of camera
              cVector3d(0.0, 0.0, 0.0),   // camera looking at origin
              cVector3d(0.0, 1.0, 0.0));  // orientation of camera (standing up)

  if (result == 0) {
    _cprintf("Could not create camera...\n");
  }

  // Create a display for graphic rendering
  viewport = new cViewport(g_main_dlg->m_gl_area_hwnd, camera, false);

  // Create a mesh to represent the floor
  m_floor = new cMesh(world);
  world->addChild(m_floor);

  // Fill in meaningful vertex positions
  m_floor->newVertex(-FLOOR_X_SIZE/2.0,  FLOOR_Y_POSITION, -FLOOR_Z_SIZE/2.0);
  m_floor->newVertex(-FLOOR_X_SIZE/2.0,  FLOOR_Y_POSITION,  FLOOR_Z_SIZE/2.0);
  m_floor->newVertex( FLOOR_X_SIZE/2.0,  FLOOR_Y_POSITION,  FLOOR_Z_SIZE/2.0);
  m_floor->newTriangle(0,1,2);
  
  m_floor->newVertex( FLOOR_X_SIZE/2.0,  FLOOR_Y_POSITION, -FLOOR_Z_SIZE/2.0);
  m_floor->newVertex(-FLOOR_X_SIZE/2.0,  FLOOR_Y_POSITION, -FLOOR_Z_SIZE/2.0);
  m_floor->newVertex( FLOOR_X_SIZE/2.0,  FLOOR_Y_POSITION,  FLOOR_Z_SIZE/2.0);
  m_floor->newTriangle(3,4,5);

  for(int n=0; n<6; n++) {
    cVertex* curVertex = m_floor->getVertex(n);
    curVertex->setNormal(0,1,0);
  }

  // Give him some material properties...
  cMaterial material;  
  material.m_ambient.set( 0.4, 0.2, 0.2, 1.0 );
  material.m_diffuse.set( 0.8, 0.6, 0.6, 1.0 );
  material.m_specular.set( 0.9, 0.9, 0.9, 1.0 );
  material.setShininess(100);
  m_floor->m_material = material;

  // Create an initial ball
  CBall* b = new CBall();
  m_active_balls.push_back(b);

  // The first ball goes at -1,0,1 (arbitrary)
  cVector3d pos(-1,0,1);
  b->setPos(pos);
  world->addChild(b);

  // Create a series of masses connected by springs
  for(int i=1; i<INITIAL_NUM_BALLS; i++) {
    add_ball();
  }

  world->computeGlobalPositions(false);

  return TRUE;
}


// Add a mass to or remove a mass from the end of the chain
void Cmass_springsApp::add_ball() {

  // Put this ball to the right of the current last ball
  CBall* b = new CBall();
  m_active_balls.push_back(b);

  int ball_index = m_active_balls.size() - 1;

  CBall* neighbor = m_active_balls[ball_index - 1];

  // Create a position for each ball, moving from left to right
  // with increasing i
  cVector3d pos = neighbor->getPos();
  pos.add(INITIAL_BALL_SPACING,0,0);
  b->setPos(pos);
  world->addChild(b);

  CSpring* s = new CSpring();
  m_active_springs.push_back(s);

  // Springs are always "located" at 0,0,0; they render directly
  // using their endpoint ball positions...
  s->setPos(0,0,0);
  s->m_endpoint_1 = m_active_balls[ball_index];
  s->m_endpoint_2 = m_active_balls[ball_index-1];

  s->m_endpoint_1->m_springs.push_back(s);
  s->m_endpoint_2->m_springs.push_back(s);

  // Set the spring's rest length to be the initial distance between
  // the balls
  double d = cDistance(s->m_endpoint_1->getPos(),s->m_endpoint_2->getPos());
  s->m_rest_length = d;
  world->addChild(s);

}

void Cmass_springsApp::remove_ball() {

  // Can't remove the only ball
  if (m_active_balls.size() == 1) return;

  CBall* b = m_active_balls.back();
  m_active_balls.pop_back();
  delete b;

  CSpring* s = m_active_springs.back();
  m_active_springs.pop_back();
  delete s;

  world->removeChild(b);
  world->removeChild(s);

  // Find the last ball and remove his reference to this spring
  int ball_index = m_active_balls.size() - 1;
  b = m_active_balls[ball_index];

  std::vector<CSpring*>::iterator iter = b->m_springs.begin();
  for(; iter != b->m_springs.end(); iter++) {
    if (*iter == s) {
      b->m_springs.erase(iter);
      break;
    }
  }

}

int Cmass_springsApp::Run() {

  MSG msg;

  // Position the console window so it looks nice...
  HWND con_wnd = GetConsoleWindow();
  RECT r;
  ::GetWindowRect(con_wnd,&r);

  // Move the real window up to the front
  ::SetWindowPos(g_main_dlg->m_hWnd,HWND_TOP,0,r.bottom,0,0,SWP_NOSIZE);  
  ::SetForegroundWindow(g_main_dlg->m_hWnd);

  // Loop forever looking for messages and rendering...
  while (1) {

    while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) == TRUE) {

      if (GetMessage(&msg, NULL, 0, 0)) {

        TranslateMessage(&msg);

        if (msg.message == WM_KEYDOWN) {

        }

        else if (msg.message == WM_KEYUP) {
        
        }

        DispatchMessage(&msg);

      }

      // Quit if GetMessage(...) fails
      else return TRUE;

    }    
    
    render_loop();

    // Sleep to be extra well-behaved, not necessary for
    // a game-like app that is happy to hog the CPU
    // Sleep(1);

  }
  
} // Run()


// Our main drawing loop...
int Cmass_springsApp::render_loop() {

  // Perform integration here if the haptic loop isn't running...
  if (haptics_enabled == 0) {
    compute_spring_forces();
  }
  
  // Just draw the scene...
  viewport->render();

  return 0;

}


// Called when the camera zoom slider changes
void Cmass_springsApp::zoom(int zoom_level) {

  //camera->setPos(p.x,((float)(zoom_level))/100.0*10.0,p.z);
  camera->setFieldViewAngle(((float)(zoom_level)));
  
}


// Called when the user moves the mouse in the main window
// while a button is held down
void Cmass_springsApp::scroll(CPoint p, int left_button) {

  // If the user hasn't clicked on any objects, we don't
  // have to move or rotate anyone
  if (selected_object == 0) return;

  cGenericObject* object_to_move = selected_object;

  // If the left button is being held down, rotate the
  // selected object
  if (left_button) {

    cVector3d axis1(-1,0,0);
    object_to_move->rotate(axis1,-1.0*(float)p.y / 50.0);
  
    cVector3d axis2(0,1,0);
    object_to_move->rotate(axis2,(float)p.x / 50.0);

  }

  // If the left button is being held down, move the
  // selected object
  else {

    object_to_move->translate((float)p.x / 100.0, 0, 0);
    object_to_move->translate(0, -1.0*(float)p.y / 100.0, 0);

  }

  // Let the object re-compute his global position data
  object_to_move->computeGlobalPositions();
  object_to_move->computeBoundaryBox(true);  
  
}


// Called when the user clicks his mouse in the main window
//
// Lets CHAI figure out which object was clicked on.
void Cmass_springsApp::select(CPoint p) {

  if (viewport->select(p.x, p.y, true)) {
    selected_object = viewport->getLastSelectedObject();         

    // Don't allow movement of the floor...
    if (selected_object == m_floor) selected_object = 0;
  }
  else {    
    selected_object = 0;
  }

}


cPrecisionClock g_timer;
double g_last_iteration_time = -1.0;

// Called from the haptics loop if it's running (from the
// graphics loop otherwise) to compute and integrate forces
//
// Updates the position and velocity of all the masses
void Cmass_springsApp::compute_spring_forces() {

  if (addball_pending) {
    add_ball();
    addball_pending = 0;
  }

  if (removeball_pending) {
    remove_ball();
    removeball_pending = 0;
  }

  double curtime = g_timer.getCPUtime();
  
  if (g_last_iteration_time < 0) {
    g_last_iteration_time = curtime;
    return;
  }

  double elapsed = curtime - g_last_iteration_time;
  g_last_iteration_time = curtime;
  
  // Use a fixed timestep on the graphics thread...
  if (haptics_enabled == 0) {
    elapsed = 0.03;
  }

  unsigned int i;

  // Clear the force that's applied to each ball
  for(i=0; i<m_active_balls.size(); i++) {
    m_active_balls[i]->current_force.set(0,0,0);
  }

  if (haptics_enabled) {
    
    CBall* b = m_active_balls[0];
    cVector3d old_p = b->getPos();
    b->setPos(tool->m_deviceGlobalPos);
    b->m_velocity = cDiv(elapsed,cSub(b->getPos(),old_p));

  }

  // Compute the current length of each spring and apply forces
  // on each mass accordingly
  for(i=0; i<m_active_springs.size(); i++) {

    CSpring* s = m_active_springs[i];

    double d = cDistance(s->m_endpoint_1->getPos(),s->m_endpoint_2->getPos());
    s->m_current_length = d;

    // This spring's deviation from its rest length
    //
    // (positive deviation -> spring is too long)
    double x = s->m_current_length - s->m_rest_length;

    // Apply a force to ball 1 that pulls it toward ball 2
    // when the spring is too long
    cVector3d f1 = cMul(s->m_spring_constant*x*1.0,
      cSub(s->m_endpoint_2->getPos(),s->m_endpoint_1->getPos()));
    s->m_endpoint_1->current_force.add(f1);
    
    // Add the opposite force to ball 2
    s->m_endpoint_2->current_force.add(cMul(-1.0,f1));

  }
  
  // Update velocities and positions based on forces
  for(i=0; i<m_active_balls.size(); i++) {

    CBall* b = m_active_balls[i];
        
    // Certain forces don't get applied to the "haptic ball"
    // when haptics are enabled...
    if (haptics_enabled == 0 || i != 0) {
      cVector3d f_damping = cMul(DAMPING_CONSTANT,b->m_velocity);
      b->current_force.add(f_damping);
    }

    cVector3d f_gravity(0,GRAVITY_CONSTANT*b->m_mass,0);
    b->current_force.add(f_gravity);

    cVector3d p = b->getPos();
    
    if (p.y - b->m_radius < FLOOR_Y_POSITION) {
      double penetration = FLOOR_Y_POSITION - (p.y - b->m_radius);
      b->current_force.add(0,m_floor_spring_constant*penetration,0);
    }
    
    cVector3d f_floor(0,0,0);
    
    cVector3d a = cDiv(b->m_mass,b->current_force);
    
    b->m_velocity.add(cMul(elapsed,a));

    // We handle the 0th ball specially when haptics is enabled
    if (haptics_enabled == 0 || i != 0) {
      p.add(cMul(elapsed,b->m_velocity));
      b->setPos(p);
    }    

  }

  // Set the haptic force appropriately to reflect the force
  // applied to ball 0
  m_haptic_force = cMul(HAPTIC_FORCE_CONSTANT,m_active_balls[0]->current_force);
  

}

  
// Our haptic loop... just computes forces on the 
// phantom every iteration, until haptics are disabled
// in the supplied Cmass_springsApp

// A single iteration through the loop...
int g_haptic_iteration = 0;
void mass_springs_haptic_iteration(void* param) {

  Cmass_springsApp* app = (Cmass_springsApp*)(param);

  app->tool->updatePose();

  // Compute spring forces and integrate...
  app->compute_spring_forces();
  
  // Set the haptic force
  //
  // We use a magic flag so we can make the force zero on a first
  // pass, which pleases the tool class...
  if (g_haptic_iteration) app->tool->m_lastComputedGlobalForce = app->m_haptic_force;
  else app->tool->m_lastComputedGlobalForce.set(0,0,0);

  g_haptic_iteration++;

  app->tool->applyForces();

}


// This loop is used only in the threaded version of this
// application... all it does is call the main haptic
// iteration loop, which is called directly from a timer
// callback if USE_MM_TIMER_FOR_HAPTICS is defined
DWORD mass_springs_haptic_loop(void* param) {

  Cmass_springsApp* app = (Cmass_springsApp*)(param);

  while(app->haptics_enabled) {

    mass_springs_haptic_iteration(param);

  }

  app->haptics_thread_running = 0;

  return 0;
}


/***
 
  Enable or disable haptics; called when the user clicks
  the enable/disable haptics button.  The "enable" parameter
  is one of :

  #define TOGGLE_HAPTICS_TOGGLE  -1
  #define TOGGLE_HAPTICS_DISABLE  0
  #define TOGGLE_HAPTICS_ENABLE   1

***/
void Cmass_springsApp::toggle_haptics(int enable) {

  
  if (enable == TOGGLE_HAPTICS_TOGGLE) {

    if (haptics_enabled) toggle_haptics(TOGGLE_HAPTICS_DISABLE);
    else toggle_haptics(TOGGLE_HAPTICS_ENABLE);

  }

  else if (enable == TOGGLE_HAPTICS_ENABLE) {
  
    if (haptics_enabled) return;

    haptics_enabled = 1;

    g_haptic_iteration = 0;

    // create a phantom tool with its graphical representation
    //
    // Use device zero, and use either the gstEffect or direct 
    // i/o communication mode, depending on the USE_PHANTOM_DIRECT_IO
    // constant
    if (tool == 0) {

      // tool = new cPhantom3dofPointer(world, 0, USE_PHANTOM_DIRECT_IO);
      tool = new cMeta3dofPointer(world, 0, USE_PHANTOM_DIRECT_IO);

      world->addChild(tool);
    
      // set up the device
      tool->initialize();

      // set up a nice-looking workspace for the phantom so 
      // it fits nicely with our shape
      tool->setWorkspace(2.0,2.0,2.0);
    
      // Rotate the tool so its axes align with our opengl-like axes
      tool->rotate(cVector3d(0,0,1),-90.0*M_PI/180.0);
      tool->rotate(cVector3d(1,0,0),-90.0*M_PI/180.0);
      tool->setRadius(0.05);
      
      // We don't want to render the proxy for this application
      tool->setRenderingMode(RENDER_DEVICE);
    }
    
    // I need to call this so the tool can update its internal
    // transformations before performing collision detection, etc.
    tool->computeGlobalPositions();

    // Open communication with the device
    tool->start();      

    // Enable forces
    tool->setForcesON();

    // Tell the proxy algorithm associated with this tool to enable its
    // "dynamic mode", which allows interaction with moving objects
    
    // The dynamic proxy is in a pretty beta state, so we turn it off for now...
    // tool->getProxy()->enableDynamicProxy(1);
    
#ifdef USE_MM_TIMER_FOR_HAPTICS

    // start the mm timer to run the haptic loop
    timer.set(0,mass_springs_haptic_iteration,this);

#else

    // start haptic thread
    haptics_thread_running = 1;

    DWORD thread_id;
    ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)(mass_springs_haptic_loop), this, 0, &thread_id);

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
    while(haptics_thread_running) Sleep(1);

#endif
    
    // Stop the haptic device...
    tool->setForcesOFF();
    tool->stop();
    
    // SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);    

  } // disabling

} // toggle_haptics()


// Method definitions for the mass/spring objects
CSpring::CSpring() {

  m_radius = 0.05;
  m_rest_length = 1.0;
  m_spring_constant = DEFAULT_SPRING_CONSTANT;

  m_endpoint_1 = 0;
  m_endpoint_2 = 0;

}
 

CSpring::~CSpring() {

}


// Render the spring graphically
void CSpring::render(const int a_renderMode) {

  if (m_endpoint_1 == 0 || m_endpoint_2 == 0) return;

  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
  glColor3f(0.2,0.2,0.8);

  glLineWidth(4.0);

  // Use a line for now, maybe a cylinder later...
  glBegin(GL_LINES);
  cVector3d v1 = m_endpoint_1->getPos();
  cVector3d v2 = m_endpoint_2->getPos();
  glVertex3d(v1.x,v1.y,v1.z);
  glVertex3d(v2.x,v2.y,v2.z);
  glEnd();

}



// A ball (rendered as a sphere) that can be pulled around by 
// springs
CBall::CBall() {

  m_material.m_ambient.set(0.2,0.7,0.2);
  m_material.m_diffuse.set(0.2,0.7,0.2);
  m_material.setShininess(110);
  m_material.m_specular.set(0.8,0.8,0.9);
  
  m_radius = 0.2;
  m_velocity.set(0,0,0);
  m_mass = DEFAULT_OBJECT_MASS;

}


CBall::~CBall() {

}


void CBall::render (const int a_renderMode) {

  static GLUquadricObj *sphereObj = 0;

  if (sphereObj == 0) sphereObj = gluNewQuadric();

  m_material.render();

  // Render a sphere at the location of this ball
  
  #define BALL_SLICES 15
  #define BALL_STACKS 15

  gluSphere(sphereObj, m_radius, BALL_SLICES, BALL_STACKS);

}
