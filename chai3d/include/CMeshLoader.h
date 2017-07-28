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
    \date       03/2004
*/
//===========================================================================
/*!
    \file CMeshLoader.h
*/
//---------------------------------------------------------------------------
#ifndef CMeshLoaderH
#define CMeshLoaderH
//---------------------------------------------------------------------------
#include "CMatrix3d.h"
#include "CVector3d.h"
#include "CVertex.h"
#include "CTriangle.h"
#include "CMesh.h"
#include "CMaterial.h"
#include "CTexture2D.h"
#include "CWorld.h"
#include "CLight.h"
#include <string>

// Specific file format loaders
#include "CFileLoaderOBJ.h"
#include "CFileLoader3DS.h"

//! Global function to load a file into a mesh (CHAI currently supports .3ds and .obj files).
bool cLoadMeshFromFile(cMesh* iMesh, const string& iFileName);

#endif
