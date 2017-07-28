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

#include "stdafx.h"
#include "CMeshTool.h"
#include "CWorld.h"


//===========================================================================
/*!
    Constructor of cMeshTool

    \fn       cMeshTool::cMeshTool(cWorld* a_world, unsigned int a_deviceNumber, 
                                   bool a_dio_access)
    \param    a_world  World in which the tool will operate.
    \param    a_deviceNumber  0-based index used to try to open a phantom
                              device.

    \param    dio_access  Also used only if the tool ends up being a phantom AND
              you're running GHOST, not OpenHaptics.  If you're not using a Phantom
              or you're using OpenHaptics, ignore this parameter.
                
              For Ghost, if dio_access is 'true', the tool will use the Ghost API's
              direct-i/o model, if it's available.  Otherwise the gstEffect i/o model
              will be used.
*/
//===========================================================================
cMeshTool::cMeshTool(cWorld* a_world, unsigned int a_deviceNumber, bool a_dio_access) :
  cMeta3dofPointer(a_world, a_deviceNumber, a_dio_access)
{
    // Create a new mesh-mesh collision detector
    m_collisionDetector = new cCollisionMeshMesh();
    m_collisionDetector->setUseNeighbors(false);

    // Create a yellow material for when tool mesh is not in collision
    m_material1.setStiffness(30.0);
    m_material1.m_ambient.set(0.8, 0.8, 0.0, 1.0);
    m_material1.m_diffuse.set(0.8, 0.8, 0.0, 1.0);
    m_material1.m_specular.set(1.0, 1.0, 1.0, 1.0);
    m_material1.setShininess(100);
    m_freeColor.set(200, 200, 0);

    // Create a red material for when tool mesh is in collision
    m_material2.setStiffness(30.0);
    m_material2.m_ambient.set(0.8, 0.0, 0.0, 1.0);
    m_material2.m_diffuse.set(0.8, 0.0, 0.0, 1.0);
    m_material2.m_specular.set(1.0, 1.0, 1.0, 1.0);
    m_material2.setShininess(100);
    m_collisionColor.set(200, 0, 0);

    m_hit = 0;
}


//===========================================================================
/*!
    Sets the mesh to use to represent this tool.

    \fn       void cMeshTool::setMesh(cMesh* a_mesh)
    \param    a_mesh   The mesh to set to represent this tool
*/
//===========================================================================
void cMeshTool::setMesh(cMesh* a_mesh)
{
    // Set the mesh to represent this tool
    m_mesh = a_mesh; 
    m_mesh->setMaterial(m_material1, true);

    // Create a list of the tool mesh's submeshes
    m_toolMeshes.clear();
    std::vector<cGenericObject*> objects_to_descend;
    objects_to_descend.push_back(m_mesh);
    while (!objects_to_descend.empty())
    {
        cGenericObject* cur_object = objects_to_descend.back();
        objects_to_descend.pop_back();

        // Add each child that is a cMesh to the list of submeshes
        cMesh* cur_mesh = dynamic_cast<cMesh*>(cur_object);
        if (cur_mesh)
            m_toolMeshes.push_back(cur_mesh);

        // Add this child's children to the list of objects to process
        for (unsigned int i=0; i<cur_object->getNumChildren(); i++)
            objects_to_descend.push_back(cur_object->getChild(i));
    }
}


//===========================================================================
/*!
    Update the pose of the tool mesh to match the current tool pose.

    \fn       void cMeshTool::render(const int a_renderMode)
    \param    a_renderMode   Rendering mode
*/
//===========================================================================
void cMeshTool::render(const int a_renderMode)
{
    // Set the position and rotation of the tool mesh
    if ((m_mesh) && (m_collisionDetector))
    {
        computeGlobalPositions(false);
        m_mesh->setRot(cMul(m_globalRot, m_deviceLocalRot));
        m_mesh->setPos(cAdd(cMul(m_deviceLocalRot, m_globalPos), m_deviceLocalPos));
        m_mesh->setPos(m_deviceGlobalPos);
        m_mesh->computeGlobalPositions(false);

        // Set the tool mesh's color depending on whether it is colliding with something
        if (m_hit == 1)
                m_mesh->setMaterial(m_material2, true);
            else
                m_mesh->setMaterial(m_material1, true);

        // Set vertex colors when not finding all collisions
        if (!m_collisionDetector->getFindAll())
        {
            if (m_mesh->getColorsEnabled())
                if (m_hit == 1)
                    m_mesh->setVertexColor(m_collisionColor, true);
                else
                    m_mesh->setVertexColor(m_freeColor, true);
            unsigned int i;
            for (i=0; i<m_collisionMeshes.size(); i++)
                if (m_collisionMeshes[i]->getColorsEnabled())
                    if (m_hit == 1)
                        m_collisionMeshes[i]->setVertexColor(m_collisionColor, true);
                    else
                        m_collisionMeshes[i]->setVertexColor(m_freeColor, true);
        }

        // Set vertex colors when we are finding all collisions
        else
        {
            std::vector<cTriangle*> cur_tris = m_tris;
            m_mesh->setVertexColor(m_freeColor, true);
            unsigned int i,j;
            for (i=0; i<m_collisionMeshes.size(); i++)
                m_collisionMeshes[i]->setVertexColor(m_freeColor, true);
            cColorb contact_color;
            contact_color.set(200,0,0);   
            for (i=0; i<cur_tris.size(); i++)
                for (j=0; j<3; j++)
                    if ((cur_tris[i]) && (cur_tris[i]->getVertex(j)))
                        cur_tris[i]->getVertex(j)->setColor(m_collisionColor);
        }
    }  
}


//===========================================================================
/*!
    Adds a mesh which this tool should check for collisions against.

    \fn       void cMeshTool::addCollisionMesh(cMesh* a_mesh)
    \param    a_mesh   The mesh to add (along with its submesh descendants)
*/
//===========================================================================
void cMeshTool::addCollisionMesh(cMesh* a_mesh)
{
    // Create a list of the tool mesh's submeshes
    std::vector<cGenericObject*> objects_to_descend;
    objects_to_descend.push_back(a_mesh);
    while (!objects_to_descend.empty())
    {
        cGenericObject* cur_object = objects_to_descend.back();
        objects_to_descend.pop_back();

        // Add each child that is a cMesh to the list of submeshes
        cMesh* cur_mesh = dynamic_cast<cMesh*>(cur_object);
        if (cur_mesh)
            m_collisionMeshes.push_back(cur_mesh);

        // Add this child's children to the list of objects to process
        for (unsigned int i=0; i<cur_object->getNumChildren(); i++)
            objects_to_descend.push_back(cur_object->getChild(i));
    }
}


//===========================================================================
/*!
    Check for collisions between the tool mesh and all other meshes in 
    the world using a cCollisionMeshMesh object

    \fn       void cMeshTool::computeForces()
*/
//===========================================================================
void cMeshTool::computeForces()
{
    // Set force to zero
    cVector3d force;
    force.zero();
    int hit = 0;
 
    m_world->computeGlobalPositions(false);
    std::vector<cTriangle*> tri1, tri2;
        tri1.clear();
            tri2.clear();

    // Iterate through all children of the tool mesh
    unsigned int i = 0;
    while ((i<m_toolMeshes.size()) && ((!hit) || (m_collisionDetector->getFindAll() == 1)))
    {
        cMesh* submesh = m_toolMeshes[i];

        unsigned int j = 0;
        while ((j<m_collisionMeshes.size()) && ((!hit) || (m_collisionDetector->getFindAll() == 1)))
        {
            cMesh* cur_mesh = m_collisionMeshes[j];

            // Check for a collision between the tool mesh and this mesh          
            if (m_collisionDetector->computeCollision(submesh, cur_mesh, tri1, tri2))
                hit = 1;
            j++;
        }
        i++;
    }

    m_hit = hit;

    for (i=0; i<tri2.size(); i++)
        tri1.push_back(tri2[i]);
    m_tris = tri1;

    // Copy force result (just zero here)
    m_lastComputedGlobalForce.copyfrom(force);
}
