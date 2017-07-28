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
#ifndef CFileLoaderOBJH
#define CFileLoaderOBJH
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
#include <stdio.h>

// Clients can use this to tell the obj loader how to behave in terms
// of vertex merging.
//
// If 'true' (default), loaded obj files will have three _distinct_ vertices
// per triangle, with no vertex re-use.
extern bool g_objLoaderShouldGenerateExtraVertices;


// A face vertex, as defined in an .obj file (a vertex/normal/texture set)
struct vertexIndexSet {
  int vIndex;
  int nIndex;
  int tIndex;
  vertexIndexSet() {
    vIndex = nIndex = tIndex = 0;
  }
  vertexIndexSet(int vIndex, int nIndex, int tIndex) {
    this->vIndex = vIndex;
    this->nIndex = nIndex;
    this->tIndex = tIndex;
  }
  vertexIndexSet(int vIndex) {
    this->vIndex = vIndex;
    nIndex = tIndex = 0;
  }
};

struct ltVertexIndexSet
{
  bool operator()(vertexIndexSet v1, vertexIndexSet v2) const
  {
    if (v1.vIndex < v2.vIndex) return 1;
    if (v2.vIndex < v1.vIndex) return 0;
    if (v1.nIndex < v2.nIndex) return 1;
    if (v2.nIndex < v1.nIndex) return 0;
    if (v1.tIndex < v2.tIndex) return 1;
    return 0;
  }
};

#include <map>
typedef std::map<vertexIndexSet,unsigned int,ltVertexIndexSet> vertexIndexSet_uint_map;

//---------------------------------------------------------------------------

//===========================================================================
/*!
      \file     CFileLoaderOBJ.h
      \brief    The following file provides a parser to load 3d images
                supporting the alias/wavefront file format.
*/
//===========================================================================

//! Load a 3d image by providing a filename and mesh in which object is loaded.
bool cLoadFileOBJ(cMesh* iMesh, const string& iFileName);

//===========================================================================
//  The following code is only used by the parser.
//===========================================================================

// OBJ File string identifiers
#define CHAI_OBJ_VERTEX_ID    "v"
#define CHAI_OBJ_TEXCOORD_ID  "vt"
#define CHAI_OBJ_NORMAL_ID    "vn"
#define CHAI_OBJ_FACE_ID      "f"
#define CHAI_OBJ_COMMENT_ID   "#"
#define CHAI_OBJ_MTL_LIB_ID   "mtllib"
#define CHAI_OBJ_USE_MTL_ID   "usemtl"
#define CHAI_OBJ_NAME_ID      "g"
// MTL File string identifiers
#define CHAI_OBJ_NEW_MTL_ID       "newmtl"
#define CHAI_OBJ_MTL_TEXTURE_ID   "map_Kd"
#define CHAI_OBJ_MTL_AMBIENT_ID	  "Ka"
#define CHAI_OBJ_MTL_DIFFUSE_ID	  "Kd"
#define CHAI_OBJ_MTL_SPECULAR_ID  "Ks"
#define CHAI_OBJ_MTL_SHININESS_ID "Ns"
#define CHAI_OBJ_MTL_ALPHA_ID     "Tr"
#define CHAI_OBJ_MTL_ALPHA_ID_ALT "d"

// Maximum size of a string that could be read out of the OBJ file
#define CHAI_OBJ_MAX_STR_SIZE 1024

// Maximum number of vertices a that a single face can have
#define CHAI_OBJ_MAX_VERTICES 256

// Image File information.
struct cOBJFileInfo
{
	unsigned int m_vertexCount;
	unsigned int m_texCoordCount;
	unsigned int m_normalCount;
	unsigned int m_faceCount;
	unsigned int m_materialCount;
};

// Information about a surface face.
struct cFace
{
	unsigned int	m_numVertices;
	unsigned int	m_materialIndex;

  // Which 'g ...' group does this face belong to?  -1 indicates no group.
  int  m_groupIndex;

	int			*m_pVertexIndices;
	cVector3d	*m_pVertices;
  int			*m_pNormalIndices;
	cVector3d	*m_pNormals;
  int			*m_pTextureIndices;
	cVector3d	*m_pTexCoords;
};

// Information about a material property
struct cMaterialInfo
{
    char m_name[1024];
    char m_texture[_MAX_PATH];
    int	m_textureID;
    float m_diffuse[3];
    float m_ambient[3];
    float m_specular[3];
    float m_emmissive[3];
    float m_alpha;
    float m_shininess;

    cMaterialInfo() {
      m_name[0] = '\0';
      m_texture[0] = '\0';
      m_textureID = -1;
      m_diffuse[0] = m_diffuse[1] = m_diffuse[2] = 0.8f;
      m_ambient[0] = m_ambient[1] = m_ambient[2] = 0.8f;
      m_specular[0] = m_specular[1] = m_specular[2] = 0.3f;
      m_emmissive[0] = m_emmissive[1] = m_emmissive[2] = 0.0f;
      m_shininess = 0;
      m_alpha = 1.0f;
    }
};

//    Main class for OBJ parser.


class cOBJModel
{
  public:
    // CONSTRUCTOR & DESTRUCTOR
    // constructor
    cOBJModel();
    // destructor
    ~cOBJModel();

    // METHODS:
    // Load model file.
    bool LoadModel(const char szFileName[]);

    // MEMBERS:
    // List of vertices.
  	cVector3d* m_pVertices;
    // List of faces.
	cFace* m_pFaces;
    // List of normals
	cVector3d* m_pNormals;
    // List of texture coordinates
	cVector3d* m_pTexCoords;
    // List of material and texture properties
	cMaterialInfo* m_pMaterials;
    // Information about image file.
	cOBJFileInfo m_OBJInfo;

  // List of names obtained from 'g' commands, with the most
  // recent at the back...
  vector<char*> m_groupNames;

  private:
    //METHODS:
    // Read next string of file.
    void  readNextString(char a_string[], FILE *hStream);
    // Get next token from file.
    void  getTokenParameter(char a_string[], const unsigned int a_strSize, FILE *a_hFile);
    // File path
    void  makePath(char a_fileAndPath[]);
    // Load material file [mtl]
    bool  loadMaterialLib(const char a_fFileName[], cMaterialInfo *a_pMaterials,
          unsigned int *a_curMaterialIndex, char a_basePath[]);
    // Parse information about face.
    void  parseFaceString(char a_faceString[], cFace *a_faceOut, const cVector3d *a_pVertices,
        const cVector3d *a_pNormals, const cVector3d *a_pTexCoords, const unsigned int a_materialIndex);
    // Read information about file.
    void  getFileInfo(FILE *a_hStream, cOBJFileInfo *a_stat, const char a_constBasePath[]);    
};

// Internal: get a (possibly new) vertex index for a vertex
unsigned int getVertexIndex(cMesh* a_Mesh, cOBJModel* a_model, 
                            vertexIndexSet_uint_map* a_VertexMap, vertexIndexSet& vis);

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------


