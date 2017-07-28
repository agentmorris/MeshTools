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
    \author:    Christopher Sewell
    \version    1.1
    \date       12/2006
*/
//===========================================================================


/*************************************************************************
*                                                                       *
* This example uses the Open Dynamics Engine and is based in part on    *
* the test_moving_trimesh example in ODE.                               *
*                                                                       *
* Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
* All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
*                                                                       *
*************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CCamera.h"
#include "CLight.h"
#include "CWorld.h"
#include "CMesh.h"
#include "CTriangle.h"
#include "CVertex.h"
#include "CMaterial.h"
#include "CTexture2D.h"
#include "CMatrix3d.h"
#include "CVector3d.h"
#include "CPrecisionClock.h"
#include "CPrecisionTimer.h"
#include "CMeta3dofPointer.h"
#include "CShapeSphere.h"
#include "CBitmap.h"
#include "CODEProxy.h"

#include <ode/ode.h>

#ifdef _MSC_VER
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#endif


// Different compilers like slightly different GLUT's 
#ifdef _MSVC
#include "../../../external/OpenGL/msvc6/glut.h"
#else
#ifdef _POSIX
#include <GL/glut.h>
#else
#include "../../../external/OpenGL/bbcp6/glut.h"
#endif
#endif

// some constants

#define NUM 200         // max number of objects
#define DENSITY (5.0)       // density of all objects
#define GPB 3           // maximum number of geometries per body
#define MAX_CONTACTS 64     // maximum number of contact points per body

//---------------------------------------------------------------------------

// the world in which we will create our environment
cWorld* world;

// the camera which is used view the environment in a window
cCamera* camera;

// a light source
cLight *light;

// a list of the meshes in the world
std::vector<cMesh*> objects;

// a 3D cursor which represents the haptic device
cMeta3dofPointer* cursor = 0;

// haptic timer callback
cPrecisionTimer timer;

// width and height of the current viewport display
int width   = 0;
int height  = 0;

// menu options
const int OPTION_FULLSCREEN     = 1;
const int OPTION_WINDOWDISPLAY  = 2;

int create_mesh_flag = 0;

//---------------------------------------------------------------------------
// dynamics and collision objects

struct MyObject {
    dBodyID body;           // the body
    dGeomID geom[GPB];      // geometries representing this body

    // Trimesh only - double buffered matrices for 'last transform' setup
    dReal matrix_dblbuff[ 16 * 2 ];
    int last_matrix_index;
};

// number of ODE objects in simulation
static int num=0;       
static dWorldID ode_world;
static dSpaceID space;
static MyObject obj[NUM];
static dJointGroupID contactgroup;
static int loading = 0;

static int inContact = -1;
cVector3d lastForce;
cVector3d lastContactPoint;

cProxyPointForceAlgo* default_proxy;

//! Pointer to callback function
dNearCallback  *ode_collision_callback;
//! Pointer to any extra data
void *ode_collision_callback_data;

int VertexCount;
int IndexCount;
float* Vertices;
int* Indices;

// amount by which to scale haptic force when applying to simulation
float force_scale = 0.1;  

typedef dReal dVector3R[3];


//---------------------------------------------------------------------------
// this is called by dSpaceCollide when two objects in space are
// potentially colliding.

static void nearCallback (void *data, dGeomID o1, dGeomID o2)
{
    int i;
    // if (o1->body && o2->body) return;

    // exit without doing anything if the two bodies are connected by a joint
    dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);
    if (b1 && b2 && dAreConnectedExcluding (b1,b2,dJointTypeContact)) return;

    dContact contact[MAX_CONTACTS];   // up to MAX_CONTACTS contacts per box-box
    for (i=0; i<MAX_CONTACTS; i++) {
        contact[i].surface.mode = dContactBounce | dContactSoftCFM;
        contact[i].surface.mu = dInfinity;
        contact[i].surface.mu2 = 0;
        contact[i].surface.bounce = 0.1;
        contact[i].surface.bounce_vel = 0.1;
        contact[i].surface.soft_cfm = 0.01;
    }
    if (int numc = dCollide (o1,o2,MAX_CONTACTS,&contact[0].geom,
        sizeof(dContact))) {
            dMatrix3 RI;
            dRSetIdentity (RI);
            const dReal ss[3] = {0.02,0.02,0.02};
            for (i=0; i<numc; i++) {
                dJointID c = dJointCreateContact (ode_world,contactgroup,contact+i);
                dJointAttach (c,b1,b2);
            }
        }
}

//---------------------------------------------------------------------------
// Set the position and rotation of the CHAI meshes to their corresponding
// ODE objects

static void syncPoses()
{
    for (int i=0; i<num; i++)
    {
        const dReal* odePosition = dGeomGetPosition(obj[i].geom[0]);
        const dReal* odeRotation = dGeomGetRotation(obj[i].geom[0]);

        cMatrix3d   chaiRotation;
        chaiRotation.set(odeRotation[0],odeRotation[1],odeRotation[2],
            odeRotation[4],odeRotation[5],odeRotation[6],
            odeRotation[8],odeRotation[9],odeRotation[10]);

        objects[i]->setRot(chaiRotation);
        objects[i]->setPos(odePosition[0],odePosition[1],odePosition[2]);

        objects[i]->computeGlobalPositions(1);
    }
}

//---------------------------------------------------------------------------

void simStep()
{
    if (loading == 0)
    { 
        dSpaceCollide (space,0,&nearCallback);
        dWorldStepFast1 (ode_world,0.05, 5);
        for (int j = 0; j < dSpaceGetNumGeoms(space); j++){
            dSpaceGetGeom(space, j);
        }
        dJointGroupEmpty (contactgroup);
    }
}

//---------------------------------------------------------------------------

void draw(void)
{
    // set the background color of the world
    cColorf color = camera->getParentWorld()->getBackgroundColor();
    glClearColor(color.getR(), color.getG(), color.getB(), color.getA());

    // clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render world in the window display
    camera->renderView(width, height);

    // check for any OpenGL errors
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR) printf("Error:  %s\n", gluErrorString(err));

    // Swap buffers
    glutSwapBuffers();

    if (inContact >= 0) 
    {
        float x =  lastContactPoint.x;
        float y =  lastContactPoint.y;
        float z =  lastContactPoint.z;

        float fx = -force_scale*lastForce.x ;
        float fy = -force_scale*lastForce.y ;
        float fz = -force_scale*lastForce.z ;

        if (loading == 0)
            dBodyAddForceAtPos(obj[inContact].body,fx,fy,fz,x,y,z);
    }

    simStep();
}

//---------------------------------------------------------------------------

static void createMesh()
{
    int i,k;
    dReal sides[3];
    dMass m;

    loading = 1;
    Sleep(100);
    i = num;
    num++;

    obj[i].body = dBodyCreate (ode_world);
    for (k=0; k<3; k++) sides[k] = dRandReal()*0.5+0.1;

    dMatrix3 R;
    
    dBodySetPosition (obj[i].body,
        dRandReal()*2-1,dRandReal()*2-1,dRandReal()+3);
    dRFromAxisAndAngle (R,dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
        dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
    
    dBodySetRotation (obj[i].body,R);
    dBodySetData (obj[i].body,(void*)(size_t)i);

    cMesh* new_object = new cMesh(world); 
    new_object->loadFromFile("resources\\models\\bunny.obj");

    new_object->computeGlobalPositions();
    new_object->createAABBCollisionDetector(true, true);
    new_object->computeAllNormals();

    // set material properties of object
    cMaterial new_material;
    new_material.setStiffness(20.0);
    new_material.setDynamicFriction(0.2);
    new_material.setStaticFriction(0.4);

    new_material.m_ambient.set(0.3, 0.3, 0.8);
    new_material.m_diffuse.set(0.8, 0.8, 0.8);
    new_material.m_specular.set(1.0, 1.0, 1.0);
    new_material.setShininess(100);
    new_object->useMaterial(true, true);
    new_object->setMaterial(new_material, true);
    new_object->useColors(false, true);

    cODEProxy* new_proxy = new cODEProxy(default_proxy);
    new_proxy->enableDynamicProxy(true);
    cursor->setRenderingMode(RENDER_DEVICE);
    new_proxy->m_defaultObject = new_object;
    new_proxy->initialize(world, cursor->m_deviceGlobalPos);
    cursor->m_pointForceAlgos.push_back(new_proxy);

    VertexCount = new_object->getNumVertices(true);
    Vertices = new float[VertexCount*3];

    IndexCount = 3*new_object->getNumTriangles(true);
    Indices = new int[IndexCount];

    // This will hold all the parents we're still searching...
    std::list<cMesh*> meshes_to_descend;
    meshes_to_descend.push_front(new_object);

    // While there are still parent meshes to process
    int cnt = 0;
    int cnt2 = 0;
    while(meshes_to_descend.empty() == 0) {

        // Grab the next parent
        cMesh* cur_mesh = meshes_to_descend.front();
        meshes_to_descend.pop_front();

        // Put all his children on the list of parents to process
        for(unsigned int i=0; i<cur_mesh->getNumChildren(); i++) {

            cGenericObject* cur_object = cur_mesh->getChild(i);

            // Only process cMesh children
            cMesh* cur_mesh = dynamic_cast<cMesh*>(cur_object);
            if (cur_mesh) 
            {
				unsigned int i;
                for (i=0; i<cur_mesh->getNumVertices(false); i++)
                {
                    Vertices[cnt] = cur_mesh->getVertex(i)->getPos().x;
                    Vertices[cnt+1] = cur_mesh->getVertex(i)->getPos().y;
                    Vertices[cnt+2] = cur_mesh->getVertex(i)->getPos().z;
                    cnt+=3;
                }
                for (i=0; i<cur_mesh->getNumTriangles(false); i++)
                {
                    Indices[cnt2] = cur_mesh->getTriangle(i)->getIndexVertex0();
                    Indices[cnt2+1] = cur_mesh->getTriangle(i)->getIndexVertex1();
                    Indices[cnt2+2] = cur_mesh->getTriangle(i)->getIndexVertex2();
                    cnt2+=3;
                }
                meshes_to_descend.push_back(cur_mesh);
            }
        }
    }

    dTriMeshDataID new_tmdata = dGeomTriMeshDataCreate();
    dGeomTriMeshDataBuildSingle(new_tmdata, &Vertices[0], 3 * sizeof(float), VertexCount, Indices, IndexCount, 3 * sizeof(int));

    obj[i].geom[0] = dCreateTriMesh(space, new_tmdata, 0, 0, 0);

    // remember the mesh's dTriMeshDataID on its userdata for convenience.
    dGeomSetData(obj[i].geom[0], new_tmdata);      

    dMassSetBox (&m,DENSITY,sides[0],sides[1],sides[2]);
    
    for (k=0; k < GPB; k++) 
        if (obj[i].geom[k]) dGeomSetBody (obj[i].geom[k],obj[i].body);

    dBodySetMass (obj[i].body,&m);
    
    new_object->setPos(0,0,100);
    new_object->computeGlobalPositions(1);
    objects.push_back(new_object);

    syncPoses();
    world->addChild(new_object);
    loading = 0;
    Sleep(100);
}

//---------------------------------------------------------------------------

void setCurrentTransform(dGeomID geom)
{
    const dTriMeshDataID TriMeshData = static_cast<dTriMeshDataID>(dGeomGetData(geom));
    const dReal* Pos = dGeomGetPosition(geom);
    const dReal* Rot = dGeomGetRotation(geom);

    const double Transform[16] = 
    {
        Rot[0], Rot[4], Rot[8],  0,
            Rot[1], Rot[5], Rot[9],  0,
            Rot[2], Rot[6], Rot[10], 0,
            Pos[0], Pos[1], Pos[2],  1
    };
    dGeomTriMeshDataSet(TriMeshData, TRIMESH_LAST_TRANSFORMATION, (void *)Transform);
}

//---------------------------------------------------------------------------

void key(unsigned char key, int x, int y)
{
    // "ESC" key is pressed
    if (key == 27)
    {
        // stop the simulation timer
        timer.stop();

        // stop the tool
        cursor->stop();

#ifndef _POSIX 
        // wait for the simulation timer to close
        Sleep(100);
#endif

        // exit application
        exit(0);
    }
    if (key == ' ')
        create_mesh_flag = 1;
}

//---------------------------------------------------------------------------

void resizeWindow(int w, int h)
{
    // update the size of the viewport
    width = w;
    height = h;

    // update viewport
    glViewport(0, 0, width, height);

    // update the size of the "chai3d" logo
    float scale = (float) w / 1500.0;
}

//---------------------------------------------------------------------------

void updateDisplay(int val)
{
    // draw scene
    draw();

    // update the GLUT timer for the next rendering call
    glutTimerFunc(30, updateDisplay, 0);
}

//---------------------------------------------------------------------------

void setOther(int value)
{
    switch (value)
    {
        case OPTION_FULLSCREEN:
            glutFullScreen();
            break;

        case OPTION_WINDOWDISPLAY:
            glutReshapeWindow(512, 512);
            glutInitWindowPosition(0, 0);
            break;
    }
    
    glutPostRedisplay();
}

//---------------------------------------------------------------------------

void hapticsLoop(void* a_pUserData)
{
    if (create_mesh_flag == 1)
    {
        createMesh();
        create_mesh_flag = 0;
    }

    syncPoses();
    cursor->computeGlobalPositions(1);

    // read the position of the haptic device
    cursor->updatePose();

    // compute forces between the cursor and the environment
    cursor->computeForces();

    // send forces to haptic device
    cursor->applyForces();

    inContact = -1;
    for (unsigned int i=0; i<cursor->m_pointForceAlgos.size(); i++)
    {
        cProxyPointForceAlgo* cur_proxy = dynamic_cast<cProxyPointForceAlgo*>(cursor->m_pointForceAlgos[i]);
        if ((cur_proxy != NULL) && (cur_proxy->getContactObject() != NULL)) 
        {      
            lastContactPoint = cur_proxy->getContactPoint();
            lastForce = cursor->m_lastComputedGlobalForce;
            inContact = i;
            break;
        }
    }
    
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    printf ("\n");
    printf ("  ===================================\n");
    printf ("  CHAI 3D\n");
    printf ("  Dynamic Meshes Demo\n");
    printf ("  Copyright 2006\n");
    printf ("  ===================================\n");
    printf ("\n");
    printf ("Press the spacebar to drop additional bunnies from the sky");
    printf ("\n");

    // create a new world
    world = new cWorld();

    // set background color
    world->setBackgroundColor(0.2f,0.2f,0.2f);

    // create a camera
    camera = new cCamera(world);
    world->addChild(camera);

    ode_world = dWorldCreate();

    space = dSimpleSpaceCreate(0);
    contactgroup = dJointGroupCreate (0);
    dWorldSetGravity (ode_world,0,0,-0.5);
    dWorldSetCFM (ode_world,1e-5);
    dCreatePlane (space,0,0,1,0);
    memset (obj,0,sizeof(obj));

    ode_collision_callback    = nearCallback;
    ode_collision_callback_data  = 0;

    // position a camera
    camera->set( cVector3d(6.0,0,3.0), 
                 cVector3d (0.0, 0.0, 0.0),
                 cVector3d (0.0, 0.0, 1.0));

    // Create a light source and attach it to the camera
    light = new cLight(world);
    light->setEnabled(true);
    light->setPos(cVector3d(2,0.5,1));
    light->setDir(cVector3d(-2,0.5,1));
    camera->addChild(light);

    // create a cursor and add it to the world.
    cursor = new cMeta3dofPointer(world, 0);
    world->addChild(cursor);
    cursor->setPos(0.0, 0.0, 0.0);

    // set up a nice-looking workspace for the cursor so it fits nicely with our
    // cube models we will be building
    cursor->setWorkspace(4.0,4.0,4.0);

    // set the diameter of the ball representing the cursor
    cursor->setRadius(0.05);

    // Replace the proxy with our custom ODE proxy
    default_proxy = (cProxyPointForceAlgo*)(cursor->m_pointForceAlgos[0]);
    cursor->m_pointForceAlgos.clear();

    // set up the device
    cursor->initialize();

    // open communication to the device
    cursor->start();

    // start haptic timer callback
    timer.set(0, hapticsLoop, NULL);

	// drop a mesh from the sky
	Sleep(1000);
	create_mesh_flag = 1;

    // initialize the GLUT windows
    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(0, 0);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow(argv[0]);
    glutDisplayFunc(draw);
    glutKeyboardFunc(key);
    glutReshapeFunc(resizeWindow);
    glutSetWindowTitle("CHAI 3D");

    // create a mouse menu
    glutCreateMenu(setOther);
    glutAddMenuEntry("Full Screen", OPTION_FULLSCREEN);
    glutAddMenuEntry("Window Display", OPTION_WINDOWDISPLAY);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    // update display
    glutTimerFunc(30, updateDisplay, 0);

    // start main graphic rendering loop
    glutMainLoop();
    return 0;
}



