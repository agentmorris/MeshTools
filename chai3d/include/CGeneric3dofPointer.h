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
#ifndef CGeneric3dofPointerH
#define CGeneric3dofPointerH
//---------------------------------------------------------------------------
#include "CGenericTool.h"
#include "CGenericDevice.h"
#include "CColor.h"
#include "CProxyPointForceAlgo.h"
#include "CPotentialFieldForceAlgo.h"
//---------------------------------------------------------------------------

typedef enum {
  RENDER_PROXY=0,RENDER_DEVICE,RENDER_PROXY_AND_DEVICE
} proxy_render_modes;
   
//===========================================================================
/*!
      \file       CGeneric3dofPointer.h
      \class      cGeneric3dofPointer
      \brief      cGeneric3dofPointer represents a haptic tool that 
                  can apply forces in three degrees of freedom and
                  maintains three or six degrees of device pose.

                  This class provides i/o with haptic devices and
                  a basic graphical representation of a tool.
*/
//===========================================================================
class cGeneric3dofPointer : public cGenericTool
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cGeneric3dofPointer.
    cGeneric3dofPointer(cWorld* a_world);
    //! Destructor of cGeneric3dofPointer.
    virtual ~cGeneric3dofPointer();

    // METHODS:

    // Graphics

    //! Render the object in OpenGL 
    virtual void render(const int a_renderMode=0);
    //! toggle on/off the tool frame
    virtual inline void visualizeFrames(const bool& a_showToolFrame) { m_showToolFrame = a_showToolFrame; }
    //! set the visual settings of the tool frame
    virtual void setToolFrame(const bool& a_showToolFrame, const double& a_toolFrameSize);
    //! Control proxy rendering options
    /*! Set to RENDER_PROXY, RENDER_DEVICE, or RENDER_PROXY_AND_DEVICE
    */
    virtual inline void setRenderingMode(const proxy_render_modes& render_mode) { m_render_mode = render_mode; }

    // Initialization / shutdown

    //! Start communication with the device connected to the tool (0 indicates success)
    virtual int start();
    //! Stop communication with the device connected to the tool (0 indicates success)
    virtual int stop();
    //! Initialize the device connected to the tool (0 indicates success).
    virtual int initialize(const bool a_resetEncoders=false);  
    //! Toggle forces on
    virtual int setForcesON();
    //! Toggle forces off
    virtual int setForcesOFF();

    // Data transfer

    //! Update position and orientation of the device.
    virtual void updatePose();
    //! Compute interaction forces with environment.
    virtual void computeForces();
    //! Apply latest computed forces to device.
    virtual void applyForces();

    // Miscellaneous 

    // Returns a scale factor from normalized coordinates to millimeters.  
    virtual cVector3d getWorkspaceScaleFactors();

    //! Set radius of pointer.
    virtual void setRadius(const double& a_radius);
    //! Set haptic device driver.
    virtual void setDevice(cGenericDevice *a_device);
    //! Get haptic device driver.
    virtual cGenericDevice* getDevice() { return m_device; }
    //! This is a convenience function; it searches the list of force functions for a proxy
    virtual cProxyPointForceAlgo* getProxy();

    //! Set virtual workspace dimensions in which tool will be working.
    virtual void setWorkspace(const double& a_workspaceAxisX, const double& a_workspaceAxisY,
                      const double& a_workspaceAxisZ);

    //! Enable or disable normalized position values (vs. absolute mm) (defaults to true, i.e. normalized)
    virtual void useNormalizedPositions(const bool& a_useNormalizedPositions)
      { m_useNormalizedPositions = a_useNormalizedPositions; }
    //! Are we currently using normalized position values (vs. absolute mm)?
    virtual bool getNormalizedPositionsEnabled() { return m_useNormalizedPositions; }

    // MEMBERS:
    //! Color of sphere representing position of device.
    cColorf m_colorDevice;
    //! Color of sphere representing position of tool (proxy).
    cColorf m_colorProxy;
    //! Color of sphere representing position of tool (proxy) when switch in ON.
    cColorf m_colorProxyButtonOn;
    //! Color of line connecting proxy and device position together
    cColorf m_colorLine;

    //! Orientation of wrist in local coordinates of device
    cMatrix3d m_deviceLocalRot;
    //! Orientation of wrist in global coordinates of device
    cMatrix3d m_deviceGlobalRot;

    //! Normally this class waits for a very small force before initializing forces
    //! to avoid initial "jerks" (a safety feature); you can bypass that requirement
    //! with this variable
    bool m_waitForSmallForce;

    //! Vector of force algorithms.  By default, a proxy algorithm object and a potential
    //! field object are added to this list (in that order).
    //!
    //! When a tool is asked to compute forces, it walks this list and asks each algorithm
    //! to compute its forces.
    std::vector<cGenericPointForceAlgo*> m_pointForceAlgos;

    //! Width of workspace.   Ignored when m_useNormalizedPositions is false.
    double m_halfWorkspaceAxisX;
    //! Height of workspace.  Ignored when m_useNormalizedPositions is false.
    double m_halfWorkspaceAxisY;
    //! Depth of workspace.   Ignored when m_useNormalizedPositions is false.
    double m_halfWorkspaceAxisZ;
    //! Position of device in device local coordinate system
    cVector3d m_deviceLocalPos;
    //! Position of device in world global coordinate system
    cVector3d m_deviceGlobalPos;
    //! Velocity of device in device local coordinate system
    cVector3d m_deviceLocalVel;
    //! Velocity of device in world global coordinate system
    cVector3d m_deviceGlobalVel;

    //! The last force computed for application to this tool, in the world coordinate
    //! system.  (N)
    //!
    //! If you want to manually send forces to a device, you can modify this
    //! value before calling 'applyForces'.
    cVector3d m_lastComputedGlobalForce;

    //! The last force computed for application to this tool, in the device coordinate
    //! system.  (N)
    cVector3d m_lastComputedLocalForce;    

  protected:
    // MEMBERS:
    //! Radius of sphere representing position of pointer.
    double m_displayRadius;
    //! haptic device driver.
    cGenericDevice *m_device;
    //! World in which tool is interacting
    cWorld* m_world;
    //! flag for frame visualization of the proxy
    bool m_showToolFrame;
    //! size of the frame visualization of the proxy
    double m_toolFrameSize;
    //! Should we render the device position, the proxy position, or both?
    proxy_render_modes m_render_mode;
    //! Should we be returning normalized (vs. absolute _mm_) positions?
    bool m_useNormalizedPositions;
    //! this flag records whether the user has enabled forces
    bool m_forceON;
    //! flag to avoid initial bumps in force (has the user sent a _small_ force yet?)
    bool m_forceStarted;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

