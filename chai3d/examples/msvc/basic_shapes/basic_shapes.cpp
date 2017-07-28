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
#include "basic_shapes.h"
#include "basic_shapesDlg.h"
#include <conio.h>
#include <process.h>

#ifndef M_PI
#define M_PI 3.1415926535898
#endif

/***

  This example demonstrates two ways to drive a haptic loop
  in CHAI.  One can use the cPrecisionTimer class, which uses
  the Windows Multimedia Timer system to request a 1kHz callback,
  which calls the program's haptic loop.  One can also spawn
  a high-priority thread that runs the haptic loop in a while(1)
  loop.

  The former approach uses less CPU but may not really deliver
  1kHz in all cases.  The latter allows complete control of the CPU
  but may slow down other threads.

  This #define controls which approach is used by this application.
***/

#define USE_MM_TIMER_FOR_HAPTICS


/***

  CHAI supports two modes of communication with the Phantom... via
  the gstEffect Ghost class, and via Ghost's direct i/o model.  This
  #define allows you to switch between them.  Each may perform better
  in specific situations.

***/

#define USE_PHANTOM_DIRECT_IO 1


#define ALLOCATE_SCOPED_GLOBALS
#include "basic_shapes_globals.h"
#undef ALLOCATE_SCOPED_GLOBALS

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Turn off annoying compiler warnings
#pragma warning(disable: 4305)
#pragma warning(disable: 4800)

BEGIN_MESSAGE_MAP(Cbasic_shapesApp, CWinApp)
	//{{AFX_MSG_MAP(Cbasic_shapesApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// No one really knows why GetConsoleWindow() is not
// properly exported by the MS include files.
extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow (); 

Cbasic_shapesApp theApp;

Cbasic_shapesApp::Cbasic_shapesApp() {

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


void Cbasic_shapesApp::uninitialize() {

  toggle_haptics(TOGGLE_HAPTICS_DISABLE);
  delete world;
  delete viewport;
  delete camera;
  
}


BOOL Cbasic_shapesApp::InitInstance() {

	AfxEnableControlContainer();

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	g_main_dlg = new Cbasic_shapesDlg;
  m_pMainWnd = g_main_dlg;

  g_main_dlg->Create(IDD_basic_shapes_DIALOG,NULL);
    
  // Now we should have a display context to work with...

  world = new cWorld();

  // set background color
  world->setBackgroundColor(0.0f,0.0f,0.0f);


  // Create a camera
  camera = new cCamera(world);

  // set camera position and orientation
  // 
  // We choose to put it out on the positive z axis, so things appear
  // the way OpenGL users expect them to appear, with z going in and
  // out of the plane of the screen.
  int result = camera->set(cVector3d(0,0,4),   // position of camera
              cVector3d(0.0, 0.0, 0.0),   // camera looking at origin
              cVector3d(0.0, 1.0, 0.0));  // orientation of camera (standing up)

  if (result == 0) {
    _cprintf("Could not create camera...\n");
  }

  // Turn on one light...
  light = new cLight(world);

  light->setEnabled(true);

  // Use a purely directional light, pointing from
  // somewhere off to the left...
  light->setDirectionalLight(true);
  light->setPos(cVector3d(-1, 1, 1));
  
  // Create a display for graphic rendering
  viewport = new cViewport(g_main_dlg->m_gl_area_hwnd, camera, false);

  // Create a mesh - we will build a pyramid manually, and later let the
  // user load 3d models
  object = new cMesh(world);
  world->addChild(object);

  double size = 1.0;

  // Create a nice little cube
  createCube(object,(float)size);

  // Move the object over some so the Phantom will not initially be
  // inside the object.
  object->translate(1.5 ,0.0 ,-1.2);

  // Rotate the cube to make it look nice
  object->rotate(cVector3d(0,1,0),30.0 * 3.14159 / 180.0);
  object->rotate(cVector3d(1,0,0),30.0 * 3.14159 / 180.0);

  // Initialize the object's collision-detection state
  object->computeGlobalPositions();
  object->computeBoundaryBox(true);

  // Note that I don't bother to build a fancy collision detector
  // for this cube, since the brute force collision detector -
  // which is the default - works fine for using the haptic proxy
  // on a simple shape with only a few triangles.
  
  // Give a color to each vertex
  for (unsigned int i=0; i<object->getNumVertices(); i++) {
    
    cVertex* nextVertex = object->getVertex(i);

    cColorb color;
    color.set(
      GLuint(0xff*(size + nextVertex->getPos().x ) / (2.0 * size)),
      GLuint(0xff*(size + nextVertex->getPos().y ) / (2.0 * size)),
      GLuint(0xff* nextVertex->getPos().z / 2*size)
      );

    nextVertex->setColor(color);
  }

  // Set object settings.  The parameters tell the object
  // to apply this alpha level to his textures and to his
  // children (of course he has neither right now).
  // object->setTransparencyLevel(0.5, true, true);

  // Give him some material properties...
  cMaterial material;

  // This sets the _haptic_ stiffness of the material
  material.setStiffness(50.0);

  material.m_ambient.set( 0.4, 0.2, 0.2, 1.0 );
  material.m_diffuse.set( 0.8, 0.6, 0.6, 1.0 );
  material.m_specular.set( 0.9, 0.9, 0.9, 1.0 );
  material.setShininess(100);
  object->m_material = material;

  world->computeGlobalPositions(false);

  return TRUE;
}


int Cbasic_shapesApp::Run() {

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
int Cbasic_shapesApp::render_loop() {

  // Just draw the scene...
  viewport->render();

  return 0;

}


// Called when the camera zoom slider changes
void Cbasic_shapesApp::zoom(int zoom_level) {

  //camera->setPos(p.x,((float)(zoom_level))/100.0*10.0,p.z);
  camera->setFieldViewAngle(((float)(zoom_level)));
  
}


// Called when the user moves the mouse in the main window
// while a button is held down
void Cbasic_shapesApp::scroll(CPoint p, int left_button) {

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
  object->computeGlobalPositions();
  object->computeBoundaryBox(true);  
  
}


// Called when the user clicks his mouse in the main window
//
// Lets CHAI figure out which object was clicked on.
void Cbasic_shapesApp::select(CPoint p) {

  if (viewport->select(p.x, p.y, true)) {
    selected_object = viewport->getLastSelectedObject();         
  }
  else {    
    selected_object = 0;
  }

}


// Our haptic loop... just computes forces on the 
// phantom every iteration, until haptics are disabled
// in the supplied Cbasic_shapesApp

// A single iteration through the loop...
void basic_shapes_haptic_iteration(void* param) {

  Cbasic_shapesApp* app = (Cbasic_shapesApp*)(param);

  app->tool->updatePose();
  app->tool->computeForces();
  app->tool->applyForces();

}


// This loop is used only in the threaded version of this
// application... all it does is call the main haptic
// iteration loop, which is called directly from a timer
// callback if USE_MM_TIMER_FOR_HAPTICS is defined
DWORD basic_shapes_haptic_loop(void* param) {

  Cbasic_shapesApp* app = (Cbasic_shapesApp*)(param);

  while(app->haptics_enabled) {

    basic_shapes_haptic_iteration(param);

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
void Cbasic_shapesApp::toggle_haptics(int enable) {

  
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
    timer.set(0,basic_shapes_haptic_iteration,this);

#else

    // start haptic thread
    haptics_thread_running = 1;

    DWORD thread_id;
    ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)(basic_shapes_haptic_loop), this, 0, &thread_id);

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


// A global function for sticking a cube in the specified mesh
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
  mesh->newVertex( radius,  radius, -radius);
  mesh->newVertex( radius,  radius,  radius);
  mesh->newVertex( radius, -radius, -radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  mesh->newVertex( radius, -radius, -radius);
  mesh->newVertex( radius,  radius,  radius);
  mesh->newVertex( radius, -radius,  radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = mesh->getVertex(n);
    curVertex->setNormal(1,0,0);
  }
  
  start_index += 6;

  // -x face
  mesh->newVertex(-radius,  radius,  radius);
  mesh->newVertex(-radius,  radius, -radius);
  mesh->newVertex(-radius, -radius, -radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  mesh->newVertex(-radius,  radius,  radius);
  mesh->newVertex(-radius, -radius, -radius);
  mesh->newVertex(-radius, -radius,  radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = mesh->getVertex(n);
    curVertex->setNormal(-1,0,0);
  }
  
  start_index += 6;

  // +y face
  mesh->newVertex(radius,  radius,  radius);
  mesh->newVertex(radius,  radius, -radius);
  mesh->newVertex(-radius, radius, -radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  mesh->newVertex(radius,  radius,  radius);
  mesh->newVertex(-radius, radius, -radius);
  mesh->newVertex(-radius, radius,  radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = mesh->getVertex(n);
    curVertex->setNormal(0,1,0);
  }
  
  start_index += 6;

  // -y face
  mesh->newVertex(radius,  -radius,  radius);
  mesh->newVertex(-radius, -radius, -radius);
  mesh->newVertex(radius,  -radius, -radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  mesh->newVertex(-radius, -radius, -radius);
  mesh->newVertex(radius,  -radius,  radius);
  mesh->newVertex(-radius, -radius,  radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = mesh->getVertex(n);
    curVertex->setNormal(0,-1,0);
  }
  
  start_index += 6;
  
  // +z face
  mesh->newVertex(-radius, -radius, radius);
  mesh->newVertex(radius,  -radius, radius);
  mesh->newVertex(radius,  radius,  radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  mesh->newVertex(-radius, -radius, radius);
  mesh->newVertex( radius,  radius, radius);
  mesh->newVertex(-radius,  radius, radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = mesh->getVertex(n);
    curVertex->setNormal(0,0,1);
  }
  
  start_index += 6;
  
  // -z face
  mesh->newVertex(-radius, -radius, -radius);
  mesh->newVertex(radius,   radius, -radius);
  mesh->newVertex(radius,  -radius, -radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  mesh->newVertex( radius,  radius, -radius);
  mesh->newVertex(-radius, -radius, -radius);
  mesh->newVertex(-radius,  radius, -radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = mesh->getVertex(n);
    curVertex->setNormal(0,0,-1);
  }
  
  start_index += 6;

}
