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
	\author:    Francois Conti
	\author:    Federico Barbagli
	\version    1.1
	\date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CGeneric3dofPointer.h"
#include "CWorld.h"
//---------------------------------------------------------------------------

// The radius used for proxy collision detection is equal to 
// CHAI_SCALE_PROXY_RADIUS * the radius that's rendered
#define CHAI_SCALE_PROXY_RADIUS 0.001f

//==========================================================================
/*!
	  Constructor of cGeneric3dofPointer.

	  \fn       cGeneric3dofPointer::cGeneric3dofPointer(cWorld* a_world)
	  \param    a_world  World in which the tool will operate.
*/
//===========================================================================
cGeneric3dofPointer::cGeneric3dofPointer(cWorld* a_world)
{
	m_waitForSmallForce = true;

	// set world
	m_world = a_world;

	// create a default proxy algorithm force renderer
	cProxyPointForceAlgo* default_proxy = new cProxyPointForceAlgo;
	m_pointForceAlgos.push_back(default_proxy);

	// create a default potential field force renderer
	cPotentialFieldForceAlgo* potentialFields = new cPotentialFieldForceAlgo();
	m_pointForceAlgos.push_back(potentialFields);

	// set up the default proxy
	default_proxy->initialize(m_world, cVector3d(0, 0, 0));
	default_proxy->setProxyRadius(m_displayRadius * CHAI_SCALE_PROXY_RADIUS);

	// set a default device for the moment
	m_device = new cGenericDevice();

	// default tool rendering settings
	m_colorDevice.set(1.0f, 0.2f, 0.0);
	m_colorProxyButtonOn.set(1.0f, 0.4f, 0.0);
	m_colorProxy.set(0.8f, 0.6f, 0.0);
	m_colorLine.set(0.7f, 0.7f, 0.7f);
	m_render_mode = RENDER_PROXY_AND_DEVICE;

	// This sets both the rendering radius and the actual proxy radius
	setRadius(0.05f);

	// tool frame settings
	m_showToolFrame = false;   // toggle tool frame off
	m_toolFrameSize = 0.2;      // default value for the tool frame size

	// force settings
	m_forceON = true;           // forces are ON at first
	m_forceStarted = false;

	// set workspace size
	m_halfWorkspaceAxisX = 0.1;
	m_halfWorkspaceAxisY = 0.1;
	m_halfWorkspaceAxisZ = 0.1;

	// init device related variables
	m_deviceGlobalPos.zero();
	m_deviceLocalPos.zero();
	m_lastComputedLocalForce.zero();
	m_lastComputedGlobalForce.zero();
	m_deviceLocalVel.zero();
	m_deviceGlobalVel.zero();

	// Use normalized positions by default
	m_useNormalizedPositions = true;
}


//==========================================================================
/*!
	  Destructor of cGeneric3dofPointer.

	  \fn       cGeneric3dofPointer::~cGeneric3dofPointer()
*/
//===========================================================================
cGeneric3dofPointer::~cGeneric3dofPointer()
{
	// check if device is available
	if (m_device == NULL) { return; }

	// close device driver
	m_device->close();

	delete m_device;
}


//==========================================================================
/*!
	Define a haptic device driver for this tool.

	\fn       void cGeneric3dofPointer::setDevice(cGenericDevice *a_device);
	\param    a_device  This is the device that should be associated with
						this tool.
*/
//===========================================================================
void cGeneric3dofPointer::setDevice(cGenericDevice *a_device)
{
	m_device = a_device;
}


//==========================================================================
/*!
	Initialize device

	\fn       void cGeneric3dofPointer::initialize(const bool a_resetEncoders=false)
	\param    a_resetEncoders  If true, this resets the device's 0 position
			  to the current position (if this device supports re-zero'ing).
			  That means that if your device supports re-zero'ing (e.g. Phantom
			  premiums), you should be careful about calling this function
			  in the middle of your program with a_resetEncoders set to 'true'.
			  Or if you do call it with a_resetEncoders set to true, you should
			  make sure your user knows to hold the Phantom in place.
	\return   0 indicates success, non-zero indicates an error
*/
//===========================================================================
int cGeneric3dofPointer::initialize(const bool a_resetEncoders)
{
	// check if device is available
	if (m_device == NULL) { return -1; }

	// initialize (calibrate) device
	if (m_device->open() != 0) return -1;
	if (m_device->initialize(a_resetEncoders) != 0) return -1;
	updatePose();
	if (m_device->close() != 0) return -1;

	// initialize all force models
	unsigned int i;
	for (i = 0; i < m_pointForceAlgos.size(); i++)
	{
		if (m_pointForceAlgos[i] != NULL)
		{
			m_pointForceAlgos[i]->initialize(m_world, m_deviceGlobalPos);
		}
	}

	return 0;
}


//==========================================================================
/*!
	  Starts communication with the haptic device.

	  \fn       void cGeneric3dofPointer::start()
	  \return   0 indicates success, non-zero indicates an error
*/
//===========================================================================
int cGeneric3dofPointer::start()
{
	// check if device is available
	if (m_device == NULL) { return -1; }

	// open connection to device
	return m_device->open();
}


//==========================================================================
/*!
	  Stop system. Apply zero force to device

	  \fn       void cGeneric3dofPointer::stop()
	  \return   0 indicates success, non-zero indicates an error
*/
//===========================================================================
int cGeneric3dofPointer::stop()
{
	// check if device is available
	if (m_device == NULL) { return -1; }

	// stop the device
	return m_device->close();
}


//==========================================================================
/*!
	  Sets the virtual volume in which the virtual tool will be moving.

	  Normalized coordinates received from the device (ranging from
	  -1.0 --> 1.0) are multiplied by half of the workspace edges to get
	  the values that are used to position the pointer.

	  \fn       void cGeneric3dofPointer::setWorkspace(const double& a_width,
				const double& a_height, const double& a_depth)
	  \param    a_width   Width of workspace.
	  \param    a_height  Height of workspace.
	  \param    a_depth   Depth of workspace.
*/
//===========================================================================
void cGeneric3dofPointer::setWorkspace(const double& a_workspaceAxisX,
	const double& a_workspaceAxisY, const double& a_workspaceAxisZ)
{
	m_halfWorkspaceAxisX = a_workspaceAxisX / 2.0;
	m_halfWorkspaceAxisY = a_workspaceAxisY / 2.0;
	m_halfWorkspaceAxisZ = a_workspaceAxisZ / 2.0;
}


//==========================================================================
/*!
	  Update position of pointer and orientation of wrist.

	  \fn       void cGeneric3dofPointer::updatePose()
*/
//===========================================================================
void cGeneric3dofPointer::updatePose()
{
	cVector3d pos;

	// check if device is available
	if (m_device == NULL) { return; }

	// read local position of device in normalized units or mm
	int result;
	if (m_useNormalizedPositions)
		result = m_device->command(CHAI_CMD_GET_POS_NORM_3D, &pos);
	else
		result = m_device->command(CHAI_CMD_GET_POS_3D, &pos);

	if (result != CHAI_MSG_OK) { return; }

	if (m_useNormalizedPositions == true)
	{
		// scale position from device into virtual workspace of tool
		m_deviceLocalPos.set(m_halfWorkspaceAxisX * pos.x,
			m_halfWorkspaceAxisY * pos.y,
			m_halfWorkspaceAxisZ * pos.z);
	}
	else {
		m_deviceLocalPos.set(pos.x, pos.y, pos.z);
	}

	// update global position of tool
	cVector3d tPos;
	m_globalRot.mulr(m_deviceLocalPos, tPos);
	tPos.addr(m_globalPos, m_deviceGlobalPos);

	// read orientation of wrist
	m_device->command(CHAI_CMD_GET_ROT_MATRIX, &m_deviceLocalRot);

	// update global orientation of tool
	m_deviceLocalRot.mulr(m_globalRot, m_deviceGlobalRot);

	// read switches
	m_device->command(CHAI_CMD_GET_SWITCH_MASK, &m_button);

	// read velocity of the device in local coordinates
	m_device->command(CHAI_CMD_GET_VEL_3D, &m_deviceLocalVel);

	// update global velocity of tool
	m_globalRot.mulr(m_deviceLocalVel, m_deviceGlobalVel);
}


//===========================================================================
/*!
	  Compute the interaction forces between the tool and the virtual
	  object inside the virtual world.

	  \fn       void cGeneric3dofPointer::computeForces()
*/
//===========================================================================
void cGeneric3dofPointer::computeForces()
{
	unsigned int i;

	// temporary variable to store forces
	cVector3d force;
	force.zero();

	// compute forces in world coordinates for each point force algorithm
	for (i = 0; i < m_pointForceAlgos.size(); i++)
	{
		if (m_pointForceAlgos[i] != NULL)
		{
			force.add(m_pointForceAlgos[i]->computeForces(m_deviceGlobalPos));
		}
	}

	// copy result
	m_lastComputedGlobalForce.copyfrom(force);
}


//==========================================================================
/*!
	  Apply the latest computed force to the device.

	  \fn       void cGeneric3dofPointer::applyForces()
*/
//===========================================================================
void cGeneric3dofPointer::applyForces()
{
	// check if device is available
	if (m_device == NULL) { return; }

	// convert force into device local coordinates
	cMatrix3d tRot;
	m_globalRot.transr(tRot);
	tRot.mulr(m_lastComputedGlobalForce, m_lastComputedLocalForce);

	if (
		(m_waitForSmallForce == false)
		||
		((!m_forceStarted) && (m_lastComputedLocalForce.lengthsq() < 0.000001))
		)
		m_forceStarted = true;

	// send force to device
	if ((m_forceON) && (m_forceStarted))
	{
		m_device->command(CHAI_CMD_SET_FORCE_3D, &m_lastComputedLocalForce);
	}
	else
	{
		cVector3d ZeroForce = cVector3d(0.0, 0.0, 0.0);
		m_device->command(CHAI_CMD_SET_FORCE_3D, &ZeroForce);
	}
}


//==========================================================================
/*!
	  Render the current tool in OpenGL.

	  \fn       void cGeneric3dofPointer::render(const int a_renderMode=0)
	  \param    a_renderMode  rendering mode; see cGenericObject.cpp.
*/
//===========================================================================
void cGeneric3dofPointer::render(const int a_renderMode)
{
	// If multipass transparency is enabled, only render on a single
	// pass...
	if (a_renderMode != CHAI_RENDER_MODE_NON_TRANSPARENT_ONLY && a_renderMode != CHAI_RENDER_MODE_RENDER_ALL)
		return;

	// render small sphere representing tip of tool
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// OpenGL matrix
	cMatrixGL frameGL;

	// compute local position of proxy
	cProxyPointForceAlgo* default_proxy = getProxy();
	cVector3d proxyLocalPos;
	if (default_proxy != NULL)
	{
		proxyLocalPos = default_proxy->getProxyGlobalPosition();
		proxyLocalPos.sub(m_globalPos);
		cMatrix3d tRot;
		m_globalRot.transr(tRot);
		tRot.mul(proxyLocalPos);
	}

	if ((m_render_mode != RENDER_DEVICE) && (default_proxy != NULL))
	{
		// render proxy

		// Button 0 determines the color of the proxy
		if (m_button & 1)
		{
			glColor4fv(m_colorProxyButtonOn.pColor());
		}
		else
		{
			glColor4fv(m_colorProxy.pColor());
		}

		frameGL.set(proxyLocalPos);
		frameGL.glMatrixPushMultiply();
		cDrawSphere(m_displayRadius, 16, 16);
		frameGL.glMatrixPop();
	}

	if (m_render_mode != RENDER_PROXY)
	{
		// render device position
		if (m_button & 1) {
			glColor4fv(m_colorProxyButtonOn.pColor());
		}
		else {
			glColor4fv(m_colorDevice.pColor());
		}
		frameGL.set(m_deviceLocalPos);
		frameGL.glMatrixPushMultiply();
		cDrawSphere(m_displayRadius, 16, 16);
		frameGL.glMatrixPop();

		if (m_showToolFrame)
		{
			// render device orientation
			frameGL.set(m_deviceLocalPos, m_deviceLocalRot);
			frameGL.glMatrixPushMultiply();
			cDrawFrame(m_toolFrameSize);
			frameGL.glMatrixPop();
		}
	}

	if ((m_render_mode == RENDER_PROXY_AND_DEVICE) && (default_proxy != NULL))
	{
		// render line between device and proxy
		glDisable(GL_LIGHTING);
		glLineWidth(1.0);
		glColor4fv(m_colorLine.pColor());
		glBegin(GL_LINES);
		glVertex3d(m_deviceLocalPos.x, m_deviceLocalPos.y, m_deviceLocalPos.z);
		glVertex3d(proxyLocalPos.x, proxyLocalPos.y, proxyLocalPos.z);
		glEnd();
		glEnable(GL_LIGHTING);
	}

	// Really useful debugging code for showing which triangles the proxy is
	// in contact with...

// #define RENDER_PROXY_CONTACT_TRIANGLES
#ifdef RENDER_PROXY_CONTACT_TRIANGLES
	if ((m_proxyPointForceAlgo.getContactObject()) && (default_proxy != NULL)) {

		cTriangle *t0, *t1, *t2;

		cGenericObject* obj = default_proxy->getContactObject(); //m_proxyPointForceAlgo.getContactObject();
		obj->computeGlobalCurrentObjectOnly();

		int result = default_proxy->getContactObject(); //m_proxyPointForceAlgo.getContacts(t0,t1,t2);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glPushMatrix();
		glLoadMatrixd(m_world->m_worldModelView);
		glBegin(GL_TRIANGLES);

		if (t0) {

			glColor3f(1.0, 0, 0);
			t0->getVertex0()->computeGlobalPosition(obj->getGlobalPos(), obj->getGlobalRot());
			t0->getVertex1()->computeGlobalPosition(obj->getGlobalPos(), obj->getGlobalRot());
			t0->getVertex2()->computeGlobalPosition(obj->getGlobalPos(), obj->getGlobalRot());

			glVertex3d(t0->getVertex0()->m_globalPos.x, t0->getVertex0()->m_globalPos.y, t0->getVertex0()->m_globalPos.z);
			glVertex3d(t0->getVertex1()->m_globalPos.x, t0->getVertex1()->m_globalPos.y, t0->getVertex1()->m_globalPos.z);
			glVertex3d(t0->getVertex2()->m_globalPos.x, t0->getVertex2()->m_globalPos.y, t0->getVertex2()->m_globalPos.z);
		}

		if (t1) {

			glColor3f(0, 1.0, 0);
			t1->getVertex0()->computeGlobalPosition(obj->getGlobalPos(), obj->getGlobalRot());
			t1->getVertex1()->computeGlobalPosition(obj->getGlobalPos(), obj->getGlobalRot());
			t1->getVertex2()->computeGlobalPosition(obj->getGlobalPos(), obj->getGlobalRot());

			glVertex3d(t1->getVertex0()->m_globalPos.x, t1->getVertex0()->m_globalPos.y, t1->getVertex0()->m_globalPos.z);
			glVertex3d(t1->getVertex1()->m_globalPos.x, t1->getVertex1()->m_globalPos.y, t1->getVertex1()->m_globalPos.z);
			glVertex3d(t1->getVertex2()->m_globalPos.x, t1->getVertex2()->m_globalPos.y, t1->getVertex2()->m_globalPos.z);
		}

		if (t2) {

			glColor3f(0, 0, 1.0);
			t2->getVertex0()->computeGlobalPosition(obj->getGlobalPos(), obj->getGlobalRot());
			t2->getVertex1()->computeGlobalPosition(obj->getGlobalPos(), obj->getGlobalRot());
			t2->getVertex2()->computeGlobalPosition(obj->getGlobalPos(), obj->getGlobalRot());

			glVertex3d(t2->getVertex0()->m_globalPos.x, t2->getVertex0()->m_globalPos.y, t2->getVertex0()->m_globalPos.z);
			glVertex3d(t2->getVertex1()->m_globalPos.x, t2->getVertex1()->m_globalPos.y, t2->getVertex1()->m_globalPos.z);
			glVertex3d(t2->getVertex2()->m_globalPos.x, t2->getVertex2()->m_globalPos.y, t2->getVertex2()->m_globalPos.z);
		}

		glEnd();
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
	}
#endif

}


//==========================================================================
/*!
	  Set the radius of the proxy. The value passed as parameter corresponds
	  to the size of the sphere which is rendered graphically. The physical
	  size of the proxy, one which collides with the triangles is set to
	  CHAI_SCALE_PROXY_RADIUS * a_radius.

	  \fn       void cGeneric3dofPointer::setRadius(const double& a_radius)
	  \param    a_radius  radius of pointer.
*/
//===========================================================================
void cGeneric3dofPointer::setRadius(const double& a_radius)
{
	// update the radius that's rendered
	m_displayRadius = a_radius;

	// update the radius used for collision detection
	cProxyPointForceAlgo* default_proxy = getProxy();
	if (default_proxy != NULL)
		default_proxy->setProxyRadius(a_radius * CHAI_SCALE_PROXY_RADIUS);
}


//==========================================================================
/*!
	  This function searches the vector of point force algorithms and
			returns the first proxy (instance of cProxyPointForceAlgo) that
			it can find.  If it does not find any, it returns NULL.  By default,
			a proxy was entered into this list in the constructor, so it should
			return this default proxy unless you have removed it.

	  This is a convenience function.  If you want to find all the available
	  proxies or all the available force-rendering algorithms, you should
	  search the m_pointForceAlgos list.

	  \fn       void cGeneric3dofPointer::getProxy()
*/
//===========================================================================
cProxyPointForceAlgo* cGeneric3dofPointer::getProxy()
{
	for (unsigned int i = 0; i < m_pointForceAlgos.size(); i++)
	{
		cProxyPointForceAlgo* default_proxy = dynamic_cast<cProxyPointForceAlgo*>(m_pointForceAlgos[i]);
		if (default_proxy != NULL)
			return default_proxy;
	}
	return NULL;
}


//==========================================================================
/*!
	  Toggles on and off the visualization of a reference frame
	  located on the tool's point.

	  \fn       void cGeneric3dofPointer::setToolFrame(const bool& a_showToolFrame, const double& a_toolFrameSize)
	  \param    a_showToolFrame Flag which controls the tool frame display.
	  \param    a_toolFrameSize Size of the tool frame.
*/
//===========================================================================
void cGeneric3dofPointer::setToolFrame(const bool& a_showToolFrame, const double& a_toolFrameSize)
{
	m_showToolFrame = a_showToolFrame;
	m_toolFrameSize = a_toolFrameSize;
}


//==========================================================================
/*!
	Turns forces ON

	\fn     void cGeneric3dofPointer::setForcesON()
	\return   0 indicates success, non-zero indicates an error

*/
//===========================================================================
int cGeneric3dofPointer::setForcesON()
{
	if (!m_forceON)
	{
		m_forceStarted = false;
		m_forceON = true;
	}
	return 0;
}


//==========================================================================
/*!
	Turns forces OFF

	\fn       void cGeneric3dofPointer::setForcesOFF()
	\return   0 indicates success, non-zero indicates an error
*/
//===========================================================================
int cGeneric3dofPointer::setForcesOFF()
{
	m_forceON = false;
	return 0;
}


//==========================================================================
/*!
	Returns scale factors from normalized coordinates to millimeters.  I.e., if
	you take pointer position and multiply by these values, you get millimeters.

	If a device does not provide this information, zeros are returned.

	By default, cGeneric3dofPointer accesses the _normalized_ device position;
	this lets you convert back to absolute coordinates.

	\fn       cVector3d cGeneric3dofPointer::getWorkspaceScaleFactors()
	\return   Scale factors: coordinates * scale = mm
*/
//===========================================================================
cVector3d cGeneric3dofPointer::getWorkspaceScaleFactors() {
	double scale;
	int result = m_device->command(CHAI_CMD_GET_NORMALIZED_SCALE_FACTOR, &scale);
	if (result != CHAI_MSG_OK) { return cVector3d(0.0, 0.0, 0.0); }

	cVector3d toReturn(1.0 / m_halfWorkspaceAxisX, 1.0 / m_halfWorkspaceAxisY, 1.0 / m_halfWorkspaceAxisZ);
	toReturn *= scale;
	return toReturn;
}

