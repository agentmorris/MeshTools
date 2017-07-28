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
	\author:    Tim Schröder
	\author:    Francois Conti
	\version    1.1
	\date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CFileLoaderOBJ.h"
//---------------------------------------------------------------------------

bool g_objLoaderShouldGenerateExtraVertices = false;

//===========================================================================
/*!
	Load a Wavefront OBJ file format image into a mesh.

	\fn         bool cLoadFileOBJ(cMesh* a_mesh, const string& a_fileName)
	\param      a_mesh         Mesh in which image file is loaded
	\param      a_fileName     Name of image file.
	\return     Return \b true if image was loaded successfully, otherwise
				return \b false.
*/
//===========================================================================
bool cLoadFileOBJ(cMesh* a_mesh, const string& a_fileName)
{
	cOBJModel fileObj;

	// load file into memory. If an error occurs, exit.
	if (!fileObj.LoadModel(a_fileName.c_str())) { return (false); };

	// get information about mesh
	cWorld* world = a_mesh->getParentWorld();

	// clear all vertices and triangle of current mesh
	a_mesh->clear();

	// get information about file
	int numMaterials = fileObj.m_OBJInfo.m_materialCount;
	int numNormals = fileObj.m_OBJInfo.m_normalCount;
	int numTexCoord = fileObj.m_OBJInfo.m_texCoordCount;

	// extract materials
	vector<cMaterial> materials;

	// object has no material properties
	if (numMaterials == 0)
	{
		// create a new child
		cMesh *newMesh = a_mesh->createMesh();
		a_mesh->addChild(newMesh);

		// Give him a default color
		a_mesh->setVertexColor(cColorb(255, 255, 255, 255), 1);
		a_mesh->useColors(1, 1);
		a_mesh->useMaterial(0, 1);
		a_mesh->enableTransparency(0, 1);
	}

	// object has material properties. Create a child for each material
	// property.
	else
	{

		int i = 0;
		bool found_transparent_material = false;

		while (i < numMaterials)
		{
			// create a new child
			cMesh *newMesh = a_mesh->createMesh();
			a_mesh->addChild(newMesh);

			// get next material
			cMaterial newMaterial;
			cMaterialInfo material = fileObj.m_pMaterials[i];

			int textureId = material.m_textureID;
			if (textureId >= 1)
			{
				cTexture2D *newTexture = world->newTexture();
				int result = newTexture->loadFromFile(material.m_texture);

				// If this didn't work out, try again in the obj file's path
				if (result == 0) {

					char model_dir[1024];
					const char* fname = a_fileName.c_str();

					find_directory(model_dir, fname);

					char new_texture_path[1024];
					sprintf(new_texture_path, "%s/%s", model_dir, material.m_texture);

					result = newTexture->loadFromFile(new_texture_path);
				}

				if (result) newMesh->setTexture(newTexture);

				// We really failed to load a texture...
				else {
#ifdef _WIN32
					// CHAI_DEBUG_PRINT("Could not load texture map %s\n",material.m_texture);
#endif
				}
			}

			float alpha = material.m_alpha;
			if (alpha < 1.0) {
				newMesh->enableTransparency(1, 0);
				found_transparent_material = true;
			}

			// get ambient component:
			newMesh->m_material.m_ambient.setR(material.m_ambient[0]);
			newMesh->m_material.m_ambient.setG(material.m_ambient[1]);
			newMesh->m_material.m_ambient.setB(material.m_ambient[2]);
			newMesh->m_material.m_ambient.setA(alpha);

			// get diffuse component:
			newMesh->m_material.m_diffuse.setR(material.m_diffuse[0]);
			newMesh->m_material.m_diffuse.setG(material.m_diffuse[1]);
			newMesh->m_material.m_diffuse.setB(material.m_diffuse[2]);
			newMesh->m_material.m_diffuse.setA(alpha);

			// get specular component:
			newMesh->m_material.m_specular.setR(material.m_specular[0]);
			newMesh->m_material.m_specular.setG(material.m_specular[1]);
			newMesh->m_material.m_specular.setB(material.m_specular[2]);
			newMesh->m_material.m_specular.setA(alpha);

			// get emissive component:
			newMesh->m_material.m_emission.setR(material.m_emmissive[0]);
			newMesh->m_material.m_emission.setG(material.m_emmissive[1]);
			newMesh->m_material.m_emission.setB(material.m_emmissive[2]);
			newMesh->m_material.m_emission.setA(alpha);

			// get shininess
			newMaterial.setShininess((GLuint)(material.m_shininess));

			i++;
		}

		// Enable material property rendering
		a_mesh->useColors(0, 1);
		a_mesh->useMaterial(1, 1);

		// Mark the presence of transparency in the root mesh; don't
		// modify the value stored in children...
		a_mesh->enableTransparency(found_transparent_material, 0);

	}

	// Keep track of vertex mapping in each mesh; maps "old" vertices
	// to new vertices
	int nMeshes = a_mesh->getNumChildren();
	vertexIndexSet_uint_map* vertexMaps = new vertexIndexSet_uint_map[nMeshes];
	vertexIndexSet_uint_map::iterator vertexMapIter;

	// build object
	{
		int i = 0;

		// get triangles
		int numTriangles = fileObj.m_OBJInfo.m_faceCount;
		int j = 0;
		while (j < numTriangles)
		{
			// get next face
			cFace face = fileObj.m_pFaces[j];

			// get material index attributed to the face
			int objIndex = face.m_materialIndex;

			// the mesh that we're reading this triangle into
			cMesh* curMesh = (cMesh*)a_mesh->getChild(objIndex);

			// create a name for this mesh if necessary (over-writing a previous
			// name if one has been written)
			if ((face.m_groupIndex >= 0) && (fileObj.m_groupNames.size() > 0))
				strncmp(curMesh->m_objectName, fileObj.m_groupNames[face.m_groupIndex], CHAI_MAX_OBJECT_NAME_LENGTH);

			// get the vertex map for this mesh
			vertexIndexSet_uint_map* curVertexMap = &(vertexMaps[objIndex]);

			// number of vertices on face
			int vertCount = face.m_numVertices;

			if (vertCount >= 3) {

				int indexV1 = face.m_pVertexIndices[0];

				if (g_objLoaderShouldGenerateExtraVertices == false) {
					vertexIndexSet vis(indexV1);
					if (numNormals > 0) vis.nIndex = face.m_pNormalIndices[0];
					if (numTexCoord > 0) vis.tIndex = face.m_pTextureIndices[0];
					indexV1 = getVertexIndex(curMesh, &fileObj, curVertexMap, vis);
				}

				for (int triangleVert = 2; triangleVert < vertCount; triangleVert++)
				{
					int indexV2 = face.m_pVertexIndices[triangleVert - 1];
					int indexV3 = face.m_pVertexIndices[triangleVert];
					if (g_objLoaderShouldGenerateExtraVertices == false) {
						vertexIndexSet vis(indexV2);
						if (numNormals > 0) vis.nIndex = face.m_pNormalIndices[triangleVert - 1];
						if (numTexCoord > 0) vis.tIndex = face.m_pTextureIndices[triangleVert - 1];
						indexV2 = getVertexIndex(curMesh, &fileObj, curVertexMap, vis);
						vis.vIndex = indexV3;
						if (numNormals > 0) vis.nIndex = face.m_pNormalIndices[triangleVert];
						if (numTexCoord > 0) vis.tIndex = face.m_pTextureIndices[triangleVert];
						indexV3 = getVertexIndex(curMesh, &fileObj, curVertexMap, vis);
					}

					// For debugging, I want to look for degenerate triangles, but
					// I don't want to assert here.
					if (indexV1 == indexV2 || indexV2 == indexV3 || indexV1 == indexV3) {

					}

					unsigned int indexTriangle;

					// create triangle:
					if (g_objLoaderShouldGenerateExtraVertices == false) {
						indexTriangle =
							curMesh->newTriangle(indexV1, indexV2, indexV3);
					}
					else {
						indexTriangle =
							curMesh->newTriangle(
								fileObj.m_pVertices[indexV1],
								fileObj.m_pVertices[indexV2],
								fileObj.m_pVertices[indexV3]
							);
					}

					cTriangle* curTriangle = curMesh->getTriangle(indexTriangle, false);

					// assign normals:
					if (numNormals > 0)
					{
						// set normals
						curTriangle->getVertex0()->setNormal(face.m_pNormals[0]);
						curTriangle->getVertex1()->setNormal(face.m_pNormals[triangleVert - 1]);
						curTriangle->getVertex2()->setNormal(face.m_pNormals[triangleVert]);
					}

					// assign texture coordinates
					if (numTexCoord > 0)
					{
						// set texture coordinates
						curTriangle->getVertex0()->setTexCoord(face.m_pTexCoords[0]);
						curTriangle->getVertex1()->setTexCoord(face.m_pTexCoords[triangleVert - 1]);
						curTriangle->getVertex2()->setTexCoord(face.m_pTexCoords[triangleVert]);
					}
				}
			}
			else {

				// This faces doesn't have 3 vertices... this line is just
				// here for debugging, since this should never happen, but
				// I don't want to assert here.

			}
			j++;
		}
		i++;
	}

	delete[] vertexMaps;

	// if no normals were specified in the file, compute them
	// based on triangle faces
	if (numNormals == 0) {
		a_mesh->computeAllNormals(true);
	}

	// compute boundary boxes
	a_mesh->computeBoundaryBox(true);

	// update global position in world
	if (world != 0) world->computeGlobalPositions(true);

	// return success
	return (true);
}

//--------------------------------------------------------------------
// OBJ PARSER IMPLEMENTATION:
//--------------------------------------------------------------------

cOBJModel::cOBJModel()
{
	m_pVertices = NULL;
	m_pFaces = NULL;
	m_pNormals = NULL;
	m_pTexCoords = NULL;
	m_pMaterials = NULL;
}

cOBJModel::~cOBJModel()
{
	if (m_pVertices) delete[] m_pVertices;
	if (m_pNormals) delete[] m_pNormals;
	if (m_pTexCoords) delete[] m_pTexCoords;
	if (m_pMaterials) delete[] m_pMaterials;
	if (m_pFaces)
	{
		for (unsigned int i = 0; i < m_OBJInfo.m_faceCount; i++)
		{
			// Delete every pointer in the face structure
			if (m_pFaces[i].m_pNormals) delete[] m_pFaces[i].m_pNormals;
			if (m_pFaces[i].m_pNormalIndices) delete[] m_pFaces[i].m_pNormalIndices;
			if (m_pFaces[i].m_pTexCoords)  delete[] m_pFaces[i].m_pTexCoords;
			if (m_pFaces[i].m_pTextureIndices) delete[] m_pFaces[i].m_pTextureIndices;
			if (m_pFaces[i].m_pVertices)  delete[] m_pFaces[i].m_pVertices;
			if (m_pFaces[i].m_pVertexIndices)  delete[] m_pFaces[i].m_pVertexIndices;
		}
		delete[] m_pFaces;
	}
	for (unsigned int i = 0; i < m_groupNames.size(); i++) {
		delete[] m_groupNames[i];
	}
}

bool cOBJModel::LoadModel(const char a_fileName[])
{
	//----------------------------------------------------------------------
	// Load a OBJ file and render its data into a display list
	//----------------------------------------------------------------------

	cOBJFileInfo currentIndex;    // Current array index
	char str[CHAI_OBJ_MAX_STR_SIZE];    // Buffer string for reading the file
	char basePath[_MAX_PATH];   // Path were all paths in the OBJ start
	int nScanReturn = 0;      // Return value of fscanf
	unsigned int curMaterial = 0; // Current material

	// Get base path
	strcpy(basePath, a_fileName);
	makePath(basePath);

	//----------------------------------------------------------------------
	// Open the OBJ file
	//----------------------------------------------------------------------
	FILE *hFile = fopen(a_fileName, "r");

	// Success opening file?
	if (!hFile)
	{
		return (false);
	}

	//----------------------------------------------------------------------
	// Allocate space for structures that hold the model data
	//----------------------------------------------------------------------

	// Which data types are stored in the file ? How many of each type ?
	getFileInfo(hFile, &m_OBJInfo, basePath);

	// Vertices and faces
	if (m_pVertices) delete[] m_pVertices;
	if (m_pFaces) delete[] m_pFaces;
	m_pVertices = new cVector3d[m_OBJInfo.m_vertexCount];
	m_pFaces = new cFace[m_OBJInfo.m_faceCount];

	// Allocate space for optional model data only if present.
	if (m_pNormals) { delete[] m_pNormals; m_pNormals = NULL; }
	if (m_pTexCoords) { delete[] m_pTexCoords; m_pTexCoords = NULL; }
	if (m_pMaterials) { delete[] m_pMaterials; m_pMaterials = NULL; }
	if (m_OBJInfo.m_normalCount)
		m_pNormals = new cVector3d[m_OBJInfo.m_normalCount];
	if (m_OBJInfo.m_texCoordCount)
		m_pTexCoords = new cVector3d[m_OBJInfo.m_texCoordCount];
	if (m_OBJInfo.m_materialCount)
		m_pMaterials = new cMaterialInfo[m_OBJInfo.m_materialCount];

	// Init structure that holds the current array index
	memset(&currentIndex, 0, sizeof(cOBJFileInfo));

	//----------------------------------------------------------------------
	// Read the file contents
	//----------------------------------------------------------------------

	// Start reading the file from the start
	rewind(hFile);

	// Quit reading when end of file has been reached
	while (!feof(hFile))
	{
		// Get next string
		readNextString(str, hFile);

		// Next three elements are floats of a vertex
		if (!strncmp(str, CHAI_OBJ_VERTEX_ID, sizeof(CHAI_OBJ_VERTEX_ID)))
		{
			// Read three floats out of the file
			float fx, fy, fz;
			nScanReturn = fscanf(hFile, "%f %f %f", &fx, &fy, &fz);

			m_pVertices[currentIndex.m_vertexCount].x = fx;
			m_pVertices[currentIndex.m_vertexCount].y = fy;
			m_pVertices[currentIndex.m_vertexCount].z = fz;

			// Next vertex
			currentIndex.m_vertexCount++;
		}

		// Next two elements are floats of a texture coordinate
		if (!strncmp(str, CHAI_OBJ_TEXCOORD_ID, sizeof(CHAI_OBJ_TEXCOORD_ID)))
		{
			// Read two floats out of the file
			float fx, fy, fz;
			nScanReturn = fscanf(hFile, "%f %f %f", &fx, &fy, &fz);

			m_pTexCoords[currentIndex.m_texCoordCount].x = fx;
			m_pTexCoords[currentIndex.m_texCoordCount].y = fy;
			m_pTexCoords[currentIndex.m_texCoordCount].z = fz;

			// Next texture coordinate
			currentIndex.m_texCoordCount++;
		}

		// Next three elements are floats of a vertex normal
		if (!strncmp(str, CHAI_OBJ_NORMAL_ID, sizeof(CHAI_OBJ_NORMAL_ID)))
		{
			// Read three floats out of the file
			float fx, fy, fz;
			nScanReturn = fscanf(hFile, "%f %f %f", &fx, &fy, &fz);

			m_pNormals[currentIndex.m_normalCount].x = fx;
			m_pNormals[currentIndex.m_normalCount].y = fy;
			m_pNormals[currentIndex.m_normalCount].z = fz;

			// Next normal
			currentIndex.m_normalCount++;
		}

		// Rest of the line contains face information
		if (!strncmp(str, CHAI_OBJ_FACE_ID, sizeof(CHAI_OBJ_FACE_ID)))
		{
			// Read the rest of the line (the complete face)
			getTokenParameter(str, sizeof(str), hFile);
			// Convert string into a face structure
			parseFaceString(str, &m_pFaces[currentIndex.m_faceCount],
				m_pVertices, m_pNormals, m_pTexCoords, curMaterial);
			// Next face
			currentIndex.m_faceCount++;
		}

		// Rest of the line contains face information
		if (!strncmp(str, CHAI_OBJ_NAME_ID, sizeof(CHAI_OBJ_NAME_ID)))
		{
			// Read the rest of the line (the complete face)
			getTokenParameter(str, sizeof(str), hFile);

			char* name = new char[strlen(str) + 1];
			strcpy(name, str);
			m_groupNames.push_back(name);

		}

		// Process material information only if needed
		if (m_pMaterials)
		{
			// Rest of the line contains the name of a material
			if (!strncmp(str, CHAI_OBJ_USE_MTL_ID, sizeof(CHAI_OBJ_USE_MTL_ID)))
			{
				// Read the rest of the line (the complete material name)
				getTokenParameter(str, sizeof(str), hFile);
				// Are any materials loaded ?
				if (m_pMaterials)
					// Find material array index for the material name
					for (unsigned i = 0; i < m_OBJInfo.m_materialCount; i++)
						if (!strncmp(m_pMaterials[i].m_name, str, sizeof(str)))
						{
							curMaterial = i;
							break;
						}
			}

			// Rest of the line contains the filename of a material library
			if (!strncmp(str, CHAI_OBJ_MTL_LIB_ID, sizeof(CHAI_OBJ_MTL_LIB_ID)))
			{
				// Read the rest of the line (the complete filename)
				getTokenParameter(str, sizeof(str), hFile);
				// Append material library filename to the model's base path
				char libraryFile[_MAX_PATH];
				strcpy(libraryFile, basePath);
				strcat(libraryFile, str);

				// Append .mtl
				//strcat(szLibraryFile, ".mtl");

				// Load the material library
				loadMaterialLib(libraryFile, m_pMaterials,
					&currentIndex.m_materialCount, basePath);
			}
		}
	}

	// Close OBJ file
	fclose(hFile);

	//----------------------------------------------------------------------
	// Success
	//----------------------------------------------------------------------

	return (true);
}

void cOBJModel::parseFaceString(char a_faceString[], cFace *a_faceOut,
	const cVector3d *a_pVertices,
	const cVector3d *a_pNormals,
	const cVector3d *a_pTexCoords,
	const unsigned int a_materialIndex)
{
	//----------------------------------------------------------------------
	// Convert face string from the OBJ file into a face structure
	//----------------------------------------------------------------------

	unsigned int i;
	int iVertex = 0, iTextureCoord = 0, iNormal = 0;

	// Pointer to the face string. Will be incremented later to
	// advance to the next triplet in the string.
	char *pFaceString = a_faceString;

	// Save the string positions of all triplets
	int iTripletPos[CHAI_OBJ_MAX_VERTICES];
	int iCurTriplet = 0;

	// Init the face structure
	memset(a_faceOut, 0, sizeof(cFace));

	// The first vertex always starts at position 0 in the string
	iTripletPos[0] = 0;
	a_faceOut->m_numVertices = 1;
	iCurTriplet++;

	if (m_groupNames.size() > 0) a_faceOut->m_groupIndex = m_groupNames.size() - 1;
	else a_faceOut->m_groupIndex = -1;

	//----------------------------------------------------------------------
	// Get number of vertices in the face
	//----------------------------------------------------------------------

	// Loop trough the whole string
	for (i = 0; i < strlen(a_faceString); i++)
	{
		// Each triplet is separated by spaces
		if (a_faceString[i] == ' ')
		{
			// One more vertex
			a_faceOut->m_numVertices++;
			// Save position of triplet
			iTripletPos[iCurTriplet] = i;
			// Next triplet
			iCurTriplet++;
		}
	}

	// Face has more vertices than spaces that separate them
	// FaceOut->iNumVertices++;

	//----------------------------------------------------------------------
	// Allocate space for structures that hold the face data
	//----------------------------------------------------------------------

	// Vertices
	a_faceOut->m_pVertices = new cVector3d[a_faceOut->m_numVertices];
	a_faceOut->m_pVertexIndices = new int[a_faceOut->m_numVertices];

	// Allocate space for normals and texture coordinates only if present
	if (m_pNormals) {
		a_faceOut->m_pNormals = new cVector3d[a_faceOut->m_numVertices];
		a_faceOut->m_pNormalIndices = new int[a_faceOut->m_numVertices];
	}
	else {
		a_faceOut->m_pNormals = 0;
		a_faceOut->m_pNormalIndices = 0;
	}

	if (m_pTexCoords) {
		a_faceOut->m_pTexCoords = new cVector3d[a_faceOut->m_numVertices];
		a_faceOut->m_pTextureIndices = new int[a_faceOut->m_numVertices];
	}
	else {
		a_faceOut->m_pTexCoords = 0;
		a_faceOut->m_pTextureIndices = 0;
	}

	//----------------------------------------------------------------------
	// Copy vertex, normal, material and texture data into the structure
	//----------------------------------------------------------------------

	// Set material
	a_faceOut->m_materialIndex = a_materialIndex;

	// Process per-vertex data
	for (i = 0; i < (unsigned int)a_faceOut->m_numVertices; i++)
	{
		// Read one triplet from the face string

		// Are vertices, normals and texture coordinates present ?
		if (m_pNormals && m_pTexCoords)
			// Yes
			sscanf(pFaceString, "%i/%i/%i",
				&iVertex, &iTextureCoord, &iNormal);
		else if (m_pNormals && !m_pTexCoords)
			// Vertices and normals but no texture coordinates
			sscanf(pFaceString, "%i//%i", &iVertex, &iNormal);
		else if (m_pTexCoords && !m_pNormals)
			// Vertices and texture coordinates but no normals
			sscanf(pFaceString, "%i/%i", &iVertex, &iTextureCoord);
		else
			// Only vertices
			sscanf(pFaceString, "%i", &iVertex);

		// Copy vertex into the face. Also check for normals and texture
		// coordinates and copy them if present.
		memcpy(&a_faceOut->m_pVertices[i], &m_pVertices[iVertex - 1],
			sizeof(cVector3d));
		a_faceOut->m_pVertexIndices[i] = iVertex - 1;

		if (m_pTexCoords) {
			memcpy(&a_faceOut->m_pTexCoords[i],
				&m_pTexCoords[iTextureCoord - 1], sizeof(cVector3d));
			a_faceOut->m_pTextureIndices[i] = iTextureCoord - 1;
		}
		if (m_pNormals) {
			memcpy(&a_faceOut->m_pNormals[i],
				&m_pNormals[iNormal - 1], sizeof(cVector3d));
			a_faceOut->m_pNormals[i].normalize();
			a_faceOut->m_pNormalIndices[i] = iNormal - 1;
		}

		// Set string pointer to the next triplet
		pFaceString = &a_faceString[iTripletPos[i + 1]];
	}
}

bool cOBJModel::loadMaterialLib(const char a_fileName[],
	cMaterialInfo* a_pMaterials,
	unsigned int* a_curMaterialIndex,
	char a_basePath[])
{
	//----------------------------------------------------------------------
	// Loads a material library file (.mtl)
	//----------------------------------------------------------------------

	char str[CHAI_OBJ_MAX_STR_SIZE];  // Buffer used while reading the file
	bool bFirstMaterial = true;   // Only increase index after first
					// material has been set

	  //----------------------------------------------------------------------
	// Open library file
	  //----------------------------------------------------------------------

	FILE *hFile = fopen(a_fileName, "r");

	// Success ?
	if (!hFile)
	{
		return (false);
	}

	//----------------------------------------------------------------------
	// Read all material definitions
	//----------------------------------------------------------------------

	// Quit reading when end of file has been reached
	while (!feof(hFile))
	{
		// Get next string
		readNextString(str, hFile);

		// Is it a "new material" identifier ?
		if (!strncmp(str, CHAI_OBJ_NEW_MTL_ID, sizeof(CHAI_OBJ_NEW_MTL_ID)))
		{
			// Only increase index after first material has been set
			if (bFirstMaterial == true)
				// First material has been set
				bFirstMaterial = false;
			else
				// Use next index
				(*a_curMaterialIndex)++;
			// Read material name
			getTokenParameter(str, sizeof(str), hFile);
			// Store material name in the structure
			strcpy(m_pMaterials[*a_curMaterialIndex].m_name, str);
		}

		// Transparency
		if (
			(!strncmp(str, CHAI_OBJ_MTL_ALPHA_ID, sizeof(CHAI_OBJ_MTL_ALPHA_ID)))
			||
			(!strncmp(str, CHAI_OBJ_MTL_ALPHA_ID_ALT, sizeof(CHAI_OBJ_MTL_ALPHA_ID_ALT)))
			)
		{
			// Read into current material
			fscanf(hFile, "%f", &m_pMaterials[*a_curMaterialIndex].m_alpha);
		}

		// Ambient material properties
		if (!strncmp(str, CHAI_OBJ_MTL_AMBIENT_ID, sizeof(CHAI_OBJ_MTL_AMBIENT_ID)))
		{
			// Read into current material
			fscanf(hFile, "%f %f %f",
				&m_pMaterials[*a_curMaterialIndex].m_ambient[0],
				&m_pMaterials[*a_curMaterialIndex].m_ambient[1],
				&m_pMaterials[*a_curMaterialIndex].m_ambient[2]);
		}

		// Diffuse material properties
		if (!strncmp(str, CHAI_OBJ_MTL_DIFFUSE_ID, sizeof(CHAI_OBJ_MTL_DIFFUSE_ID)))
		{
			// Read into current material
			fscanf(hFile, "%f %f %f",
				&m_pMaterials[*a_curMaterialIndex].m_diffuse[0],
				&m_pMaterials[*a_curMaterialIndex].m_diffuse[1],
				&m_pMaterials[*a_curMaterialIndex].m_diffuse[2]);
		}

		// Specular material properties
		if (!strncmp(str, CHAI_OBJ_MTL_SPECULAR_ID, sizeof(CHAI_OBJ_MTL_SPECULAR_ID)))
		{
			// Read into current material
			fscanf(hFile, "%f %f %f",
				&m_pMaterials[*a_curMaterialIndex].m_specular[0],
				&m_pMaterials[*a_curMaterialIndex].m_specular[1],
				&m_pMaterials[*a_curMaterialIndex].m_specular[2]);
		}

		// Texture map name
		if (!strncmp(str, CHAI_OBJ_MTL_TEXTURE_ID, sizeof(CHAI_OBJ_MTL_TEXTURE_ID)))
		{
			// Read texture filename
			getTokenParameter(str, sizeof(str), hFile);
			// Append material library filename to the model's base path
			char textureFile[_MAX_PATH];
			strcpy(textureFile, a_basePath);
			strcat(textureFile, str);
			// Store texture filename in the structure
			strcpy(m_pMaterials[*a_curMaterialIndex].m_texture, textureFile);
			// Load texture and store its ID in the structure
			m_pMaterials[*a_curMaterialIndex].m_textureID = 1;//LoadTexture(szTextureFile);
		}

		// Shininess
		if (!strncmp(str, CHAI_OBJ_MTL_SHININESS_ID, sizeof(CHAI_OBJ_MTL_SHININESS_ID)))
		{
			// Read into current material
			fscanf(hFile, "%f",
				&m_pMaterials[*a_curMaterialIndex].m_shininess);

			// OBJ files use a shininess from 0 to 1000; Scale for OpenGL
			m_pMaterials[*a_curMaterialIndex].m_shininess /= 1000.0f;
			m_pMaterials[*a_curMaterialIndex].m_shininess *= 128.0f;
		}
	}

	fclose(hFile);

	// Increment index cause LoadMaterialLib() assumes that the passed
	// index is always empty
	(*a_curMaterialIndex)++;

	return (true);
}

void cOBJModel::getFileInfo(FILE *a_hStream, cOBJFileInfo *a_info,
	const char a_constBasePath[])
{
	//----------------------------------------------------------------------
	// Read the count of all important identifiers out of the given stream
	//----------------------------------------------------------------------

	char str[CHAI_OBJ_MAX_STR_SIZE]; // Buffer for reading the file
	char basePath[_MAX_PATH];  // Needed to append a filename to the base path

	// Init structure
	memset(a_info, 0, sizeof(cOBJFileInfo));

	// Rollback the stream
	rewind(a_hStream);

	// Quit reading when end of file has been reached
	while (!feof(a_hStream))
	{
		// Get next string
		readNextString(str, a_hStream);

		// Vertex ?
		if (!strncmp(str, CHAI_OBJ_VERTEX_ID, sizeof(CHAI_OBJ_VERTEX_ID)))
			a_info->m_vertexCount++;
		// Texture coordinate ?
		if (!strncmp(str, CHAI_OBJ_TEXCOORD_ID, sizeof(CHAI_OBJ_TEXCOORD_ID)))
			a_info->m_texCoordCount++;
		// Vertex normal ?
		if (!strncmp(str, CHAI_OBJ_NORMAL_ID, sizeof(CHAI_OBJ_NORMAL_ID)))
			a_info->m_normalCount++;
		// Face ?
		if (!strncmp(str, CHAI_OBJ_FACE_ID, sizeof(CHAI_OBJ_FACE_ID)))
			a_info->m_faceCount++;

		// Material library definition ?
		if (!strncmp(str, CHAI_OBJ_MTL_LIB_ID, sizeof(CHAI_OBJ_MTL_LIB_ID)))
		{
			// Read the filename of the library
			getTokenParameter(str, sizeof(str), a_hStream);
			// Copy the model's base path into a none-constant string
			strcpy(basePath, a_constBasePath);
			// Append material library filename to the model's base path
			strcat(basePath, str);
			// Append .mtl
			//strcat(szBasePath, ".mtl");
			// Open the library file
			FILE *hMaterialLib = fopen(basePath, "r");
			// Success ?
			if (hMaterialLib)
			{
				// Quit reading when end of file has been reached
				while (!feof(hMaterialLib))
				{
					// Read next string
					fscanf(hMaterialLib, "%s", str);
					// Is it a "new material" identifier ?
					if (!strncmp(str, CHAI_OBJ_NEW_MTL_ID, sizeof(CHAI_OBJ_NEW_MTL_ID)))
						// One more material defined
						a_info->m_materialCount++;
				}
				// Close material library
				fclose(hMaterialLib);
			}
		}

		// Clear string two avoid counting something twice
		memset(str, '\0', sizeof(str));
	}
}

void cOBJModel::readNextString(char a_str[], FILE *a_hStream)
{
	//----------------------------------------------------------------------
	// Read the next string that isn't a comment
	//----------------------------------------------------------------------

	bool bSkipLine = false; // Skip the current line ?
	int nScanReturn = 0;  // Return value of fscanf

	// Skip all strings that contain comments
	do
	{
		// Read new string
		nScanReturn = fscanf(a_hStream, "%s", a_str);
		// Is rest of the line a comment ?
		if (!strncmp(a_str, CHAI_OBJ_COMMENT_ID, sizeof(CHAI_OBJ_COMMENT_ID)))
		{
			// Skip the rest of the line
			fgets(a_str, sizeof(a_str), a_hStream);
			bSkipLine = true;
		}
		else
		{
			bSkipLine = false;
		}
	} while (bSkipLine == true);
}

void cOBJModel::makePath(char a_fileAndPath[])
{
	//----------------------------------------------------------------------
	// Rips the filenames out of a path and adds a slash (if needed)
	//----------------------------------------------------------------------

	// Get string length
	int iNumChars = strlen(a_fileAndPath);

	// Loop from the last to the first char
	for (int iCurChar = iNumChars - 1; iCurChar >= 0; iCurChar--)
	{
		// If the current char is a slash / backslash
		if (a_fileAndPath[iCurChar] == char('\\') ||
			a_fileAndPath[iCurChar] == char('/'))
		{
			// Terminate the the string behind the slash / backslash
			a_fileAndPath[iCurChar + 1] = char('\0');
			return;
		}
	}

	// No slash there, set string length to zero
	a_fileAndPath[0] = char('\0');
}

void cOBJModel::getTokenParameter(char a_str[],
	const unsigned int a_strSize, FILE *a_hFile)
{
	//----------------------------------------------------------------------
	// Read the parameter of a token, remove space and newline character
	//----------------------------------------------------------------------

	// Read the parameter after the token
	fgets(a_str, a_strSize, a_hFile);

	char* first_non_whitespace_character = a_str;
	while (*first_non_whitespace_character == ' ') first_non_whitespace_character++;

	// Remove space before the token
	strcpy(a_str, first_non_whitespace_character);

	// Remove newline character after the token
	if (a_str[strlen(a_str) - 1] == '\r' || a_str[strlen(a_str) - 1] == '\n')
		a_str[strlen(a_str) - 1] = '\0';
	if (a_str[strlen(a_str) - 1] == '\r' || a_str[strlen(a_str) - 1] == '\n')
		a_str[strlen(a_str) - 1] = '\0';
}



unsigned int getVertexIndex(cMesh* a_mesh, cOBJModel* a_model,
	vertexIndexSet_uint_map* a_vertexMap, vertexIndexSet& vis) {

	unsigned int index;

	// Have we seen this vertex before?
	vertexIndexSet_uint_map::iterator vertexMapIter = a_vertexMap->find(vis);

	// If we have, just grab the new index for this vertex
	if (vertexMapIter != a_vertexMap->end()) {
		index = (*vertexMapIter).second;
		return index;
	}

	// Otherwise create a new vertex and put the mapping in our map
	else {
		index = a_mesh->newVertex(a_model->m_pVertices[vis.vIndex]);
		(*a_vertexMap)[vis] = index;
		return index;
	}

}
