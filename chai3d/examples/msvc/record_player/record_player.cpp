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
    \author:    Federico Barbagli and Christopher Sewell
    \version    1.0
    \date       06/2004
*/
//===========================================================================

#include "stdafx.h"
#include "record_player.h"
#include "record_playerDlg.h"
#include <conio.h>
#include <process.h>

#ifndef M_PI
#define M_PI 3.1415926535898
#endif

// A hard-coded list of sound and image files
const char* g_audio_files[] = {
  "resources\\sounds\\italy.mp3",
  "resources\\sounds\\switzerland.mp3",
  "resources\\sounds\\usa.mp3"
};

const char* g_flag_files[] = {
  "resources\\images\\italy.bmp",
  "resources\\images\\switzerland.bmp",
  "resources\\images\\usa.bmp"
};

const char* g_record_names[] = {
  "Italy", "Switzerland", "USA"  
};

#define NUM_RECORDS (sizeof(g_audio_files)/sizeof(char*))

int getNumRecords() { return NUM_RECORDS; }

const char* getRecordName(int a_index) {
  if (a_index < 0 || a_index >= NUM_RECORDS) return 0;
  return g_record_names[a_index];
}

#define TURNTABLE_MODEL "resources\\models\\turntable.obj"

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

void HapticLoop(void* param);

/***

  CHAI supports two modes of communication with the Phantom... via
  the gstEffect Ghost class, and via Ghost's direct i/o model.  This
  #define allows you to switch between them.  Each may perform better
  in specific situations.

***/

#define USE_PHANTOM_DIRECT_IO 1

#define ALLOCATE_SCOPED_GLOBALS
#include "record_player_globals.h"
#undef ALLOCATE_SCOPED_GLOBALS

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Turn off annoying compiler warnings
#pragma warning(disable: 4305)
#pragma warning(disable: 4800)

BEGIN_MESSAGE_MAP(Crecord_playerApp, CWinApp)
    //{{AFX_MSG_MAP(Crecord_playerApp)
    //}}AFX_MSG
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// No one really knows why GetConsoleWindow() is not
// properly exported by the MS include files.
extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow (); 

#define MESH_SCALE_SIZE 1.0

// Global variables for the audio stream
HSTREAM stream;
BASS_CHANNELINFO info;
QWORD stream_length;
char *data;
int record_direction = 1;
unsigned int pos = 0;


// Load an audio file in .wav format
bool LoadAudioFile(LPCSTR szFileName)
{
  // Load the data from the specified file
  HSTREAM file_stream = BASS_StreamCreateFile(FALSE,szFileName,0,0,BASS_STREAM_DECODE);

  // Get the length and header info from the loaded file
  stream_length=BASS_StreamGetLength(file_stream); 
  BASS_ChannelGetInfo(file_stream, &info);

  // Get the audio samples from the loaded file
  data = new char[(unsigned int)stream_length];
  BASS_ChannelGetData(file_stream, data, (unsigned int)stream_length);
    
  // Set playing to begin at the beginning of the loaded data
  pos = 0;

  return false;
}


// Write the requested data from the loaded buffer to the sound card
DWORD CALLBACK MyStreamWriter(HSTREAM handle, void *buf, DWORD len, DWORD user)
{
  // Cast the buffer to a character array
  char *d=(char*)buf;

  // Loop the file when it reaches the beginning or end
  if ((pos >= stream_length) && (record_direction == 1))
    pos = 0;
  if ((pos <= 0) && (record_direction == -1))
    pos = (unsigned int)stream_length;
        
  // If record is spinning in positive direction, write requested 
  // amount of data from current position forwards
  if (record_direction == 1)
  {
    int up = len + pos;
    if (up > stream_length) 
      up = (unsigned int)stream_length; 

    for (int i=pos; i<up; i+=1) 
      d[(i-pos)] = data[i]; 

    int amt = (up-pos);
    pos += amt;
    return amt;
  }

  // If record is spinning in negative direction, write requested 
  // amount of data from current position backwards
  if (record_direction == -1)
  {
    int up = pos - len;

    if (up < 0)
      up = 0;

    int cnt = 0;
    for (int i=pos; i>up; i-=1) 
      d[cnt++] = data[i]; 

    int amt = cnt;
    pos -= amt;

    return amt;
  }

  return 0;
}


Crecord_playerApp theApp;

Crecord_playerApp::Crecord_playerApp() {

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


void Crecord_playerApp::uninitialize() {

  toggle_haptics(TOGGLE_HAPTICS_DISABLE);
}


int Crecord_playerApp::LoadModel(char* filename) {

  // create a new mesh
  cMesh* new_object = new cMesh(world);
  _cprintf("Loading mesh file %s\n",filename);

  // load 3d object file
  int result = new_object->loadFromFile(filename);

  if (result == 0) {

    _cprintf("Could not load model %s\n",filename);
    delete new_object;
    return -1;
  }

  // I'm going to scale the object so his maximum axis has a
  // size of MESH_SCALE_SIZE. This will make him fit nicely in
  // our viewing area.

  // Tell him to compute a bounding box...
  new_object->computeBoundaryBox(true);

  cVector3d min = new_object->getBoundaryMin();
  cVector3d max = new_object->getBoundaryMax();

  // This is the "size" of the object
  cVector3d span = max;
  span.sub(min);

  // Find his maximum dimension
  float max_size = span.x;
  if (span.y > max_size) max_size = span.y;
  if (span.z > max_size) max_size = span.z;

  // We'll center all vertices, then multiply by this amount,
  // to scale to the desired size.
  float scale_factor = MESH_SCALE_SIZE / max_size;

  // To center vertices, we add this amount (-1 times the
  // center of the object's bounding box)
  cVector3d offset = max;
  offset.add(min);
  offset.div(2.0);
  offset.negate();

  // Now we need to actually scale all the vertices.  However, the
  // vertices might not actually be in this object; they might
  // be in children or grand-children of this mesh (depending on how the 
  // model was defined in the file).
  // 
  // So we find all the sub-meshes we loaded from this file, by descending
  // through all available children.

  // This will hold all the meshes we need to operate on... we'll fill
  // it up as we find more children.
  std::list<cMesh*> meshes_to_scale;

  // This will hold all the parents we're still searching...
  std::list<cMesh*> meshes_to_descend;
  meshes_to_descend.push_front(new_object);

  // Keep track of how many meshes we've found, just to print
  // it out for the user
  int total_meshes = 0;

  // While there are still parent meshes to process
  while(meshes_to_descend.empty() == 0) {

    total_meshes++;

    // Grab the next parent
    cMesh* cur_mesh = meshes_to_descend.front();
    meshes_to_descend.pop_front();
    meshes_to_scale.push_back(cur_mesh);

    // Put all his children on the list of parents to process
    for(unsigned int i=0; i<cur_mesh->getNumChildren(); i++) {

      cGenericObject* cur_object = cur_mesh->getChild(i);

      // Only process cMesh children
      cMesh* cur_mesh = dynamic_cast<cMesh*>(cur_object);
      if (cur_mesh) meshes_to_descend.push_back(cur_mesh);
    }
  }

  _cprintf("Loaded %d vertices in %d meshes from model %s\n",
    new_object->getNumVertices(true),total_meshes,filename);
  
  std::list<cMesh*>::iterator mesh_iter;

  //_cprintf("Offset: %f %f %f\n", offset.x, offset.y, offset.z);

  // Now loop over _all_ the meshes we found...
  for(mesh_iter = meshes_to_scale.begin(); mesh_iter != meshes_to_scale.end(); mesh_iter++) {

    cMesh* cur_mesh = *mesh_iter;
    vector<cVertex>* vertices = cur_mesh->pVertices();
    int num_vertices = cur_mesh->getNumVertices(false);
    cVertex* cur_vertex = (cVertex*)(vertices);

    // Move and scale each vertex in this mesh...
    for(int i=0; i<num_vertices; i++) {
      cur_vertex = cur_mesh->getVertex(i);
      cVector3d pos = cur_vertex->getPos();      
      pos.add(offset);
      pos.mul(scale_factor);
      cur_vertex->setPos(pos);
      cur_vertex++;
    }
    cur_mesh->computeGlobalPositions(false);
  }

  int size = new_object->pTriangles()->size();

  // Re-compute a bounding box
  new_object->computeBoundaryBox(true);
  
  // Build a nice collision-detector for this object, so
  // the proxy will work nicely when haptics are enabled.
  _cprintf("Building collision detector...\n");

  new_object->computeGlobalPositions(false);

  // new_object->createSphereTreeCollisionDetector(true,true);
  new_object->createAABBCollisionDetector(true,true);
  
  _cprintf("Finished building collision detector...\n");

  new_object->computeGlobalPositions();
  
  object = new_object;
  
  world->addChild(object);
  return 0;

}



BOOL Crecord_playerApp::InitInstance() {

    AfxEnableControlContainer();

#ifdef _AFXDLL
    Enable3dControls();         // Call this when using MFC in a shared DLL
#else
    Enable3dControlsStatic();   // Call this when linking to MFC statically
#endif

    g_main_dlg = new Crecord_playerDlg;
  m_pMainWnd = g_main_dlg;

  g_main_dlg->Create(IDD_record_player_DIALOG,NULL);
    
    // create a new world
  world = new cWorld();

  // set background color
  world->setBackgroundColor(0.0f,0.0f,0.0f);

  // Create a camera
  camera = new cCamera(world);
  world->addChild(camera);

  // Create a light source and attach it to camera
  light = new cLight(world);
  light->setEnabled(true);
  light->setPos(cVector3d(2,1,1));

  // define camera position
  flagCameraInMotion = false;

  camera->set(cVector3d(2,0,1), cVector3d(0,0,0), cVector3d(0,0,1));

  // create a display for graphic rendering
  viewport = new cViewport(g_main_dlg->m_gl_area_hwnd, camera, true);
  viewport->setStereoOn(false);

  // init var
  m_rotPos = 0;
  m_rotVel = 0;
  m_inertia = 0.04;
  m_clock.initialize();
  m_lastGoodPosition = 0.0;

  m_reduction = 5.4*10;
  m_cpt = 120*4;          
  m_desiredPos = 0.0;
  m_P = 0.4;
  m_I = 0.0014;
  m_integratorVal = 0.0;
  m_D = 0.02;

  m_lastAngle = 0.0;

  m_velocity = 0;
  m_velocityOld = 0;

  m_RFDInitialAngle = 0.0;
  m_inContact = false;

  LoadModel(TURNTABLE_MODEL);

  object->translate(0,0,-0.3);

  // set stiffness
  double stiffness = (double)35;
  object->setStiffness(stiffness, true);

  // Initialize sound device and create audio stream
  if (!BASS_Init(1,44100,0,0,NULL))
      _cprintf("Init error %d\n", BASS_ErrorGetCode());
    
  // Load a record onto the record player
  load_record(0);
  
  object->computeGlobalPositions(false);
  world->computeGlobalPositions(false);

  return TRUE;
}


int Crecord_playerApp::Run() {

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
int Crecord_playerApp::render_loop() {

  // Just draw the scene...
    if (viewport) viewport->render();

  return 0;

}


// Called when the camera zoom slider changes
void Crecord_playerApp::zoom(int zoom_level) {
  camera->setFieldViewAngle(((float)(zoom_level)));
}


// Called when the user moves the mouse in the main window
// while a button is held down
void Crecord_playerApp::scroll(CPoint p, int left_button) {

  // For now, we're going to disable mouse interaction in
  // this example to simplify the interaction between
  // the haptic device and the record player...
  return;

  // If the user hasn't clicked on any objects, we don't
  // have to move or rotate anyone
  //if (selected_object == 0) return;

  cGenericObject* object_to_move = selected_object;

  // If the left button is being held down, rotate the
  // selected object
  if (left_button) {

        m_recordMesh->translate(cMul(-0.04, m_recordMesh->getRot().getCol2()));
    cVector3d axis1(-1,0,0);
    object->rotate(axis1,-1.0*(float)p.y / 50.0);
        m_recordMesh->rotate(axis1,-1.0*(float)p.y / 50.0);
  
    cVector3d axis2(0,1,0);
    object->rotate(axis2,(float)p.x / 50.0);
        m_recordMesh->rotate(axis2,(float)p.x / 50.0);
        m_recordMesh->translate(cMul(0.04, m_recordMesh->getRot().getCol2()));
  }

  // If the left button is being held down, move the
  // selected object
  else {

    object->translate(0, 0, -1.0*(float)p.y / 100.0);
        m_recordMesh->translate(0, 0, -1.0*(float)p.y / 100.0);
    object->translate(0, 1.0*(float)p.x / 100.0, 0);
        m_recordMesh->translate(0, 1.0*(float)p.x / 100.0, 0);

  }

  // Let the object re-compute his global position data
  object->computeGlobalPositions();
    m_recordMesh->computeGlobalPositions();
  object->computeBoundaryBox(true);  
  
}


// Called when the user clicks his mouse in the main window
//
// Lets CHAI figure out which object was clicked on.
void Crecord_playerApp::select(CPoint p) {

  if (viewport->select(p.x, p.y, true)) {
    selected_object = viewport->getLastSelectedObject();         
  }
  else {    
    selected_object = 0;
  }

}


// Our haptic loop... just computes forces on the 
// phantom every iteration, until haptics are disabled
// in the supplied Crecord_playerApp

void HapticLoop(void* param)
{
  // simulation in now ON
    Crecord_playerApp* app = (Crecord_playerApp*)(param);

  // read position from haptic device
  app->tool->updatePose();

  // compute forces
  app->tool->computeForces();

  // get last interaction force in global coordinate frame
  app->m_interactionForce = cMul(cTrans(app->object->getRot()), cSub(app->tool->m_lastComputedGlobalForce, app->object->getPos()));

  app->tool->applyForces();

  // figure out if we're touching the record
  cProxyPointForceAlgo * algo = app->tool->getProxy();
  if (algo->getContactObject() == app->m_recordMesh)
  {
    if (!app->m_inContact)
    {
      app->m_inContact = true;
      app->m_RFDInitialAngle = app->m_rotPos - app->m_lastGoodPosition*CHAI_PI/180;
    }
    app->animateObject(app->m_interactionForce);
  }
  else
  {
    app->animateObject(cVector3d(0.0, 0.0, 0.0));
    app->m_inContact = false;
  }
}


/***
 
  Enable or disable haptics; called when the user clicks
  the enable/disable haptics button.  The "enable" parameter
  is one of :

  #define TOGGLE_HAPTICS_TOGGLE  -1
  #define TOGGLE_HAPTICS_DISABLE  0
  #define TOGGLE_HAPTICS_ENABLE   1

***/
void Crecord_playerApp::toggle_haptics(int enable) {

  if (enable == TOGGLE_HAPTICS_TOGGLE)
  {
    if (haptics_enabled) toggle_haptics(TOGGLE_HAPTICS_DISABLE);
    else toggle_haptics(TOGGLE_HAPTICS_ENABLE);
  }

  else if (enable == TOGGLE_HAPTICS_ENABLE)
  {
    if (haptics_enabled) return;
    haptics_enabled = 1;
        
    if (tool == NULL)
    {

      tool = new cMeta3dofPointer(world, 0);
      world->addChild(tool);
      tool->setPos(0.0, 0.0, 0.0);

      // turn on dynamic proxy
      cProxyPointForceAlgo* forceAlgo = tool->getProxy();  
      forceAlgo->enableDynamicProxy(true);

      // set up a nice-looking workspace for the phantom so
      // it fits nicely with our models
      tool->setWorkspace(2.0,2.0,2.0);
      tool->setRadius(0.02);
    }

    // set up the device
    tool->initialize();

    // open communication to the device
    tool->start();

    // I need to call this so the tool can update its internal
    // transformations before performing collision detection, etc.
    tool->computeGlobalPositions();
    tool->setForcesON();

    timer.set(0,HapticLoop,this);

    // initialize clock
    m_time = new cPrecisionClock();
    m_time->initialize();

  }
  else if (enable == TOGGLE_HAPTICS_DISABLE) 
  {

    // Don't do anything if haptics are already off
    if (haptics_enabled == 0) return;

    // tell the haptic thread to quit
    haptics_enabled = 0;
    timer.stop();

    if (stream) BASS_ChannelStop(stream);
           
    // Stop the haptic device...
    tool->setForcesOFF();
    tool->stop();
  }

} // toggle_haptics()



// Load a record onto the record player
void Crecord_playerApp::load_record(int a_index)
{

  // Verify valid input
  if (a_index < 0 || a_index >= NUM_RECORDS) return;

  if (stream) { BASS_ChannelStop(stream); Sleep(100); }
  int restart_haptics = 0;
  if (haptics_enabled) {
    restart_haptics = 1;
    toggle_haptics();
  }
  pos = 0;

  LoadAudioFile(g_audio_files[a_index]);
    
  stream=BASS_StreamCreate(info.freq,info.chans,0,&MyStreamWriter,0);
        
  // delete existing record
  if (m_recordMesh) { world->removeChild(m_recordMesh); delete m_recordMesh; }

  // create new record
  m_recordMesh = new cMesh(world);
  createTexCoords(m_recordMesh, 0.33);
  cTexture2D *record = new cTexture2D();

  bool result = record->loadFromFile(g_flag_files[a_index]);
  
  m_recordMesh->setTexture(record);
  m_recordMesh->useTexture(true, true);

  // Compute object size
  m_recordMesh->computeBoundaryBox(true);

  // Build a collision-detector for this object, so
  // the proxy will work nicely when haptics are enabled.
  m_recordMesh->createAABBCollisionDetector(true, true);

  // set size of frame
  m_recordMesh->setFrameSize(0.3, 1.0, true);

  // update global position
  m_recordMesh->computeGlobalPositions();

  // add object to world and translate
  world->addChild(m_recordMesh);
  m_recordMesh->translate(-0.1, 0, -0.21);

  // set stiffness
  double stiffness = (double)35;
  if (m_recordMesh)
    m_recordMesh->setStiffness(stiffness, true);

  // set static and dynamic friction
  double staticFriction = (double)100 / 100.0;
  double dynamicFriction = (double)100 / 100.0;
  if (m_recordMesh)
    m_recordMesh->setFriction(staticFriction, dynamicFriction, true);

  m_rotVel = 0.0;
  if (restart_haptics) toggle_haptics();
}
//---------------------------------------------------------------------------


void Crecord_playerApp::animateObject(cVector3d force)
{
  // do stuff with force: relate force to torque applied about the disc axis

  // integrate torque
  cVector3d planeForce;
  planeForce.set(force.x, force.y, 0);

  // get position of proxy on plane, get vector from center to application point
  cProxyPointForceAlgo* forceAlgo = tool->getProxy();
  m_proxyPos = forceAlgo->getProxyGlobalPosition();
    m_proxyPos.sub(m_recordMesh->getGlobalPos());
  cVector3d radius;
  radius.set(m_proxyPos.x, m_proxyPos.y, 0);

  // do cross product
  cVector3d torqueVec;
  planeForce.crossr(radius, torqueVec);

  // call that the torque applied to the disc and compute new position
  m_torque = torqueVec.z;

  // do the math for how the rotational friction may affect the whole motion
  double time_step = 0.001;
  m_rotVel = m_rotVel + m_torque/m_inertia * time_step;
  m_rotPos = m_rotPos + m_rotVel * time_step;

    // set audio direction and frequency based on rotational velocity
  if (haptics_enabled && (fabs(m_rotVel)) > 0.0)
    {
        if (m_rotVel < 0.0) record_direction = 1;
        else record_direction = -1;
    BASS_ChannelSetAttributes(stream, (int)(info.freq*fabs(m_rotVel)/6.5), -1, -1); 
        if (!(BASS_ChannelPlay(stream,FALSE)))
          _cprintf("Play error %d\n", BASS_ErrorGetCode()); 
    }
    else
    {
        BASS_ChannelStop(stream);
    }

  // rotate object
  m_recordMesh->rotate(m_recordMesh->getRot().getCol2(), m_rotVel * time_step);
  m_recordMesh->computeGlobalPositions();
}
//---------------------------------------------------------------------------


// Define the texture coordinates for the record
void createTexCoords(cMesh *a_mesh, double radius) {

  double vectorY1, originY, vectorX1, originX;
  vectorY1 = 0; originY = 0;
  vectorX1 = 0; originX = 0;
  cVector3d vector1, vector2, origin;
  origin.set(originX, originY, 0);
  vector2.set(originX, originY, 0);

  int num = 28;
  double divisore = num / (2*CHAI_PI);

  for(int i=0; i<=num; i++)
  {
    double angle=(float)(((double)i)/divisore);
    vector1.set(originX+(radius*(float)sin((double)angle)), originY+(radius*(float)cos((double)angle)), 0);
    a_mesh->newTriangle(origin, vector1, vector2);
    vector2.set(vector1.x, vector1.y, vector1.z);
  }

  for(unsigned int n=0; n<a_mesh->getNumVertices(); n++)
  {
    cVertex* curVertex = a_mesh->getVertex(n);
    curVertex->setTexCoord(
        (curVertex->getPos().x + radius) / (2.0 * radius),
        (curVertex->getPos().y + radius) / (2.0 * radius)
    );
    curVertex->setNormal(0,0,1);
  }

  // compute normals
  a_mesh->computeAllNormals(true);

  // compute boundary box
  a_mesh->computeBoundaryBox(true);
}
