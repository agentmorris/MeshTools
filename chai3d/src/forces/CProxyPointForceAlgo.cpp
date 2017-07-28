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
	\author:    Chris Sewell
	\author:    Dan Morris
	\version    1.2
	\date       03/2005
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CProxyPointForceAlgo.h"
//---------------------------------------------------------------------------
#include "CMesh.h"
#include "CWorld.h"
#include <float.h>

//---------------------------------------------------------------------------

// Controls whether the proxy searches invisible objects
//
// If this constant is zero, the proxy will use visible _and_
// invisible objects.
#define CHAI_PROXY_ONLY_USES_VISIBLE_OBJECTS 0

// Using the Melder et al friction model, it becomes useful to decouple the
// _angle_ used for dynamic friction from the actual friction coefficient.
//
// In CHAI's proxy, the angle computed from the coefficient is multiplied
// by this constant to avoid rapidly oscillating between slipping and sticking
// without having to turn the dynamic friction level way down.
#define DYNAMIC_FRICTION_HYSTERESIS_MULTIPLIER 0.6


//---------------------------------------------------------------------------

//===========================================================================
/*!
	Constructor of cProxyPointForceAlgo.

	\fn       cProxyPointForceAlgo::cProxyPointForceAlgo()
*/
//===========================================================================
cProxyPointForceAlgo::cProxyPointForceAlgo()
{
	// initialize world pointer
	m_world = NULL;

	// no contacts yet between proxy and environment
	m_numContacts = 0;

	// initialize device and proxy positions
	m_deviceGlobalPos.zero();
	m_proxyGlobalPos.zero();
	m_lastGlobalForce.zero();

	// set default colors
	m_colorProxy.set(1.0f, 0.7f, 0.0, 1.0f);
	m_colorLine.set(0.5f, 0.5f, 0.5f);

	// this will generally be over-written by the calling pointer
	m_radius = 0.1f;

	// by default, we do not use dynamic proxy (which handles moving objects)
	m_dynamicProxy = false;

	// initialize dynamic proxy members
	m_lastObjectGlobalPos.set(0, 0, 0);
	m_lastObjectGlobalRot.identity();
	m_touchingObject = 0;
	m_numContacts = 0;
	m_movingObject = 0;

	m_slipping = true;
	m_useFriction = true;
	m_useZillesFriction = false;
	m_useMelderFriction = true;
}


//===========================================================================
/*!
	Initialize the algorithm, including setting the pointer to the world
	in which the algorithm is to operate, and setting the initial position
	of the device.

	\fn       void cProxyPointForceAlgo::initialize(cWorld* a_world,
			  const cVector3d& a_initialPos)
	\param    a_world  Pointer to world in which force algorithm is operating.
	\param    a_initialPos  Initial position of the device.
*/
//===========================================================================
void cProxyPointForceAlgo::initialize(cWorld* a_world, const cVector3d& a_initialPos)
{
	// no contacts yet between proxy and environment
	m_numContacts = 0;

	m_slipping = 1;

	// initialize device and proxy positions
	m_deviceGlobalPos = a_initialPos;
	m_proxyGlobalPos = a_initialPos;
	m_lastGlobalForce.zero();

	// set pointer to world in which force algorithm operates
	m_world = a_world;
}


//===========================================================================
/*!
	This method computes the force to add to the device due to any collisions
	with meshes by calling computeNextBestProxyPosition() to update the
	proxy location and then computeForce() to calculate a force vector based
	on the proxy location.

	\fn       cVector3d cProxyPointForceAlgo::computeForces(
			  const cVector3d& a_nextDevicePos)
	\param    a_nextDevicePos  Current position of the device.
	\return   Return the force to add to the device due to any collisions
			  with meshes.
*/
//===========================================================================
cVector3d cProxyPointForceAlgo::computeForces(const cVector3d& a_nextDevicePos)
{
	// check if world has been defined; if so, compute forces
	if (m_world != NULL)
	{
		// update position of device
		m_deviceGlobalPos = a_nextDevicePos;

		// if dynamic proxy is enabled, account for object motion
		if (m_dynamicProxy) correctProxyForObjectMotion();

		// compute next best position of proxy
		computeNextBestProxyPosition(m_deviceGlobalPos);

		// update proxy to next best position
		m_proxyGlobalPos = m_nextBestProxyGlobalPos;

		// compute force vector applied to device
		computeForce();

		// Update "last-state" dynamic contact information
		updateDynamicContactState();

		// return result
		return (m_lastGlobalForce);
	}

	// if no world has been defined in which algorithm operates, there is no force
	else
	{
		return (cVector3d(0.0, 0.0, 0.0));
	}
}


//===========================================================================
/*!
	Given the new position of the device and considering the current
	position of the proxy, this function attempts to move the proxy towards
	the device position (the goal).  If its path is blocked by an obstacle
	(e.g., a triangle in a mesh), the proxy is moved to this intersection
	point and a new goal is calculated as the closest point to the original
	goal in the half-plane above the intersection triangle.
	The process is repeated if necessary, bringing the proxy to its
	final location.

	\fn   void cProxyPointForceAlgo::computeNextBestProxyPosition(cVector3d a_goal)
	\param  a_goal  The goal towards which to move the proxy, subject to constraints
*/
//===========================================================================
void cProxyPointForceAlgo::computeNextBestProxyPosition(cVector3d a_goal)
{
	// local variable declarations

	// Each of these variables is re-used in each of three collision passes...

	// Did we hit a triangle this pass?
	bool hit = false;

	//! Initialize normal and tangential forces.
	m_normalForce.set(0, 0, 0);
	m_tangentialForce.set(0, 0, 0);

	// What object, triangle, and point did we collide with?
	cGenericObject* colObject = 0;
	cTriangle* colTriangle = 0;
	cVector3d colPoint;

	// This is the parent object of the triangle we hit, which should really
	// be the same as colObject...
	cGenericObject* parent = 0;

	// How far away was the collision from the current proxy position?
	double colDistance = DBL_MAX;

	// The three vertices of the triangle we most recently collided with...
	cVector3d vertex0, vertex1, vertex2;

	// The current positions of the proxy and the goal
	cVector3d proxy, goal;

	// temporary variable to used to describe a segment path from the proxy to its
	// next goal position
	cVector3d segmentPointB;

	// A vector from the proxy to the goal
	cVector3d vProxyGoal;

	// A vector from the most recent collision point to the proxy
	cVector3d vColProxy;

	// The offset we'll apply to each goal position to "push it away" from
	// the collision plane, to account for the proxy's radius.
	cVector3d goalOffset;

	// The goal we try to move the proxy to _after_ finding a constraint plane
	// (i.e. after the proxy->goal segment is found to hit an object)
	cVector3d colGoal;

	// The force exerted on the proxy by the user
	cVector3d force;

	// The surface normal of each of the three triangles we come in contact with.
	// These variables are not re-used across collisions.
	cVector3d normal0, normal1, normal2;

	// Used for maintaining a minimum distance between the proxy and all
	// constraint planes...
	double cosAngle, distanceTriangleProxy;

	// A vector from the device to the proxy; used to orient surface normals
	cVector3d vDeviceProxy;

	// Read the current position of proxy and device; the device position
	// is considered the initial goal position for the proxy.
	proxy = m_proxyGlobalPos;
	goal = a_goal;

	// Store the previous number of contacts for debugging...
	int previousNumberOfContacts = m_numContacts;

	// No contacts with triangles have occurred yet.
	m_numContacts = 0;

	// If the distance between the proxy and the goal position (device) is
	// very small then we can be considered done.
	if (goalAchieved(proxy, goal))
	{
		m_nextBestProxyGlobalPos = proxy;
		m_numContacts = 0;
		m_touchingObject = 0;
		return;
	}

	// Test whether the path from the proxy to the goal is obstructed.
	// For this we create a segment that goes from the proxy position to
	// the goal position plus a little extra to take into account the
	// physical radius of the proxy.
	segmentPointB = goal;
	offsetGoalPosition(segmentPointB, proxy);

	hit = m_world->computeCollisionDetection(proxy, segmentPointB, colObject,
		colTriangle, colPoint, colDistance, CHAI_PROXY_ONLY_USES_VISIBLE_OBJECTS, 1);

	// If no collision occurs, then we move the proxy to its goal, and we're done
	if (hit == false)
	{
		m_nextBestProxyGlobalPos = goal;
		m_numContacts = 0;
		m_touchingObject = 0;
		m_slipping = 0;
		return;
	}

	m_touchingPoint = colPoint;

	//-----------------------------------------------------------------------
	// FIRST COLLISION:
	//-----------------------------------------------------------------------

	// Since a collision has occurred with a triangle, we store a pointer to
	// the intersected triangle and increment the number of contacts.
	m_triangle0 = colTriangle;
	m_numContacts = 1;

	// A first collision has occurred; we first compute the global positions
	// of each vertex of the triangle we hit...
	parent = colTriangle->getParent();

	cMatrix3d rot = parent->getGlobalRot();
	cVector3d pos = parent->getGlobalPos();

	rot.mulr(colTriangle->getVertex0()->getPos(), vertex0);
	vertex0.add(pos);
	rot.mulr(colTriangle->getVertex1()->getPos(), vertex1);
	vertex1.add(pos);
	rot.mulr(colTriangle->getVertex2()->getPos(), vertex2);
	vertex2.add(pos);

	// Compute the triangle surface normal in world coordinates
	normal0 = cComputeSurfaceNormal(vertex0, vertex1, vertex2);

	// Align the surface normal to point away from the device
	vDeviceProxy = cSub(proxy, a_goal);
	vDeviceProxy.normalize();
	if (cDot(normal0, vDeviceProxy) < 0) normal0.negate();

	// Compute a vector from the proxy to the goal (device)
	goal.subr(proxy, vProxyGoal);

	// We want the center of the proxy to move as far toward the triangle as it can,
	// but we want it to stop when the _sphere_ representing the proxy hits the
	// triangle.  We want to compute how far the proxy center will have to
	// be pushed _away_ from the collision point - along the vector from the proxy
	// to the goal - to keep a distance m_radius between the proxy center and the
	// triangle.
	//
	// So we compute the cosine of the angle between the normal and proxy-goal vector...
	vProxyGoal.normalize();
	cosAngle = vProxyGoal.dot(normal0);

	// Now we compute how far away from the collision point - _backwards_
	// along vProxyGoal - we have to put the proxy to keep it from penetrating
	// the triangle.
	//
	// If only ASCII art were a little more expressive...
	distanceTriangleProxy = m_radius / cAbs(cosAngle);

	// We compute the projection of the vector between the proxy and the collision
	// point onto the normal of the triangle.  This is the direction in which
	// we'll move the _goal_ to "push it away" from the triangle (to account for
	// the radius of the proxy).

	// A vector from the collision point to the proxy...
	proxy.subr(colPoint, vColProxy);

	// Move the proxy to the collision point, minus the distance along the
	// movement vector that we computed above.
	//
	// Note that we're adjusting the 'proxy' variable, which is just a local
	// copy of the proxy position.  We still might decide not to move the
	// 'real' proxy due to friction.
	vColProxy.normalize();
	vColProxy.mul(distanceTriangleProxy);
	colPoint.addr(vColProxy, proxy);

	// If the distance between the proxy and the goal position (device) is
	// very small then we can be considered done.
	if (goalAchieved(proxy, goal))
	{
		m_nextBestProxyGlobalPos = proxy;
		m_touchingObject = parent;
		return;
	}

	// The proxy is now constrained on a plane; we now calculate the nearest
	// point to the original goal (device position) on this plane; this point
	// is computed by projecting the ideal goal onto the plane defined by the
	// intersected triangle
	goal = cProjectPointOnPlane(a_goal, vertex0, vertex1, vertex2);

	// Since the proxy has a radius, the new goal position is offset to be
	// on the same side of the plane as the proxy, so the proxy will not
	// penetrate the triangle.
	goalOffset = normal0;
	goalOffset.mul(m_radius);
	goal.add(goalOffset);

	// Before moving the proxy to this new goal position, we need to check
	// if a second triangle could stop the proxy from reaching its new goal.
	//
	// We compute a new collision segment going from the proxy towards
	// the goal position...
	goal.subr(proxy, colGoal);

	// If the distance between the proxy and the new goal position is
	// smaller than CHAI_SMALL, we consider the proxy to be at the same position
	// as the goal, and we're done.
	if (colGoal.length() < CHAI_SMALL)
	{
		m_nextBestProxyGlobalPos = proxy;
		m_touchingObject = parent;
		return;
	}


	if (m_useZillesFriction)
	{
		// Calculate static friction, and see if we should move the _real_ proxy
		// position...

		// Calculate the force exerted on the proxy by the user as the stiffness
		// coefficient of the intersected triangle's mesh times the vector between the
		// proxy and the device...
		force = cMul(colTriangle->getParent()->m_material.getStiffness(),
			cSub(proxy, a_goal));

		// Calculate the normal component of that force...
		m_normalForce = cProject(force, normal0);

		// Calculate the tangential component of that force...
		m_tangentialForce = cSub(force, m_normalForce);

		// If the magnitude of the tangential force is less than the
		// static friction coefficient times the magnitude of the normal force,
		// the user's position is in the friction cone of the surface, and the
		// proxy should not be moved.
		if (m_tangentialForce.length() <
			colTriangle->getParent()->m_material.getStaticFriction()*m_normalForce.length())
		{
			m_nextBestProxyGlobalPos = m_proxyGlobalPos;
			m_touchingObject = parent;
			return;
		}
	}

	// If the distance between the proxy and the goal position (device) is
	// very small then we can be considered done.
	if (goalAchieved(proxy, goal))
	{
		m_nextBestProxyGlobalPos = proxy;
		m_numContacts = 0;
		m_touchingObject = 0;
		return;
	}

	// Test whether the path along the virtual line between the updated proxy
	// and its new goal is obstructed
	segmentPointB = goal;
	offsetGoalPosition(segmentPointB, proxy);

	hit = m_world->computeCollisionDetection(proxy, segmentPointB, colObject,
		colTriangle, colPoint, colDistance, CHAI_PROXY_ONLY_USES_VISIBLE_OBJECTS, 2);

	// If no collision occurs, we move the proxy to its goal, unless
	// friction prevents us from doing so.

	if (hit == false)
	{
		testFrictionAndMoveProxy(goal, proxy, normal0, parent);
		return;
	}

	//-----------------------------------------------------------------------
	// SECOND COLLISION:
	//-----------------------------------------------------------------------

	// Since a collision has occurred with a triangle, we store a pointer to
	// the intersected triangle and increment the number of contacts.
	m_triangle1 = colTriangle;
	m_numContacts = 2;

	// A second collision has occurred; we first compute the global positions
	// of each vertex of the triangle from the mesh to which the triangle
	// belongs (its parent)
	parent = colTriangle->getParent();

	rot = parent->getGlobalRot();
	pos = parent->getGlobalPos();

	rot.mulr(colTriangle->getVertex0()->getPos(), vertex0);
	vertex0.add(pos);
	rot.mulr(colTriangle->getVertex1()->getPos(), vertex1);
	vertex1.add(pos);
	rot.mulr(colTriangle->getVertex2()->getPos(), vertex2);
	vertex2.add(pos);

	// Next we compute the triangle surface normal in world coordinates...
	normal1 = cComputeSurfaceNormal(vertex0, vertex1, vertex2);

	// Align the surface normal to point away from the device
	vDeviceProxy = cSub(proxy, a_goal);
	vDeviceProxy.normalize();
	if (cDot(normal1, vDeviceProxy) < 0) normal1.negate();

	// Compute a vector from the proxy to the goal...
	goal.subr(proxy, vProxyGoal);

	// Compute the distance we'll have to push the proxy "backwards" along
	// the proxy-goal vector to account for the proxy's radius.
	//
	// See above for a more detailed explanation of what's happening here.
	vProxyGoal.normalize();
	cosAngle = vProxyGoal.dot(normal1);
	distanceTriangleProxy = m_radius / cAbs(cosAngle);

	// Move the proxy to its new position, preventing it from quite reaching
	// the goal to account for the proxy radius.
	vProxyGoal.mul(distanceTriangleProxy);
	colPoint.subr(vProxyGoal, proxy);

	// If the distance between the proxy and the new goal position is
	// smaller than CHAI_SMALL, we consider the proxy to be at the same position
	// as the goal, and we're done.
	if (goalAchieved(proxy, goal))
	{
		m_nextBestProxyGlobalPos = proxy;
		m_touchingObject = parent;
		return;
	}

	// The proxy is now constrained by two triangles and can only move along
	// a virtual line; we now calculate the nearest point to the original
	// goal (device position) along this line by projecting the ideal
	// goal onto the line.
	//
	// The line is expressed by the cross product of both surface normals,
	// which have both been oriented to point away from the device
	cVector3d line;
	normal0.crossr(normal1, line);

	// check result.
	if (line.equals(cVector3d(0, 0, 0)))
	{
		m_nextBestProxyGlobalPos = proxy;
		m_touchingObject = parent;
		return;
	}

	// Compute the projection of the device position (goal) onto the line; this
	// gives us the new goal position.
	goal = cProjectPointOnLine(a_goal, proxy, line);

	// Before moving the proxy to this new goal position, we need to check
	// if a third triangle could stop the proxy from reaching its new goal.
	goal.subr(proxy, colGoal);

	// If the distance between the proxy and the new goal position is
	// smaller than CHAI_SMALL, we consider the proxy to be at the same position
	// as the goal, and we're done.
	if (colGoal.length() < CHAI_SMALL)
	{
		m_nextBestProxyGlobalPos = proxy;
		m_touchingObject = parent;
		return;
	}

	if (m_useZillesFriction)
	{
		// Calculate static friction, and see if we should move the _real_ proxy
		// position...

		// Calculate the force exerted on the proxy by the user as the stiffness
		// coefficient of the intersected triangle's mesh times the vector between the
		// proxy and the device.
		force = cMul(colTriangle->getParent()->m_material.getStiffness(),
			cSub(proxy, a_goal));

		// Calculate the normal component of this force
		m_normalForce = cProject(force, normal1);

		// Calculate the tangential component of the force
		m_tangentialForce = cSub(force, m_normalForce);

		// If the magnitude of the tangential force is less than the
		// static friction coefficient times the magnitude of the normal force,
		// the user's position is in the friction cone of the surface, and the
		// proxy should not be moved.
		if (m_tangentialForce.length() <
			colTriangle->getParent()->m_material.getStaticFriction()*m_normalForce.length())
		{
			m_nextBestProxyGlobalPos = m_proxyGlobalPos;
			m_touchingObject = parent;
			return;
		}
	}

	// If the distance between the proxy and the goal position (device) is
	// very small then we can be considered done.
	if (goalAchieved(proxy, goal))
	{
		m_nextBestProxyGlobalPos = proxy;
		m_numContacts = 0;
		m_touchingObject = 0;
		return;
	}

	// Test whether the path along the virtual line between the updated proxy
	// and its new goal is obstructed
	segmentPointB = goal;
	offsetGoalPosition(segmentPointB, proxy);

	hit = m_world->computeCollisionDetection(proxy, segmentPointB, colObject,
		colTriangle, colPoint, colDistance, CHAI_PROXY_ONLY_USES_VISIBLE_OBJECTS, 3);

	// If no collision occurs, we move the proxy to its goal, unless
	// friction prevents us from doing so

	if (hit == false)
	{
		testFrictionAndMoveProxy(goal, proxy, cMul(0.5, cAdd(normal0, normal1)), parent);
		return;
	}

	//-----------------------------------------------------------------------
	// THIRD COLLISION:
	//-----------------------------------------------------------------------

	// The proxy is now constrained to a single point just above the third triangle
	m_triangle2 = colTriangle;
	m_numContacts = 3;

	// A third collision has occurred; we first compute the global positions
	// of each vertex of the triangle...
	parent = colTriangle->getParent();

	rot = parent->getGlobalRot();
	pos = parent->getGlobalPos();

	rot.mulr(colTriangle->getVertex0()->getPos(), vertex0);
	vertex0.add(pos);
	rot.mulr(colTriangle->getVertex1()->getPos(), vertex1);
	vertex1.add(pos);
	rot.mulr(colTriangle->getVertex2()->getPos(), vertex2);
	vertex2.add(pos);

	// Next we compute the triangle surface normal in world coordinates...
	normal2 = cComputeSurfaceNormal(vertex0, vertex1, vertex2);

	// Compute a vector from the proxy to the goal point...
	goal.subr(proxy, vProxyGoal);

	// Compute the distance we'll have to push the proxy "backwards" along
	// the proxy-goal vector to account for the proxy's radius.
	//
	// See above for a more detailed explanation of what's happening here.
	vProxyGoal.normalize();
	cosAngle = vProxyGoal.dot(normal2);
	distanceTriangleProxy = m_radius / cAbs(cosAngle);

	// Note that we don't really have to do another "goal offset" here, because
	// we're moving the proxy along a line, and we will make sure not to move
	// the proxy too close to the colliding triangle...

	// Move the proxy to the collision point, minus the distance along the
	// movement vector that we computed above.
	//
	// Note that we're adjusting the 'proxy' variable, which is just a local
	// copy of the proxy position.  We still might decide not to move the
	// 'real' proxy due to friction.
	vProxyGoal.normalize();
	vProxyGoal.mul(distanceTriangleProxy);
	colPoint.subr(vProxyGoal, proxy);

	// TODO: There actually should be a third friction test to see if we
	// can make it to our new goal position, but this is generally such a
	// small movement in one iteration that it's irrelevant...

	m_nextBestProxyGlobalPos = proxy;
	m_touchingObject = parent;
	return;
}


//===========================================================================
/*!
	Test whether the proxy has reached the goal point, allowing for subclass-
	specific approximations.

	\fn   virtual bool cProxyPointForceAlgo::goalAchieved(const cVector3d& a_proxy, const cVector3d& a_goal) const;
	\param    a_goal        The location to which we'd like to move the proxy
	\param    a_proxy       The current position of the proxy
	\return   true is the proxy has effectively reached the goal
*/
//===========================================================================
bool cProxyPointForceAlgo::goalAchieved(const cVector3d& a_proxy, const cVector3d& a_goal) const
{
	if (m_dynamicProxy) return false;
	return (a_proxy.distance(a_goal) < CHAI_SMALL);
}


//===========================================================================
/*!
	Offset the current goal position to account for the volume/shape of the proxy.

	\fn   virtual void cProxyPointForceAlgo::offsetGoalPosition(cVector3d& a_goal, const cVector3d& a_proxy) const;
	\param    a_goal        The location to which we'd like to move the proxy, offset upon return
	\param    a_proxy       The current position of the proxy
*/
//===========================================================================
void cProxyPointForceAlgo::offsetGoalPosition(cVector3d& a_goal, const cVector3d& a_proxy) const
{
	if (m_dynamicProxy) return;
	a_goal = cAdd(a_goal, cMul(m_radius, cNormalize(cSub(a_goal, a_proxy))));
}



//===========================================================================
/*!
  Attempt to move the proxy, subject to friction constraints.  This is called
  from computeNextBestProxyPosition when the proxy is ready to move along a
  known surface.

  \fn   void cProxyPointForceAlgo::testFrictionAndMoveProxy(const cVector3d& goal, const cVector3d& proxy,
			 cVector3d normal, cGenericObject* parent)
  \param    goal        The location to which we'd like to move the proxy
  \param    proxy       The current position of the proxy
  \param    normal      The surface normal at the obstructing surface
  \param    parent      The surface along which we're moving
*/
//===========================================================================
void cProxyPointForceAlgo::testFrictionAndMoveProxy(const cVector3d& goal, const cVector3d& proxy,
	cVector3d normal, cGenericObject* parent)
{
	if (m_useFriction == false || m_useMelderFriction == false)
	{
		m_nextBestProxyGlobalPos = goal;
		m_touchingObject = parent;
		return;
	}

	// Compute penetration depth; how far is the device "behind" the
	// plane of the obstructing surface
	cVector3d projectedGoal = cProjectPointOnPlane(m_deviceGlobalPos, proxy, normal);
	double penetrationDepth = cSub(m_deviceGlobalPos, projectedGoal).length();

	// Find the appropriate friction coefficient

	// Our dynamic and static coefficients...
	cMesh* parent_mesh = dynamic_cast<cMesh*>(parent);

	// Right now we can only work with cMesh's
	if (parent_mesh == 0)
	{
		m_nextBestProxyGlobalPos = goal;
		m_touchingObject = parent;
		return;
	}

	double mud = parent_mesh->m_material.getDynamicFriction();
	double mus = parent_mesh->m_material.getStaticFriction();

	// No friction; don't try to compute friction cones
	if (mud == 0 && mus == 0)
	{
		m_nextBestProxyGlobalPos = goal;
		m_touchingObject = parent;
		return;
	}

	// The corresponding friction cone radii
	double atmd = atan(mud);
	double atms = atan(mus);

	// Compute a vector from the device to the proxy, for computing
	// the angle of the friction cone
	cVector3d vDeviceProxy = cSub(proxy, m_deviceGlobalPos);
	vDeviceProxy.normalize();

	// Now compute the angle of the friction cone...
	double theta = acos(vDeviceProxy.dot(normal));

	// Manage the "slip-friction" state machine

	// If the dynamic friction radius is for some reason larger than the
	// static friction radius, always slip
	if (mud > mus)
	{
		m_slipping = 1;
	}

	// If we're slipping...
	else if (m_slipping)
	{
		if (theta < atmd * DYNAMIC_FRICTION_HYSTERESIS_MULTIPLIER) m_slipping = false;
		else m_slipping = true;
	}

	// If we're not slipping...
	else
	{
		if (theta > atms) m_slipping = true;
		else m_slipping = false;
	}

	// The friction coefficient we're going to use...
	double mu;
	if (m_slipping) mu = mud;
	else mu = mus;

	// Calculate the friction radius as the absolute value of the penetration
	// depth times the coefficient of friction
	double frictionRadius = fabs(penetrationDepth * mu);

	// Calculate the distance between the proxy position and the current
	// goal position.
	double r = proxy.distance(goal);

	// If this distance is smaller than CHAI_SMALL, we consider the proxy
	// to be at the same position as the goal, and we're done...
	if (r < CHAI_SMALL)
	{
		m_nextBestProxyGlobalPos = proxy;
	}

	// If the proxy is outside the friction cone, update its position to
	// be on the perimeter of the friction cone...
	else if (r > frictionRadius)
	{
		m_nextBestProxyGlobalPos = cAdd(goal, cMul(frictionRadius / r, cSub(proxy, goal)));
	}

	// A hack to prevent the proxy from getting stuck in corners...
	else if (m_numContacts >= 2)
	{
		m_nextBestProxyGlobalPos = proxy;
	}

	// Otherwise, if the proxy is inside the friction cone, the proxy
	// should not be moved (set next best position to current position)
	else
	{
		m_nextBestProxyGlobalPos = proxy;
	}

	// We're done; record the fact that we're still touching an object...
	m_touchingObject = parent;
	return;
}

//===========================================================================
/*!
	This method uses the information computed earlier in
	computeNextProxyPosition() to determine the force to apply to the device.
	The function computes a force proportional to the distance between the
	positions of the proxy and the device and scaled by the average
	stiffness of each contact triangle.

	\fn       void cProxyPointForceAlgo::computeForce()
*/
//===========================================================================
void cProxyPointForceAlgo::computeForce()
{
	// A local stiffness is computed by computing the average stiffness
	// for each triangle we're in contact with...
	double stiffness;

	// if there are no contacts between proxy and environment, no force is applied
	if (m_numContacts == 0)
	{
		m_lastGlobalForce.zero();
		return;
	}

	// if there is one contact point, the stiffness is equal to the stiffness
	// of the one intersected triangle's mesh
	else if (m_numContacts == 1)
	{
		stiffness = (m_triangle0->getParent()->m_material.getStiffness());
	}

	// if there are two contact points, the stiffness is the average of the
	// stiffnesses of the two intersected triangles' meshes
	else if (m_numContacts == 2)
	{
		stiffness = (m_triangle0->getParent()->m_material.getStiffness() +
			m_triangle1->getParent()->m_material.getStiffness()) / 2.0;
	}

	// if there are three contact points, the stiffness is the average of the
	// stiffnesses of the three intersected triangles' meshes
	else if (m_numContacts == 3)
	{
		stiffness = (m_triangle0->getParent()->m_material.getStiffness() +
			m_triangle1->getParent()->m_material.getStiffness() +
			m_triangle2->getParent()->m_material.getStiffness()) / 3.0;
	}

	// compute the force by modeling a spring between the proxy and the device
	cVector3d force;
	m_proxyGlobalPos.subr(m_deviceGlobalPos, force);
	force.mul(stiffness);
	m_lastGlobalForce = force;
}


//===========================================================================
/*!
	Return the number of current contacts, and the associated triangles in the
	output parameters.

	\fn       unsigned int cProxyPointForceAlgo::getContacts(cTriangle*& a_t0,
			  cTriangle*& a_t1, cTriangle*& a_t2);
	\param    a_t0  Returns pointer to first contact triangle.
	\param    a_t1  Returns pointer to second contact triangle.
	\param    a_t2  Returns pointer to third contact triangle.
	\return   Return the number of current contacts.
*/
//===========================================================================
unsigned int cProxyPointForceAlgo::getContacts(cTriangle*& a_t0, cTriangle*& a_t1,
	cTriangle*& a_t2)
{
	// set the triangle pointers to the contact triangles
	a_t0 = (m_numContacts > 0) ? m_triangle0 : 0;
	a_t1 = (m_numContacts > 1) ? m_triangle1 : 0;
	a_t2 = (m_numContacts > 2) ? m_triangle2 : 0;

	// return the number of triangle intersections
	return m_numContacts;
}


//===========================================================================
/*!
	This method sets the dynamic proxy state to reflect new contact
	information.

	\fn       void cProxyPointForceAlgo::updateDynamicContactState();
*/
//===========================================================================
void cProxyPointForceAlgo::updateDynamicContactState()
{
	// Update "last-state" dynamic contact information
	if (m_dynamicProxy && m_movingObject)
	{
		cGenericObject* savedTouchingObject = m_touchingObject;
		m_touchingObject = m_movingObject;
		if (m_touchingObject != 0)
		{
			m_lastObjectGlobalPos = m_touchingObject->getGlobalPos();
			m_lastObjectGlobalRot = m_touchingObject->getGlobalRot();
		}
		m_touchingObject = savedTouchingObject;
		return;
	}

	// if the proxy is not currently in contact with any object, no update
	// is needed
	if (m_touchingObject == 0) return;

	// update the position and rotation of the object currently being touched
	m_lastObjectGlobalPos = m_touchingObject->getGlobalPos();
	m_lastObjectGlobalRot = m_touchingObject->getGlobalRot();
}


//===========================================================================
/*!
	This method lets the proxy move along with the object it's touching, if
	the object has moved since the previous proxy iteration.

	\fn       void cProxyPointForceAlgo::correctProxyForObjectMotion();
*/
//===========================================================================
void cProxyPointForceAlgo::correctProxyForObjectMotion()
{
	if (m_dynamicProxy && m_movingObject) m_touchingObject = m_movingObject;

	// if the proxy is not currently in contact with any object, no update
	// is needed
	if (m_touchingObject == NULL) return;

	// start with the non-dynamic proxy position
	cVector3d newGlobalProxy = m_proxyGlobalPos;

	// get the position and rotation of the object contacted in the last iteration
	cVector3d lastGlobalPos;
	getContactObjectLastGlobalPos(lastGlobalPos);
	cMatrix3d lastGlobalRot;
	getContactObjectLastGlobalRot(lastGlobalRot);

	// combine the rotation and translation into one matrix
	cMatrix3d lastGlobalRotT;
	lastGlobalRot.transr(lastGlobalRotT);

	// convert the last segment point A (proxy position) into the
	// _current_ reference frame of the object
	newGlobalProxy.sub(lastGlobalPos);

	// apply rotation as necessary
	lastGlobalRotT.mul(newGlobalProxy);
	m_touchingObject->getGlobalRot().mul(newGlobalProxy);

	// convert to global coordinates
	newGlobalProxy.add(m_touchingObject->getGlobalPos());

	// this ends up being the proxy in global coordinates
	m_proxyGlobalPos = newGlobalProxy;
}
