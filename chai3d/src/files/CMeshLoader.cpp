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

#include "CMeshLoader.h"

//===========================================================================
/*!
	 Global function to load a file into a mesh (CHAI currently supports
	 .3ds and .obj files).  Returns true if the file is loaded successfully.

	 The file type is determined based on the file extension supplied by
	 the caller.

	  \fn     bool cLoadMeshFromFile(cMesh* iMesh, const string& iFileName);
	  \param  iMesh  The mesh into which we should write the loaded data
	  \param  iFileName The filename from which we should load the mesh
	  \return true if the file is loaded successfully, false for an error
*/
//===========================================================================
bool cLoadMeshFromFile(cMesh* iMesh, const string& iFileName) {

	const char* filename = iFileName.c_str();
	char* extension = find_extension(filename);

	// We need a file extension to figure out file type
	if (extension == 0) {
		return false;
	}

	char lower_extension[1024];
	string_tolower(lower_extension, extension);

	// Load an .obj file
	if (strcmp(lower_extension, "obj") == 0) {

		return cLoadFileOBJ(iMesh, iFileName);

	}

	// Load a .3ds file
	else if (strcmp(lower_extension, "3ds") == 0) {

		return cLoadFile3DS(iMesh, iFileName);

	}

	// Unrecognized format...
	else return false;

}
