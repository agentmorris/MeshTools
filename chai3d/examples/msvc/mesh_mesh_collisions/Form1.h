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
    \author:    Chris Sewell
    \version    1.0
    \date       05/2006
*/
//===========================================================================

#pragma once

#include "CWorld.h"
#include "CViewport.h"
#include "CGenericObject.h"
#include "CMeshTool.h"
#include "CPrecisionTimer.h"
#include "CLight.h"

// Prototype for haptic iteration callback function
void haptic_iteration(void* param);

DWORD haptic_loop(void* param);

int haptics_enabled;

//#define USE_MM_TIMER_FOR_HAPTICS


namespace tri_tri_collisions
{
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary> 
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the 
	///          'Resource File Name' property for the managed resource compiler tool 
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{	
	public:
		Form1(void)
		{
			InitializeComponent();
		}
  
	protected:
		!Form1()
		{
			if (components)
			{
				// components->Dispose();
			}
			// __super::Dispose(true);
		}
	private: System::Windows::Forms::Panel ^  panel1;
	private: System::Windows::Forms::Button ^  button1;

	public:
		cWorld* world;
		cCamera* camera;
		cViewport* viewport;
		cLight* light;
		cMesh* object;
		cPrecisionTimer* mm_timer;
		cMeshTool* tool;
		cMesh* tool_object;
	private: System::Windows::Forms::Timer ^  timer1;
	private: System::Windows::Forms::CheckBox ^  findAll;
	private: System::ComponentModel::IContainer ^  components;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = gcnew System::ComponentModel::Container();
			this->panel1 = gcnew System::Windows::Forms::Panel();
			this->button1 = gcnew System::Windows::Forms::Button();
			this->timer1 = gcnew System::Windows::Forms::Timer(this->components);
			this->findAll = gcnew System::Windows::Forms::CheckBox();
			this->SuspendLayout();
			// 
			// panel1
			// 
			this->panel1->Location = System::Drawing::Point(16, 16);
			this->panel1->Name = "panel1";
			this->panel1->Size = System::Drawing::Size(656, 464);
			this->panel1->TabIndex = 0;
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(136, 496);
			this->button1->Name = "button1";
			this->button1->Size = System::Drawing::Size(176, 24);
			this->button1->TabIndex = 1;
			this->button1->Text = "Toggle Haptics";
			this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
			// 
			// timer1
			// 
			this->timer1->Enabled = true;
			this->timer1->Interval = 30;
			this->timer1->Tick += gcnew System::EventHandler(this, &Form1::timer1_Tick);
			// 
			// findAll
			// 
			this->findAll->Checked = true;
			this->findAll->CheckState = System::Windows::Forms::CheckState::Checked;
			this->findAll->Location = System::Drawing::Point(392, 497);
			this->findAll->Name = "findAll";
			this->findAll->Size = System::Drawing::Size(144, 24);
			this->findAll->TabIndex = 2;
			this->findAll->Text = "Find All Collisions";
			// 
			// Form1
			// 
			this->AutoScaleBaseSize = System::Drawing::Size(5, 13);
			this->ClientSize = System::Drawing::Size(688, 534);
			this->Controls->Add(this->findAll);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->panel1);
			this->Name = "Form1";
			this->Text = "Mesh Mesh Collisions";
			this->Closing += gcnew System::ComponentModel::CancelEventHandler(this, &Form1::Form1_Closing);
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			this->ResumeLayout(false);

		}	
	private: System::Void Form1_Load(System::Object ^  sender, System::EventArgs ^  e)
			 {
				 // Allocate a console for outputting _cprintf statements for debugging
				 AllocConsole();

				 // Create a world and set a white background color
				 world = new cWorld();
				 world->setBackgroundColor(1.0,1.0,1.0);

				 // Create a camera and set its position, look-at point, and orientation (up-direction)
				 camera = new cCamera(world);
				 int result = camera->set(cVector3d(0,0,4), cVector3d(0,0,0), cVector3d(0,1,0));

				 // Create, enable, and position a light source
				 light = new cLight(world);
				 light->setEnabled(true);
				 light->setPos(cVector3d(0,1,4));

				 // Get a pointer to the panel object's handle and create a viewport, telling it to render to that panel
				 IntPtr ptr = panel1->Handle; // get_Handle();
				 HWND hwnd = (HWND)ptr.ToPointer();
				 viewport = new cViewport(hwnd, camera, false);

				 // Load a gear mesh from a .3DS file
				 object = new cMesh(world);
				 object->loadFromFile("resources\\models\\small_gear.3ds");

				 // Scale the object to fit nicely in our viewport
				 // compute size of object
                 object->computeBoundaryBox(true);

                 cVector3d min = object->getBoundaryMin();
                 cVector3d max = object->getBoundaryMax();

                 // This is the "size" of the object
                 cVector3d span = cSub(max, min);
                 double size = cMax(span.x, cMax(span.y, span.z));

                 // We'll center all vertices, then multiply by this amount,
                 // to scale to the desired size.
                 double scaleFactor = 2.0 / size;
                 object->scale(scaleFactor);

				 // Tell him to compute a bounding box...
				 object->computeBoundaryBox(true);
					
				 // Build a nice collision-detector for this object
				 object->createSphereTreeCollisionDetector(true,true);
	
				 // Automatically compute normals for all triangles
				 object->computeAllNormals();

				 // Translate and rotate so that the airplane is flying towards the right of the screen
				 object->translate(0.7, 0.0, 0.0);
                 object->rotate(cVector3d(0,1,0),-90.0 * 3.14159 / 180.0);
				 object->rotate(cVector3d(1,0,0),-30.0 * 3.14159 / 180.0);
				 object->computeGlobalPositions(false);

				 // Use vertex colors so that we can see which triangles collide
			     object->useColors(true, true);
				 
				 // Add the mesh object to the world
				 world->addChild(object);

				 // Create a callback timer (to be used for haptics loop)
				 mm_timer = new cPrecisionTimer();

				 // Initilize some variables
				 world->computeGlobalPositions();
				 tool = 0;
				 haptics_enabled = 0;

			 }

	private: System::Void timer1_Tick(System::Object ^  sender, System::EventArgs ^  e)
			 {
				 if (tool)
		             if (tool->m_collisionDetector)
			             tool->m_collisionDetector->setFindAll(findAll->Checked);
				 // Render the scene
				 if (viewport)
					 viewport->render();
			 }


private: System::Void button1_Click(System::Object ^  sender, System::EventArgs ^  e)
		 {
			 // If haptics isn't on, turn it on...
			 if (haptics_enabled == 0) {
				 haptics_enabled = 1;

				 // Create a tool if we haven't yet
				 if (tool == 0) {
	                 // Create a new tool with this mesh
					 tool = new cMeshTool(world, 0, true);
					 world->addChild(tool);

					 // Load a gear mesh from a .3DS file
					 tool_object = new cMesh(world);
					 tool_object->loadFromFile("resources\\models\\small_gear.3ds");
				     tool_object->computeGlobalPositions(false);

					 // Scale the object to fit nicely in our viewport
					 // compute size of object
					 tool_object->computeBoundaryBox(true);

					 cVector3d min = tool_object->getBoundaryMin();
					 cVector3d max = tool_object->getBoundaryMax();

					 // This is the "size" of the object
					 cVector3d span = cSub(max, min);
					 double size = cMax(span.x, cMax(span.y, span.z));

					 // We'll center all vertices, then multiply by this amount,
					 // to scale to the desired size.
					 double scaleFactor = 2.0 / size;
					 tool_object->scale(scaleFactor);

					 // Create a sphere tree bounding volume hierarchy for collision detection on this mesh
					 tool_object->createSphereTreeCollisionDetector(true, true);

					 // Use vertex colors so that we can see which triangles collide
					 tool_object->useColors(true, true);

					 // Add the mesh object to the world
					 world->addChild(tool_object);

					 // Set the mesh for this tool
					 tool->setMesh(tool_object);

					 // Tell the tool to search for collisions with this mesh
					 tool->addCollisionMesh(object);

					 // Set up the device
					 tool->initialize();

					 // Set up a nice-looking workspace for the phantom so 
                     // it fits nicely with our shape
					 tool->setWorkspace(2.0, 2.0, 2.0);

					 // Rotate the tool so its axes align with our opengl-like axes
					 tool->rotate(cVector3d(0,0,1), -90.0*3.14159/180.0);
					 tool->rotate(cVector3d(1,0,0), -90.0*3.14159/180.0);
					 tool->setRadius(0.05);
				 }
				 tool->computeGlobalPositions();

				 // Open communication with the device
				 tool->start();

				 // Enable forces
				 tool->setForcesON();

#ifdef USE_MM_TIMER_FOR_HAPTICS

				 // Set the callback timer to call the haptic_iteration function every 100 milliseconds,
				 // since we aren't providing any actual force feedback
				 mm_timer->set(0, haptic_iteration, tool);

#else
                 DWORD thread_id;
                 ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)(haptic_loop), tool, 0, &thread_id);

                 // Boost thread and process priority
                 ::SetThreadPriority(&thread_id, THREAD_PRIORITY_ABOVE_NORMAL);

#endif
			 }

			 // If haptics is already on, turn it off...
			 else {
				 haptics_enabled = 0;

				 // Stop the timer and the haptic device...
				 mm_timer->stop();
				 tool->setForcesOFF();
				 tool->stop();
			 }

		 }

private: System::Void Form1_Closing(System::Object ^  sender, System::ComponentModel::CancelEventArgs ^  e)
		 {
			 // Stop the haptic timer and device if they are on
			 if (haptics_enabled) {
				 mm_timer->stop();
				 tool->setForcesOFF();
				 tool->stop();
			 }
		 }

};
}


void haptic_iteration(void* param) {
	cGeneric3dofPointer* tool = (cGeneric3dofPointer*)(param);

	// Just update the pose and compute forces (collisions); no forces are being applied in this example
	tool->updatePose();
	tool->computeForces();
	//tool->applyForces();
}


// This loop is used only in the threaded version of this
// application... all it does is call the main haptic
// iteration loop, which is called directly from a timer
// callback if USE_MM_TIMER_FOR_HAPTICS is defined
DWORD haptic_loop(void* param) {

  cGeneric3dofPointer* tool = (cGeneric3dofPointer*)(param);

  while(haptics_enabled) {
    haptic_iteration(param);
  }
  return 0;
}


