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

#include "CMeta3dofPointer.h"
#include "CMesh.h"
#include "CCollisionMeshMesh.h"

//===========================================================================
/*!
      \class      cMeshTool
      \brief      cMeshTool extends cMeta3dofPointer for detecting collisions
                    between a tool mesh and other meshes in the world
*/
//===========================================================================
class cMeshTool : public cMeta3dofPointer
{
  public:
    // CONSTRUCTOR AND DESTRUCTOR:
    //! Constructor of cMeshTool
    cMeshTool(cWorld* a_world, unsigned int a_deviceNumber = 0, bool a_dio_access = false);

    // METHODS:
    //! Update the pose of the tool mesh to match the current tool pose
    virtual void render(const int a_renderMode=0);
    //! Find collisions between the tool mesh and all other meshes in the world
    virtual void computeForces();
    //! Sets the mesh to use to represent this tool
    virtual void setMesh(cMesh* a_mesh);
    //! Adds a mesh which this tool should check for collisions against
    virtual void addCollisionMesh(cMesh* a_mesh);

    // PROPERTIES:
    //! The mesh that moves with the tool
    cMesh* m_mesh;
    //! A mesh to mesh collision detector
    cCollisionMeshMesh* m_collisionDetector;
    //! Material when mesh is not in contact
    cMaterial m_material1;
    //! Material when mesh is in contact
    cMaterial m_material2;
    //! List of all the tool mesh's submeshes
    std::vector<cMesh*> m_toolMeshes;
    //! List of all submeshes against which to check for collision
    std::vector<cMesh*> m_collisionMeshes;
    //! Is the tool mesh currently in contact with a mesh?
    int m_hit;
    //! List of triangles currently in collision
    std::vector<cTriangle*> m_tris;
    //! The vertex color for vertices in collision
    cColorb m_collisionColor;
    //! The vertex color for vertices not in collision
    cColorb m_freeColor;
};
