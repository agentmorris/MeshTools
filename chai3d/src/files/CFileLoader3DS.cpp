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

	The bulk of this file comes from Lev Povalahev's l3ds.cpp,
	copyright (c) 2001-2002 Lev Povalahev.  Used with permission.

	\author:    <http://www.chai3d.org>
	\author:    Lev Povalahev
	\author:    Dan Morris
	\version    1.0
	\date       03/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CFileLoader3DS.h"

// We want to get around snprintf in posix environments
#ifdef _POSIX
#define _snprintf(x,y,...) sprintf(x,__VA_ARGS__) 
#endif

#include <map>

typedef std::map<unsigned int, unsigned int> uint_uint_map;

bool g_3dsLoaderShouldGenerateExtraVertices = false;

//---------------------------------------------------------------------------

//===========================================================================
/*!
	Load a 3d studio max 3ds file format image into a mesh.

	\fn         bool cLoadFile3DS(cMesh* a_mesh, const string& a_fileName)
	\param      a_mesh         Mesh in which image file is loaded
	\param      a_fileName     Name of image file.
	\return     Return \b true if image was loaded successfully, otherwise
				return \b false.
*/
//===========================================================================
bool cLoadFile3DS(cMesh* a_mesh, const string& a_fileName)
{
	int k;

	// Instantiate a loader
	L3DS loader;

	const char* fname = a_fileName.c_str();
	if (loader.LoadFile(fname) == false) return false;

	cWorld* world = a_mesh->getParentWorld();

	// clear all vertices and triangles of the current mesh
	a_mesh->clear();

	// Global rendering flags that we'll use to mark the root mesh usefully
	bool transparency_enabled = false;
	bool materials_enabled = false;

	// get information about file

	// The number of meshes loaded from this file
	int num_file_meshes = loader.GetMeshCount();

	// Create a child mesh for each mesh we found in the file,
	// and load its information
	for (int current_file_mesh = 0; current_file_mesh < num_file_meshes; current_file_mesh++) {

		LMesh& cur_mesh = loader.GetMesh(current_file_mesh);

		// This CHAI mesh corresponds to the 3ds submesh
		cMesh* sub_mesh = a_mesh->createMesh();

		// Assign a name to this mesh
		strncpy(sub_mesh->m_objectName, (const char*)(cur_mesh.GetName().c_str()), CHAI_MAX_OBJECT_NAME_LENGTH);

		// Create a vertex map for each submesh
		sub_mesh->m_userData = new uint_uint_map;

		a_mesh->addChild(sub_mesh);

		// For each mesh in the file, we're going to create an additional mesh for
		// each material, since in CHAI each mesh has a specific material
		int num_materials = cur_mesh.GetMaterialCount();

		// We'll put material-specific submeshes here if we find them...
		cMesh** newMeshes = 0;

		// Process materials if there are any...
		if (num_materials > 0) {

			materials_enabled = true;

			newMeshes = new cMesh*[num_materials];

			// Create the new meshes and add them to the submesh we're working on now
			for (int i = 0; i < num_materials; i++) {

				newMeshes[i] = a_mesh->createMesh();
				cMesh* newMesh = newMeshes[i];

				// Assign a name to this mesh
				if (num_materials > 1)
					_snprintf(newMesh->m_objectName, CHAI_MAX_OBJECT_NAME_LENGTH, "%s (mat %d)",
					(const char*)(cur_mesh.GetName().c_str()), i);
				else
					_snprintf(newMesh->m_objectName, CHAI_MAX_OBJECT_NAME_LENGTH, "%s",
					(const char*)(cur_mesh.GetName().c_str()));

				newMesh->m_userData = new uint_uint_map;
				sub_mesh->addChild(newMesh);

				// Set up material properties for each mesh

				// Get the global material id for each material used in this mesh
				int global_material_id = cur_mesh.GetMaterial(i);

				LMaterial& cur_material = loader.GetMaterial(global_material_id);

				// Does this material have a texture?  Note that we only
				// support one texture per material right now.
				LMap& curmap = cur_material.GetTextureMap1();

				if (strlen(curmap.mapName) > 0) {

					cTexture2D *newTexture = world->newTexture();
					int result = newTexture->loadFromFile(curmap.mapName);

					// If this didn't work out, try again in the 3ds file's path
					if (result == 0) {

						char model_dir[1024];
						find_directory(model_dir, fname);

						char new_texture_path[1024];
						sprintf(new_texture_path, "%s/%s", model_dir, curmap.mapName);

						result = newTexture->loadFromFile(new_texture_path);

					}

					if (result) {
						newMeshes[i]->setTexture(newTexture, 1);
					}

					// We really failed to load a texture...
					else {
#ifdef _WIN32
						// CHAI_DEBUG_PRINT("Could not load texture map %s\n",curmap.mapName);
#endif
					}

				}

				// Get transparency
				double transparency = cur_material.GetTransparency();
				float alpha = 1.0f - (float)(transparency);
				if (alpha < 1.0) {
					transparency_enabled = true;
					newMesh->enableTransparency(true, false);
				}

				float shininess = cur_material.GetShininess();
				newMesh->m_material.setShininess(int(shininess * 255.0));

				// get ambient component:
				LColor3 ambient = cur_material.GetAmbientColor();
				newMesh->m_material.m_ambient.setR((float)ambient.r);
				newMesh->m_material.m_ambient.setG((float)ambient.g);
				newMesh->m_material.m_ambient.setB((float)ambient.b);
				newMesh->m_material.m_ambient.setA(alpha);

				// get diffuse component:
				LColor3 diffuse = cur_material.GetDiffuseColor();
				newMesh->m_material.m_diffuse.setR((float)diffuse.r);
				newMesh->m_material.m_diffuse.setG((float)diffuse.g);
				newMesh->m_material.m_diffuse.setB((float)diffuse.b);
				newMesh->m_material.m_diffuse.setA(alpha);

				// get specular component:
				LColor3 specular = cur_material.GetSpecularColor();
				newMesh->m_material.m_specular.setR((float)specular.r);
				newMesh->m_material.m_specular.setG((float)specular.g);
				newMesh->m_material.m_specular.setB((float)specular.b);
				newMesh->m_material.m_specular.setA(alpha);

				// get shininess
				newMesh->m_material.setShininess((GLuint)(128.0 * cur_material.GetShininess()));

			} // For every material in this mesh

		} // If this submesh has materials

		// Local copy of the material list, for quick lookup
		vector<uint> materials = cur_mesh.m_materials;

		// Now loop over all the triangles in this mesh and put them in the right
		// place in our CHAI meshes...
		unsigned int num_triangles = cur_mesh.GetTriangleCount();

		for (unsigned int cur_tri_index = 0; cur_tri_index < num_triangles; cur_tri_index++) {

			// Get all the information about this triangle
			// LTriangle2& cur_tri = cur_mesh.GetTriangle2(cur_tri_index);

			// Get the vertex indices for this triangle...
			const LTriangle& cur_indexed_tri = cur_mesh.GetTriangle(cur_tri_index);

			// Which CHAI mesh are we going to insert this triangle into?
			cMesh* cur_chai_mesh;

			// Default to the material-independent submesh
			cur_chai_mesh = sub_mesh;

			// Look for a material-specific submesh for this triangle if there is one...
			if (num_materials > 0) {

				int curMaterialId = cur_mesh.m_tris[cur_tri_index].materialId;

				// This material id is in terms of the global material id, but we need the
				// local material id to find the right CHAI mesh...
				int global_mat_id = curMaterialId;

				int local_mat_id = -1;

				// So we look it up in the list
				for (unsigned int k = 0; k < materials.size(); k++) {
					if (materials[k] == global_mat_id) {
						local_mat_id = k;
						break;
					}
				}


				if (local_mat_id == -1) {

					// Error finding the right mesh to put this triangle in
					// continue;

					// If there's no material, put it in the material-independent sub-mesh
					cur_chai_mesh = sub_mesh;
				}

				else cur_chai_mesh = newMeshes[local_mat_id];

			}

			unsigned int indexTriangle = 0;
			bool foundTriangle = false;

			if (g_3dsLoaderShouldGenerateExtraVertices == false) {

				uint_uint_map* vertex_map = (uint_uint_map*)cur_chai_mesh->m_userData;
				unsigned short indices[3];
				unsigned short* pindex = (unsigned short*)(&cur_indexed_tri);
				uint_uint_map::iterator viter;

				// For each vertex indexed by this triangle...
				for (int k = 0; k < 3; k++) {

					indices[k] = pindex[k];
					viter = vertex_map->find(indices[k]);

					LVector4 v = cur_mesh.GetVertex(indices[k]);
					// If we've never seen this vertex before...
					if (viter == vertex_map->end()) {
						// Create a new vertex and put him in this map
						int newVertexIndex = cur_chai_mesh->newVertex(v.x, v.y, v.z);
						(*vertex_map)[(indices[k])] = newVertexIndex;
						indices[k] = newVertexIndex;
					}
					else {
						// Otherwise just grab his index...
						indices[k] = (*viter).second;
					}
				}

				// Create the new triangle...
				indexTriangle = cur_chai_mesh->newTriangle(indices[0], indices[1], indices[2]);
			}

			if (g_3dsLoaderShouldGenerateExtraVertices == true) {

				LVector4 v0 = cur_mesh.GetVertex(cur_indexed_tri.a);
				LVector4 v1 = cur_mesh.GetVertex(cur_indexed_tri.b);
				LVector4 v2 = cur_mesh.GetVertex(cur_indexed_tri.c);

				cVector3d vert0(v0.x, v0.y, v0.z);
				cVector3d vert1(v1.x, v1.y, v1.z);
				cVector3d vert2(v2.x, v2.y, v2.z);

				indexTriangle = cur_chai_mesh->newTriangle(vert0, vert1, vert2);
			}

			cTriangle* curTriangle = cur_chai_mesh->getTriangle(indexTriangle);

			double transparency = cur_chai_mesh->m_material.m_diffuse.getA();

			// Give properties to each vertex of this triangle
			unsigned short* pindex = (unsigned short*)(&cur_indexed_tri);

			for (k = 0; k < 3; k++) {

				unsigned short curindex = pindex[k];

				LVector3 norm = cur_mesh.GetNormal(curindex);
				LVector2 uv = cur_mesh.GetUV(curindex);
				LColor3 color = cur_mesh.GetColor(curindex);

				cVertex* vertex = curTriangle->getVertex(k);
				vertex->setNormal(norm.x, norm.y, norm.z);
				vertex->setTexCoord(uv.x, 1.0 - uv.y);
				vertex->setColor(color.r, color.g, color.b, (float)transparency);
			}

		} // For every triangle in this 3ds mesh

		// We're now done with all the vertex maps for this mesh...
		for (int i = 0; i < num_materials; i++) {
			cMesh* curMesh = newMeshes[i];
			uint_uint_map* pmap = (uint_uint_map*)curMesh->m_userData;
			delete pmap;
		}

		if (newMeshes) delete[] newMeshes;

		uint_uint_map* pmap = (uint_uint_map*)sub_mesh->m_userData;
		delete pmap;

	} // For every mesh in the 3ds file

	// Copy relevant rendering state as generally useful flags in the main mesh.
	//
	// Don't over-write the "real" variables in other submeshes

	a_mesh->enableTransparency(transparency_enabled, false);
	a_mesh->useMaterial(materials_enabled, false);
	a_mesh->useColors(!materials_enabled, false);

	// compute boundary boxes
	a_mesh->computeBoundaryBox(true);

	// update global position in world
	if (world != 0) world->computeGlobalPositions(true);

	return true;

}

/******

  The remainder of this file comes from Lev Povalahev's l3ds.cpp
  copyright (c) 2001-2002 Lev Povalahev

******/

typedef unsigned long ulong;

#define SEEK_START           1900
#define SEEK_CURSOR          1901

// common chunks
// colors
#define COLOR_F             0x0010
#define COLOR_24            0x0011
#define LIN_COLOR_24        0x0012
#define LIN_COLOR_F         0x0013

// percentage
#define INT_PERCENTAGE      0x0030
#define FLOAT_PERCENTAGE    0x0031

// ambient light
#define AMBIENT_LIGHT       0x2100

#define MAIN3DS             0x4D4D
#define EDIT3DS             0x3D3D  // this is the start of the editor config

// keyframer chunk ids
#define KFDATA              0xB000  // the keyframer section
#define KFHDR               0xB00A
#define OBJECT_NODE_TAG     0xB002
#define NODE_HDR            0xB010
#define PIVOT               0xB013
#define POS_TRACK_TAG       0xB020
#define ROT_TRACK_TAG       0xB021
#define SCL_TRACK_TAG       0xB022

// material entries
#define MAT_ENTRY           0xAFFF
#define MAT_NAME            0xA000
#define MAT_AMBIENT         0xA010
#define MAT_DIFFUSE         0xA020
#define MAT_SPECULAR        0xA030
#define MAT_SHININESS       0xA040
#define MAT_SHIN2PCT        0xA041
#define MAT_TRANSPARENCY    0xA050
#define MAT_SHADING         0xA100
#define MAT_TWO_SIDE        0xA081
#define MAT_ADDITIVE        0xA083
#define MAT_WIRE            0xA085
#define MAT_FACEMAP         0xA088
#define MAT_WIRESIZE        0xA087
#define MAT_DECAL           0xA082
#define MAT_TEXMAP          0xA200
#define MAT_MAPNAME         0xA300
#define MAT_MAP_TILING      0xA351
#define MAT_MAP_USCALE      0xA354
#define MAT_MAP_VSCALE      0xA356
#define MAT_MAP_UOFFSET     0xA358
#define MAT_MAP_VOFFSET     0xA35A
#define MAT_MAP_ANG         0xA35C
#define MAT_TEX2MAP         0xA33A
#define MAT_OPACMAP         0xA210
#define MAT_BUMPMAP         0xA230
#define MAT_SPECMAP         0xA204
#define MAT_SHINMAP         0xA33C
#define MAT_REFLMAP         0xA220
#define MAT_ACUBIC          0xA310

#define EDIT_OBJECT         0x4000
#define OBJ_TRIMESH         0x4100
#define OBJ_LIGHT           0x4600
#define OBJ_CAMERA          0x4700

#define CAM_RANGES			0x4720

#define LIT_OFF             0x4620
#define LIT_SPOT            0x4610
#define LIT_INRANGE         0x4659
#define LIT_OUTRANGE        0x465A

#define TRI_VERTEXLIST      0x4110
#define TRI_VERTEXOPTIONS   0x4111
#define TRI_FACELIST        0x4120
#define TRI_MAT_GROUP       0x4130
#define TRI_SMOOTH_GROUP    0x4150
#define TRI_FACEMAPPING     0x4140
#define TRI_MATRIX          0x4160

#define SPOTLIGHT           0x4610

//---------------------------------------------------------------------------

#define MAX_SHARED_TRIS     100

// the error reporting routine

void ErrorMsg(const char *msg)
{
	/*
	char buf[1000];
	sprintf(buf,"%s\n",msg);
	CHAI_DEBUG_PRINT(buf);
	*/
}

//---------------------------------------------------------------------------

LColor3 black = { 0, 0, 0 };

LVector3 zero3(0, 0, 0);

LVector4 zero4 = { 0, 0, 0, 0 };

LMap emptyMap = { 0, "", 1, 1, 0, 0, 0 };

LVector3 _4to3(const LVector4 &vec)
{
	LVector3 t;
	t.x = vec.x;
	t.y = vec.y;
	t.z = vec.z;
	return t;
}

LVector3 AddVectors(const LVector3 &a, const LVector3 &b)
{
	LVector3 t;
	t.x = a.x + b.x;
	t.y = a.y + b.y;
	t.z = a.z + b.z;
	return t;
}

LVector3 SubtractVectors(const LVector3 &a, const LVector3 &b)
{
	LVector3 t;
	t.x = a.x - b.x;
	t.y = a.y - b.y;
	t.z = a.z - b.z;
	return t;
}

float VectorLength(const LVector3 &vec)
{
	return (float)sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

LVector3 NormalizeVector(const LVector3 &vec)
{
	float a = VectorLength(vec);
	if (a == 0)
		return vec;
	float b = 1 / a;
	LVector3 v;
	v.x = vec.x*b;
	v.y = vec.y*b;
	v.z = vec.z*b;
	return v;
}

LVector3 CrossProduct(const LVector3 &a, const LVector3 &b)
{
	LVector3 v;
	v.x = a.y*b.z - a.z*b.y;
	v.y = a.z*b.x - a.x*b.z;
	v.z = a.x*b.y - a.y*b.x;
	return v;
}

void LoadIdentityMatrix(LMatrix4 &m)
{
	m._11 = 1.0f;
	m._12 = 0.0f;
	m._13 = 0.0f;
	m._14 = 0.0f;

	m._21 = 0.0f;
	m._22 = 1.0f;
	m._23 = 0.0f;
	m._24 = 0.0f;

	m._31 = 0.0f;
	m._32 = 0.0f;
	m._33 = 1.0f;
	m._34 = 0.0f;

	m._41 = 0.0f;
	m._42 = 0.0f;
	m._43 = 1.0f;
	m._44 = 0.0f;
}

LVector4 VectorByMatrix(const LMatrix4 &m, const LVector4 &vec)
{
	LVector4 res;
	res.x = m._11*vec.x + m._12*vec.y + m._13*vec.z + m._14*vec.w;
	res.y = m._21*vec.x + m._22*vec.y + m._23*vec.z + m._24*vec.w;
	res.z = m._31*vec.x + m._32*vec.y + m._33*vec.z + m._34*vec.w;
	res.w = m._41*vec.x + m._42*vec.y + m._43*vec.z + m._44*vec.w;
	if (res.w != 0)
	{
		float b = 1 / res.w;
		res.x *= b;
		res.y *= b;
		res.z *= b;
		res.w = 1;
	}
	else
		res.w = 1;

	return res;
}

void QuatToMatrix(const LVector4 &quat, LMatrix4 &m)
{

}

//---------------------------------------------------------------------------
// LObject implementation
//---------------------------------------------------------------------------

LObject::LObject()
{
	m_name = "";//.clear();
}

LObject::~LObject()
{
	// nothing here
}

void LObject::SetName(const string& value)
{
	m_name = value;
}

const string& LObject::GetName()
{
	return m_name;
}

bool LObject::IsObject(const string &name)
{
	return (m_name == name);
}


//---------------------------------------------------------------------------
// LMaterial implementation
//---------------------------------------------------------------------------

LMaterial::LMaterial()
	: LObject()
{
	m_id = 0;
	m_texMap1 = emptyMap;
	m_texMap2 = emptyMap;
	m_opacMap = emptyMap;
	m_bumpMap = emptyMap;
	m_reflMap = emptyMap;
	m_specMap = emptyMap;
	m_ambient = black;
	m_diffuse = black;
	m_specular = black;
	m_shading = sGouraud;
	m_shininess = 0;
	m_transparency = 0;
}

LMaterial::~LMaterial()
{

}

uint LMaterial::GetID()
{
	return m_id;
}

LMap& LMaterial::GetTextureMap1()
{
	return m_texMap1;
}

LMap& LMaterial::GetTextureMap2()
{
	return m_texMap2;
}

LMap& LMaterial::GetOpacityMap()
{
	return m_opacMap;
}

LMap& LMaterial::GetSpecularMap()
{
	return m_specMap;
}

LMap& LMaterial::GetBumpMap()
{
	return m_bumpMap;
}

LMap& LMaterial::GetReflectionMap()
{
	return m_reflMap;
}

LColor3 LMaterial::GetAmbientColor()
{
	return m_ambient;
}

LColor3 LMaterial::GetDiffuseColor()
{
	return m_diffuse;
}

LColor3 LMaterial::GetSpecularColor()
{
	return m_specular;
}

float LMaterial::GetShininess()
{
	return m_shininess;
}

float LMaterial::GetTransparency()
{
	return m_transparency;
}

LShading LMaterial::GetShadingType()
{
	return m_shading;
}

void LMaterial::SetID(uint value)
{
	m_id = value;
}

void LMaterial::SetAmbientColor(const LColor3 &color)
{
	m_ambient = color;
}

void LMaterial::SetDiffuseColor(const LColor3 &color)
{
	m_diffuse = color;
}

void LMaterial::SetSpecularColor(const LColor3 &color)
{
	m_specular = color;
}

void LMaterial::SetShininess(float value)
{
	m_shininess = value;
	if (m_shininess < 0)
		m_shininess = 0;
	if (m_shininess > 1)
		m_shininess = 1;
}

void LMaterial::SetTransparency(float value)
{
	m_transparency = value;
	if (m_transparency < 0)
		m_transparency = 0;
	if (m_transparency > 1)
		m_transparency = 1;
}

void LMaterial::SetShadingType(LShading shading)
{
	m_shading = shading;
}

//---------------------------------------------------------------------------
// LMesh implementation
//---------------------------------------------------------------------------

LMesh::LMesh()
	: LObject()
{
	Clear();
}

LMesh::~LMesh()
{
	Clear();
}

void LMesh::Clear()
{
	m_vertices.clear();
	m_normals.clear();
	m_uv.clear();
	m_colors.clear();
	m_tangents.clear();
	m_binormals.clear();
	m_triangles.clear();
	m_tris.clear();
	m_materials.clear();
	LoadIdentityMatrix(m_matrix);
}

uint LMesh::GetVertexCount()
{
	return m_vertices.size();
}

void LMesh::SetVertexArraySize(uint value)
{
	m_vertices.resize(value);
	m_normals.resize(value);
	m_uv.resize(value);
	m_colors.resize(value);
	m_tangents.resize(value);
	m_binormals.resize(value);
}

uint LMesh::GetTriangleCount()
{
	return m_triangles.size();
}

void LMesh::SetTriangleArraySize(uint value)
{
	m_triangles.reserve(value);
	m_tris.reserve(value);
	m_tris.resize(value);
	m_triangles.resize(value);
}

const LVector4& LMesh::GetVertex(uint index)
{
	return m_vertices[index];
}

const LVector3& LMesh::GetNormal(uint index)
{
	return m_normals[index];
}

LColor3& LMesh::GetColor(uint index) {
	return m_colors[index];
}


const LVector2& LMesh::GetUV(uint index)
{
	return m_uv[index];
}

const LVector3& LMesh::GetTangent(uint index)
{
	return m_tangents[index];
}

const LVector3& LMesh::GetBinormal(uint index)
{
	return m_binormals[index];
}

void LMesh::SetVertex(const LVector4 &vec, uint index)
{
	if (index >= m_vertices.size())
		return;
	m_vertices[index] = vec;
}

void LMesh::SetNormal(const LVector3 &vec, uint index)
{
	if (index >= m_vertices.size())
		return;
	m_normals[index] = vec;
}

void LMesh::SetUV(const LVector2 &vec, uint index)
{
	if (index >= m_vertices.size())
		return;
	m_uv[index] = vec;
}


void LMesh::SetColor(const LColor3 &vec, uint index)
{
	if (index >= m_vertices.size())
		return;
	m_colors[index] = vec;
}


void LMesh::SetTangent(const LVector3 &vec, uint index)
{
	if (index >= m_vertices.size())
		return;
	m_tangents[index] = vec;
}

void LMesh::SetBinormal(const LVector3 &vec, uint index)
{
	if (index >= m_vertices.size())
		return;
	m_binormals[index] = vec;
}

const LTriangle& LMesh::GetTriangle(uint index)
{
	return m_triangles[index];
}

LTriangle2 LMesh::GetTriangle2(uint index)
{
	LTriangle2 f;
	LTriangle t = GetTriangle(index);
	f.vertices[0] = GetVertex(t.a);
	f.vertices[1] = GetVertex(t.b);
	f.vertices[2] = GetVertex(t.c);

	f.vertexNormals[0] = GetNormal(t.a);
	f.vertexNormals[1] = GetNormal(t.b);
	f.vertexNormals[2] = GetNormal(t.c);

	f.textureCoords[0] = GetUV(t.a);
	f.textureCoords[1] = GetUV(t.b);
	f.textureCoords[2] = GetUV(t.c);

	f.color[0] = GetColor(t.a);
	f.color[1] = GetColor(t.b);
	f.color[2] = GetColor(t.c);

	LVector3 a, b;

	a = SubtractVectors(_4to3(f.vertices[1]), _4to3(f.vertices[0]));
	b = SubtractVectors(_4to3(f.vertices[1]), _4to3(f.vertices[2]));

	f.faceNormal = CrossProduct(b, a);

	f.faceNormal = NormalizeVector(f.faceNormal);

	f.materialId = m_tris[index].materialId;

	return f;
}

LMatrix4 LMesh::GetMatrix()
{
	return m_matrix;
}

void LMesh::SetMatrix(LMatrix4 m)
{
	m_matrix = m;
}

void LMesh::TransformVertices()
{

	for (uint i = 0; i < m_vertices.size(); i++)
		m_vertices[i] = VectorByMatrix(m_matrix, m_vertices[i]);
}

void LMesh::CalcNormals(bool useSmoothingGroups)
{
	uint i;
	// first calculate the face normals
	for (i = 0; i < m_triangles.size(); i++)
	{
		LVector3 a, b;
		a = SubtractVectors(_4to3(m_vertices[m_tris[i].b]), _4to3(m_vertices[m_tris[i].a]));
		b = SubtractVectors(_4to3(m_vertices[m_tris[i].b]), _4to3(m_vertices[m_tris[i].c]));
		m_tris[i].normal = NormalizeVector(CrossProduct(b, a));
	}

	vector< vector<int> > array;
	array.resize(m_vertices.size());
	for (i = 0; i < m_triangles.size(); i++)
	{
		uint k = m_tris[i].a;
		array[k].push_back(i);

		k = m_tris[i].b;
		array[k].push_back(i);

		k = m_tris[i].c;
		array[k].push_back(i);
	}

	LVector3 temp;

	if (!useSmoothingGroups)
	{
		// now calculate the normals without using smoothing groups
		for (i = 0; i < m_vertices.size(); i++)
		{
			temp = zero3;
			int t = array[i].size();

			for (int k = 0; k < t; k++)
			{
				temp.x += m_tris[array[i][k]].normal.x;
				temp.y += m_tris[array[i][k]].normal.y;
				temp.z += m_tris[array[i][k]].normal.z;
			}
			m_normals[i] = NormalizeVector(temp);
		}
	}
	else
	{
		// now calculate the normals _USING_ smoothing groups
		// I'm assuming a triangle can only belong to one smoothing group at a time!
		vector<ulong> smGroups;
		vector< vector <uint> > smList;

		uint loop_size = m_vertices.size();

		for (i = 0; i < loop_size; i++)
		{
			int t = array[i].size();

			if (t == 0)
				continue;

			smGroups.clear();
			smList.clear();
			smGroups.push_back(m_tris[array[i][0]].smoothingGroups);
			smList.resize(smGroups.size());
			smList[smGroups.size() - 1].push_back(array[i][0]);

			// first build a list of smoothing groups for the vertex
			for (int k = 0; k < t; k++)
			{
				bool found = false;
				for (uint j = 0; j < smGroups.size(); j++)
				{
					if (m_tris[array[i][k]].smoothingGroups == smGroups[j])
					{
						smList[j].push_back(array[i][k]);
						found = true;
					}
				}
				if (!found)
				{
					smGroups.push_back(m_tris[array[i][k]].smoothingGroups);
					smList.resize(smGroups.size());
					smList[smGroups.size() - 1].push_back(array[i][k]);
				}
			}
			// now we have the list of faces for the vertex sorted by smoothing groups


			// now duplicate the vertices so that there's only one smoothing group "per vertex"
			if (smGroups.size() > 1)
				for (uint j = 1; j < smGroups.size(); j++)
				{
					m_vertices.push_back(m_vertices[i]);
					m_normals.push_back(m_normals[i]);
					m_uv.push_back(m_uv[i]);
					m_colors.push_back(m_colors[i]);
					m_tangents.push_back(m_tangents[i]);
					m_binormals.push_back(m_binormals[i]);

					uint t = m_vertices.size() - 1;
					for (uint h = 0; h < smList[j].size(); h++)
					{
						if (m_tris[smList[j][h]].a == i)
							m_tris[smList[j][h]].a = t;
						if (m_tris[smList[j][h]].b == i)
							m_tris[smList[j][h]].b = t;
						if (m_tris[smList[j][h]].c == i)
							m_tris[smList[j][h]].c = t;
					}
				}
		}

		// now rebuild a face list for each vertex, since the old one is invalidated
		for (i = 0; i < array.size(); i++)
			array[i].clear();
		array.clear();
		array.resize(m_vertices.size());
		for (i = 0; i < m_triangles.size(); i++)
		{
			uint k = m_tris[i].a;
			array[k].push_back(i);

			k = m_tris[i].b;
			array[k].push_back(i);

			k = m_tris[i].c;
			array[k].push_back(i);
		}

		// now compute the normals
		for (i = 0; i < m_vertices.size(); i++)
		{
			temp = zero3;
			int t = array[i].size();

			for (int k = 0; k < t; k++)
			{
				temp.x += m_tris[array[i][k]].normal.x;
				temp.y += m_tris[array[i][k]].normal.y;
				temp.z += m_tris[array[i][k]].normal.z;
			}
			m_normals[i] = NormalizeVector(temp);
		}

	}

	// copy m_tris to m_triangles
	for (i = 0; i < m_triangles.size(); i++)
	{
		m_triangles[i].a = m_tris[i].a;
		m_triangles[i].b = m_tris[i].b;
		m_triangles[i].c = m_tris[i].c;
	}
}

void LMesh::CalcTextureSpace()
{
	// a understandable description of how to do that can be found here:
	// http://members.rogers.com/deseric/tangentspace.htm
	// first calculate the tangent for each triangle
	LVector3 x_vec,
		y_vec,
		z_vec;
	LVector3 v1, v2;
	uint i;

	for (i = 0; i < m_triangles.size(); i++)
	{
		v1.x = m_vertices[m_tris[i].b].x - m_vertices[m_tris[i].a].x;
		v1.y = m_uv[m_tris[i].b].x - m_uv[m_tris[i].a].x;
		v1.z = m_uv[m_tris[i].b].y - m_uv[m_tris[i].a].y;

		v2.x = m_vertices[m_tris[i].c].x - m_vertices[m_tris[i].a].x;
		v2.y = m_uv[m_tris[i].c].x - m_uv[m_tris[i].a].x;
		v2.z = m_uv[m_tris[i].c].y - m_uv[m_tris[i].a].y;

		x_vec = CrossProduct(v1, v2);

		v1.x = m_vertices[m_tris[i].b].y - m_vertices[m_tris[i].a].y;
		v1.y = m_uv[m_tris[i].b].x - m_uv[m_tris[i].a].x;
		v1.z = m_uv[m_tris[i].b].y - m_uv[m_tris[i].a].y;

		v2.x = m_vertices[m_tris[i].c].y - m_vertices[m_tris[i].a].y;
		v2.y = m_uv[m_tris[i].c].x - m_uv[m_tris[i].a].x;
		v2.z = m_uv[m_tris[i].c].y - m_uv[m_tris[i].a].y;

		y_vec = CrossProduct(v1, v2);

		v1.x = m_vertices[m_tris[i].b].z - m_vertices[m_tris[i].a].z;
		v1.y = m_uv[m_tris[i].b].x - m_uv[m_tris[i].a].x;
		v1.z = m_uv[m_tris[i].b].y - m_uv[m_tris[i].a].y;

		v2.x = m_vertices[m_tris[i].c].z - m_vertices[m_tris[i].a].z;
		v2.y = m_uv[m_tris[i].c].x - m_uv[m_tris[i].a].x;
		v2.z = m_uv[m_tris[i].c].y - m_uv[m_tris[i].a].y;

		z_vec = CrossProduct(v1, v2);

		m_tris[i].tangent.x = -(x_vec.y / x_vec.x);
		m_tris[i].tangent.y = -(y_vec.y / y_vec.x);
		m_tris[i].tangent.z = -(z_vec.y / z_vec.x);

		m_tris[i].binormal.x = -(x_vec.z / x_vec.x);
		m_tris[i].binormal.y = -(y_vec.z / y_vec.x);
		m_tris[i].binormal.z = -(z_vec.z / z_vec.x);

	}

	// now for each vertex build a list of face that share this vertex
	vector< vector<int> > array;
	array.resize(m_vertices.size());

	for (i = 0; i < m_triangles.size(); i++)
	{
		uint k = m_tris[i].a;
		array[k].push_back(i);

		k = m_tris[i].b;
		array[k].push_back(i);

		k = m_tris[i].c;
		array[k].push_back(i);
	}

	// now average the tangents and compute the binormals as (tangent X normal)
	for (i = 0; i < m_vertices.size(); i++)
	{
		v1 = zero3;
		v2 = zero3;
		int t = array[i].size();

		for (int k = 0; k < t; k++)
		{
			v1.x += m_tris[array[i][k]].tangent.x;
			v1.y += m_tris[array[i][k]].tangent.y;
			v1.z += m_tris[array[i][k]].tangent.z;

			v2.x += m_tris[array[i][k]].binormal.x;
			v2.y += m_tris[array[i][k]].binormal.y;
			v2.z += m_tris[array[i][k]].binormal.z;
		}
		m_tangents[i] = NormalizeVector(v1);
		//m_binormals[i] = NormalizeVector(v2);

		m_binormals[i] = NormalizeVector(CrossProduct(m_tangents[i], m_normals[i]));
	}
}

void LMesh::Optimize(LOptimizationLevel value)
{
	switch (value)
	{
	case oNone:
		//TransformVertices();
		break;
	case oSimple:
		//TransformVertices();
		CalcNormals(false);
		break;
	case oFull:
		//TransformVertices();
		CalcNormals(true);
		CalcTextureSpace();
		break;
	}
}

void LMesh::SetTri(const LTri &tri, uint index)
{
	if (index >= m_triangles.size())
		return;
	m_tris[index] = tri;
}

LTri& LMesh::GetTri(uint index)
{
	return m_tris[index];
}

uint LMesh::GetMaterial(uint index)
{
	return m_materials[index];
}

uint LMesh::AddMaterial(uint id)
{
	m_materials.push_back(id);
	return m_materials.size() - 1;
}

uint LMesh::GetMaterialCount()
{
	return m_materials.size();
}

//---------------------------------------------------------------------------
// LCamera implementation
//---------------------------------------------------------------------------

LCamera::LCamera()
	: LObject()
{
	Clear();
}

LCamera::~LCamera()
{

}

void LCamera::Clear()
{
	m_pos.x = m_pos.y = m_pos.z = 0.0f;
	m_target.x = m_target.y = m_target.z = 0.0f;
	m_fov = 80;
	m_bank = 0;;
	m_near = 10;
	m_far = 10000;
}

void LCamera::SetPosition(LVector3 vec)
{
	m_pos = vec;
}

LVector3 LCamera::GetPosition()
{
	return m_pos;
}

void LCamera::SetTarget(LVector3 target)
{
	m_target = target;
}

LVector3 LCamera::GetTarget()
{
	return m_target;
}

void LCamera::SetFOV(float value)
{
	m_fov = value;
}

float LCamera::GetFOV()
{
	return m_fov;
}

void LCamera::SetBank(float value)
{
	m_bank = value;
}

float LCamera::GetBank()
{
	return m_bank;
}

void LCamera::SetNearplane(float value)
{
	m_near = value;
}

float LCamera::GetNearplane()
{
	return m_near;
}

void LCamera::SetFarplane(float value)
{
	m_far = value;
}

float LCamera::GetFarplane()
{
	return m_far;
}

//---------------------------------------------------------------------------
// LLight implementation
//---------------------------------------------------------------------------

LLight::LLight()
	: LObject()
{
	Clear();
}

LLight::~LLight()
{

}

void LLight::Clear()
{
	m_pos.x = m_pos.y = m_pos.z = 0.0f;
	m_color.r = m_color.g = m_color.b = 0.0f;
	m_spotlight = false;
	m_attenuationend = 100;
	m_attenuationstart = 1000;
}

void LLight::SetPosition(LVector3 vec)
{
	m_pos = vec;
}

LVector3 LLight::GetPosition()
{
	return m_pos;
}

void LLight::SetColor(LColor3 color)
{
	m_color = color;
}

LColor3 LLight::GetColor()
{
	return m_color;
}

void LLight::SetSpotlight(bool value)
{
	m_spotlight = value;
}

bool LLight::GetSpotlight()
{
	return m_spotlight;
}

void LLight::SetTarget(LVector3 target)
{
	m_target = target;
}

LVector3 LLight::GetTarget()
{
	return m_target;
}

void LLight::SetHotspot(float value)
{
	m_hotspot = value;
}

float LLight::GetHotspot()
{
	return m_hotspot;
}

void LLight::SetFalloff(float value)
{
	m_falloff = value;
}

float LLight::GetFalloff()
{
	return m_falloff;
}

void LLight::SetAttenuationstart(float value)
{
	m_attenuationend = value;
}

float LLight::GetAttenuationstart()
{
	return m_attenuationend;
}

void LLight::SetAttenuationend(float value)
{
	m_attenuationstart = value;
}

float LLight::GetAttenuationend()
{
	return m_attenuationstart;
}

//---------------------------------------------------------------------------
// LImporter implementation
//---------------------------------------------------------------------------

LImporter::LImporter()
{
	Clear();
}

LImporter::~LImporter()
{
	Clear();
}

uint LImporter::GetMeshCount()
{
	return m_meshes.size();
}

uint LImporter::GetLightCount()
{
	return m_lights.size();
}

uint LImporter::GetMaterialCount()
{
	return m_materials.size();
}

uint LImporter::GetCameraCount()
{
	return m_cameras.size();
}

LMesh& LImporter::GetMesh(uint index)
{
	return m_meshes[index];
}

LLight& LImporter::GetLight(uint index)
{
	return m_lights[index];
}

LMaterial& LImporter::GetMaterial(uint index)
{
	return m_materials[index];
}

LCamera& LImporter::GetCamera(uint index)
{
	return m_cameras[index];
}

LMaterial* LImporter::FindMaterial(const string& name)
{
	for (uint i = 0; i < m_materials.size(); i++)
		if (m_materials[i].IsObject(name))
			return &m_materials[i];
	return 0;
}

LMesh* LImporter::FindMesh(const string& name)
{
	for (uint i = 0; i < m_meshes.size(); i++)
		if (m_meshes[i].IsObject(name))
			return &m_meshes[i];
	return 0;
}

LLight* LImporter::FindLight(const string& name)
{
	for (uint i = 0; i < m_lights.size(); i++)
		if (m_lights[i].IsObject(name))
			return &m_lights[i];
	return 0;
}

void LImporter::Clear()
{
	m_meshes.clear();
	m_lights.clear();
	m_materials.clear();
	m_optLevel = oSimple;
}

void LImporter::SetOptimizationLevel(LOptimizationLevel value)
{
	m_optLevel = value;
}

LOptimizationLevel LImporter::GetOptimizationLevel()
{
	return m_optLevel;
}

//---------------------------------------------------------------------------
// L3DS implementation
//---------------------------------------------------------------------------

L3DS::L3DS()
	: LImporter()
{
	m_buffer = 0;
	m_bufferSize = 0;
	m_pos = 0;
	m_eof = false;
}

L3DS::L3DS(const char *filename)
	: LImporter()
{
	m_buffer = 0;
	m_bufferSize = 0;
	m_pos = 0;
	m_eof = false;
	LoadFile(filename);
}

L3DS::~L3DS()
{
	if (m_bufferSize > 0)
		free(m_buffer);
}

bool L3DS::LoadFile(const char *filename) {
	FILE *f;
	f = fopen(filename, "rb");
	if (f == 0)
	{
		ErrorMsg("L3DS::LoadFile - cannot open file");
		return false;
	}
	fseek(f, 0, SEEK_END);
	m_bufferSize = ftell(f);
	fseek(f, 0, SEEK_SET);
	m_buffer = (unsigned char*)calloc(m_bufferSize, 1);
	if (m_buffer == 0)
	{
		ErrorMsg("L3DS::LoadFile - not enough memory (malloc failed)");
		return false;
	}
	if (fread(m_buffer, m_bufferSize, 1, f) != 1)
	{
		fclose(f);
		free(m_buffer);
		m_bufferSize = 0;
		ErrorMsg("L3DS::LoadFile - error reading from file");
		return false;
	}
	fclose(f);
	Clear();
	bool res = Read3DS();
	free(m_buffer);
	m_buffer = 0;
	m_bufferSize = 0;


	/***

	 Added by Dan Morris.

	 Populates the vertex color list from the material array, using
	 diffuse colors, to approximate the correct colors.

	 ***/
	if (this->GetMaterialCount() > 0) {

		uint last_mat = 0;

		// Now build colors for each triangle...
		for (uint i = 0; i < this->GetMeshCount(); i++) {

			LMesh &mesh = this->GetMesh(i);

			LColor3* color_array = &(mesh.GetColor(0));

			// For each triangle
			for (uint j = 0; j < mesh.m_tris.size(); j++) {

				uint mat_id = mesh.m_tris[j].materialId;

				if (mat_id >= this->GetMaterialCount()) {
					mat_id = last_mat;
				}

				last_mat = mat_id;

				LMaterial& mat = this->GetMaterial(mat_id);

				LColor3 c = mat.GetDiffuseColor();

				// Set each vertex's color          
				mesh.SetColor(c, mesh.m_tris[j].a);
				mesh.SetColor(c, mesh.m_tris[j].b);
				mesh.SetColor(c, mesh.m_tris[j].c);
			}
		}

	}

	return res;
}

short L3DS::ReadShort()
{
	if ((m_buffer != 0) && (m_bufferSize != 0) && ((m_pos + 2) < m_bufferSize))
	{
		short *w = (short*)(m_buffer + m_pos);
		short s = *w;//(short)*(m_buffer+m_pos);
		m_pos += 2;
		return s;
	}
	m_eof = true;
	return 0;
}

int L3DS::ReadInt()
{
	if ((m_buffer != 0) && (m_bufferSize != 0) && ((m_pos + 4) < m_bufferSize))
	{
		int *w = (int*)(m_buffer + m_pos);
		int s = *w;//(int)*(m_buffer+m_pos);
		m_pos += 4;
		return s;
	}
	m_eof = true;
	return 0;
}

char L3DS::ReadChar()
{
	if ((m_buffer != 0) && (m_bufferSize != 0) && ((m_pos + 1) < m_bufferSize))
	{
		char s = (char)*(m_buffer + m_pos);
		m_pos += 1;
		return s;
	}
	m_eof = true;
	return 0;
}

float L3DS::ReadFloat()
{
	if ((m_buffer != 0) && (m_bufferSize != 0) && ((m_pos + 4) < m_bufferSize))
	{
		float *w = (float*)(m_buffer + m_pos);
		float s = *w;//(float)*(m_buffer+m_pos);
		m_pos += 4;
		return s;
	}
	m_eof = true;
	return 0.0;
}

byte L3DS::ReadByte()
{
	if ((m_buffer != 0) && (m_bufferSize != 0) && ((m_pos + 1) < m_bufferSize))
	{
		byte s = (byte)*(m_buffer + m_pos);
		m_pos += 1;
		return s;
	}
	m_eof = true;
	return 0;
}

int L3DS::ReadASCIIZ(char *buf, int max_count)
{
	int count;
	if ((m_buffer == 0) || (m_bufferSize == 0) || (m_pos >= m_bufferSize))
	{
		count = 0;
		m_eof = true;
		return count;
	}
	count = 0;
	char c = ReadChar();
	while ((c != 0) && (count < max_count - 1))
	{
		buf[count] = c;
		count++;
		c = ReadChar();
	}
	buf[count] = 0;
	return count;
}

void L3DS::Seek(int offset, int origin)
{
	if (origin == SEEK_START)
		m_pos = offset;
	if (origin == SEEK_CURSOR)
		m_pos += offset;
	if (m_pos < 0)
		m_pos = 0;
	if (m_pos >= m_bufferSize)
		m_pos = m_bufferSize - 1;
	m_eof = false;
}

uint L3DS::Pos()
{
	return m_pos;
}

LChunk L3DS::ReadChunk()
{
	LChunk chunk;
	chunk.id = ReadShort();
	int a = ReadInt();
	chunk.start = Pos();
	chunk.end = chunk.start + a - 6;
	return chunk;
}

bool L3DS::FindChunk(LChunk &target, const LChunk &parent)
{
	if (Pos() >= parent.end)
		return false;
	LChunk chunk;
	chunk = ReadChunk();
	while ((chunk.id != target.id) && (chunk.end <= parent.end))
	{
		SkipChunk(chunk);
		if (chunk.end >= parent.end)
			break;
		chunk = ReadChunk();
	}
	if (chunk.id == target.id)
	{
		target.start = chunk.start;
		target.end = chunk.end;
		return true;
	}
	return false;
}

void L3DS::SkipChunk(const LChunk &chunk)
{
	Seek(chunk.end, SEEK_START);
}

void L3DS::GotoChunk(const LChunk &chunk)
{
	Seek(chunk.start, SEEK_START);
}

LColor3 L3DS::ReadColor(const LChunk &chunk)
{
	LColor3 col = black;
	GotoChunk(chunk);
	switch (chunk.id)
	{
	case COLOR_F:
		col.r = ReadFloat();
		col.g = ReadFloat();
		col.b = ReadFloat();
		break;
	case COLOR_24:
		col.r = ReadByte() / 255.0f;
		col.g = ReadByte() / 255.0f;
		col.b = ReadByte() / 255.0f;
		break;
	case LIN_COLOR_F:
		col.r = ReadFloat();
		col.g = ReadFloat();
		col.b = ReadFloat();
		break;
	case LIN_COLOR_24:
		col.r = ReadByte() / 255.0f;
		col.g = ReadByte() / 255.0f;
		col.b = ReadByte() / 255.0f;
		break;
	default:
		ErrorMsg("L3DS::ReadColor - error this is not a color chunk");
	}
	return col;
}

float L3DS::ReadPercentage(const LChunk &chunk)
{
	GotoChunk(chunk);
	switch (chunk.id)
	{
	case INT_PERCENTAGE:
		return (ReadShort() / 100.0f);
	case FLOAT_PERCENTAGE:
		return ReadFloat();
	}
	ErrorMsg("L3DS::ReadPercentage - error, the chunk is not a percentage chunk");
	return 0;
}

bool L3DS::Read3DS()
{
	LChunk mainchunk;
	LChunk edit;
	edit.id = EDIT3DS;
	mainchunk = ReadChunk();
	if (mainchunk.id != MAIN3DS)
	{
		ErrorMsg("L3DS::Read3DS - wrong file format");
		return false;
	}
	if (!FindChunk(edit, mainchunk))
		return false;
	LChunk obj;
	LChunk ml;

	GotoChunk(edit);
	obj.id = MAT_ENTRY;
	while (FindChunk(obj, edit))
	{
		ReadMaterial(obj);
		SkipChunk(obj);
	}
	GotoChunk(edit);

	int chunkcount = 0;

	obj.id = EDIT_OBJECT;
	{
		while (FindChunk(obj, edit))
		{
			ReadASCIIZ(m_objName, 99);
			ml = ReadChunk();
			if (ml.id == OBJ_TRIMESH)
			{
				ReadMesh(ml);
			}
			else if (ml.id == OBJ_LIGHT)
			{
				ReadLight(ml);
			}
			else if (ml.id == OBJ_CAMERA)
			{
				ReadCamera(ml);
			}
			SkipChunk(obj);
		}
	}

	// read the keyframer data here to find out correct object orientation

	LChunk keyframer;
	keyframer.id = KFDATA;

	LChunk objtrack;
	objtrack.id = OBJECT_NODE_TAG;

	GotoChunk(mainchunk);
	if (FindChunk(keyframer, mainchunk))
	{   // keyframer chunk is present
		GotoChunk(keyframer);
		while (FindChunk(objtrack, keyframer))
		{
			ReadKeyframeData(objtrack);
			SkipChunk(objtrack);
		}
	}

	for (uint i = 0; i < m_meshes.size(); i++)
		m_meshes[i].Optimize(m_optLevel);
	m_pos = 0;
	strcpy(m_objName, "");
	return true;
}

void L3DS::ReadLight(const LChunk &parent)
{
	float t;
	LVector3 v;
	LLight light;
	light.SetName(m_objName);
	v.x = ReadFloat();
	v.y = ReadFloat();
	v.z = ReadFloat();
	light.SetPosition(v);
	LChunk chunk = ReadChunk();
	while (chunk.end <= parent.end)
	{
		switch (chunk.id)
		{
		case COLOR_24:
		case COLOR_F:
		case LIN_COLOR_F:
		case LIN_COLOR_24:
			light.SetColor(ReadColor(chunk));
			break;
		case SPOTLIGHT:
			v.x = ReadFloat();
			v.y = ReadFloat();
			v.z = ReadFloat();
			light.SetTarget(v);
			t = ReadFloat();
			light.SetHotspot(t);
			t = ReadFloat();
			light.SetFalloff(t);
			break;
		case LIT_INRANGE:
			light.SetAttenuationstart(ReadFloat());
			break;
		case LIT_OUTRANGE:
			light.SetAttenuationend(ReadFloat());
			break;
		default:
			break;
		}
		SkipChunk(chunk);
		if (chunk.end >= parent.end)
			break;
		chunk = ReadChunk();

	}
	m_lights.push_back(light);
}

void L3DS::ReadCamera(const LChunk &parent)
{
	LVector3 v, t;
	LCamera camera;
	camera.SetName(m_objName);
	v.x = ReadFloat();
	v.y = ReadFloat();
	v.z = ReadFloat();
	t.x = ReadFloat();
	t.y = ReadFloat();
	t.z = ReadFloat();
	camera.SetPosition(v);
	camera.SetTarget(t);
	camera.SetBank(ReadFloat());
	camera.SetFOV(2400.0f / ReadFloat());
	LChunk chunk = ReadChunk();
	while (chunk.end <= parent.end)
	{
		switch (chunk.id)
		{
		case CAM_RANGES:
			camera.SetNearplane(ReadFloat());
			camera.SetFarplane(ReadFloat());
			break;
		default:
			break;
		}
		SkipChunk(chunk);
		if (chunk.end >= parent.end)
			break;
		chunk = ReadChunk();

	}
	m_cameras.push_back(camera);
}

void L3DS::ReadMesh(const LChunk &parent)
{
	unsigned short count, i;
	LVector4 p;
	LMatrix4 m;
	LVector2 t;
	p.w = 1.0f;
	LMesh mesh;
	mesh.SetName(m_objName);
	GotoChunk(parent);
	LChunk chunk = ReadChunk();
	while (chunk.end <= parent.end)
	{
		switch (chunk.id)
		{
		case TRI_VERTEXLIST:
			count = ReadShort();
			mesh.SetVertexArraySize(count);
			for (i = 0; i < count; i++)
			{
				p.x = ReadFloat();
				p.y = ReadFloat();
				p.z = ReadFloat();
				mesh.SetVertex(p, i);
			}
			break;
		case TRI_FACEMAPPING:
			count = ReadShort();
			if (mesh.GetVertexCount() == 0)
				mesh.SetVertexArraySize(count);
			for (i = 0; i < count; i++)
			{
				t.x = ReadFloat();
				t.y = ReadFloat();
				mesh.SetUV(t, i);
			}
			break;
		case TRI_FACELIST:
			ReadFaceList(chunk, mesh);
			break;
		case TRI_MATRIX:
			m._11 = ReadFloat();
			m._12 = ReadFloat();
			m._13 = ReadFloat();

			m._21 = ReadFloat();
			m._22 = ReadFloat();
			m._23 = ReadFloat();

			m._31 = ReadFloat();
			m._32 = ReadFloat();
			m._33 = ReadFloat();

			/*
			m._41 = ReadFloat();
			m._42 = ReadFloat();
			m._43 = ReadFloat();
			*/

			m._41 = 0.0;
			m._42 = 0.0;
			m._43 = 0.0;

			m._14 = 0.0f;
			m._24 = 0.0f;
			m._34 = 0.0f;
			m._44 = 1.0f;

			mesh.SetMatrix(m);

			break;
		default:
			break;
		}
		SkipChunk(chunk);
		if (chunk.end >= parent.end)
			break;
		chunk = ReadChunk();
	}
	m_meshes.push_back(mesh);
}

void L3DS::ReadFaceList(const LChunk &chunk, LMesh &mesh)
{
	// variables 
	unsigned short count, t;
	uint i;
	LTri tri;
	LChunk ch;
	char str[20];
	//uint mat;

	// consistency checks
	if (chunk.id != TRI_FACELIST)
	{
		ErrorMsg("L3DS::ReadFaceList - internal error: wrong chunk passed as parameter");
		return;
	}
	GotoChunk(chunk);
	tri.smoothingGroups = 1;
	// read the number of faces
	count = ReadShort();
	mesh.SetTriangleArraySize(count);

	for (i = 0; i < count; i++)
	{
		tri.a = ReadShort();
		tri.b = ReadShort();
		tri.c = ReadShort();
		ReadShort();
		mesh.SetTri(tri, i);
	}

	// now read the optional chunks
	ch = ReadChunk();
	int mat_id;
	while (ch.end <= chunk.end)
	{
		LMaterial* mat;

		switch (ch.id)
		{
		case TRI_MAT_GROUP:
			ReadASCIIZ(str, 20);
			mat = FindMaterial(str);

			if (mat) {
				mat_id = mat->GetID();
				mesh.AddMaterial(mat_id);
			}

			count = ReadShort();

			for (i = 0; i < count; i++) {

				t = ReadShort();
				if (mat) mesh.GetTri(t).materialId = mat_id;
			}

			break;
		case TRI_SMOOTH_GROUP:
			for (i = 0; i < mesh.GetTriangleCount(); i++)
				mesh.GetTri(i).smoothingGroups = (ulong)ReadInt();
			break;
		}
		SkipChunk(ch);
		ch = ReadChunk();
	}
}

void L3DS::ReadMaterial(const LChunk &parent) {
	// variables
	LChunk chunk;
	LChunk child;
	char str[30];
	LMaterial mat;
	short sh;

	GotoChunk(parent);

	chunk = ReadChunk();
	while (chunk.end <= parent.end)
	{
		switch (chunk.id)
		{
		case MAT_NAME:
			ReadASCIIZ(str, 30);
			mat.SetName(str);
			break;
		case MAT_AMBIENT:
			child = ReadChunk();
			mat.SetAmbientColor(ReadColor(child));
			break;
		case MAT_DIFFUSE:
			child = ReadChunk();
			mat.SetDiffuseColor(ReadColor(child));
			break;
		case MAT_SPECULAR:
			child = ReadChunk();
			mat.SetSpecularColor(ReadColor(child));
			break;
		case MAT_SHININESS:
			child = ReadChunk();
			mat.SetShininess(ReadPercentage(child));
			break;
		case MAT_TRANSPARENCY:
			child = ReadChunk();
			mat.SetTransparency(ReadPercentage(child));
			break;
		case MAT_SHADING:
			sh = ReadShort();
			switch (sh)
			{
			case 0:
				mat.SetShadingType(sWireframe);
				break;
			case 1:
				mat.SetShadingType(sFlat);
				break;
			case 2:
				mat.SetShadingType(sGouraud);
				break;
			case 3:
				mat.SetShadingType(sPhong);
				break;
			case 4:
				mat.SetShadingType(sMetal);
				break;
			}
			break;
		case MAT_WIRE:
			mat.SetShadingType(sWireframe);
			break;
		case MAT_TEXMAP:
			ReadMap(chunk, mat.GetTextureMap1());
			break;
		case MAT_TEX2MAP:
			ReadMap(chunk, mat.GetTextureMap2());
			break;
		case MAT_OPACMAP:
			ReadMap(chunk, mat.GetOpacityMap());
			break;
		case MAT_BUMPMAP:
			ReadMap(chunk, mat.GetBumpMap());
			break;
		case MAT_SPECMAP:
			ReadMap(chunk, mat.GetSpecularMap());
			break;
		case MAT_REFLMAP:
			child = ReadChunk();
			mat.GetReflectionMap().strength = ReadPercentage(child);
			SkipChunk(child);
			child = ReadChunk();
			if (child.id != MAT_MAPNAME)
			{
				ErrorMsg("L3DS::ReadMaterial - error, expected chunk not found");
				return;
			}
			ReadASCIIZ(str, 30);
			if (strcmp(str, "") == 0)
				strcpy(mat.GetReflectionMap().mapName, "auto");
			break;
		}

		SkipChunk(chunk);
		chunk = ReadChunk();
	}
	m_materials.push_back(mat);
	m_materials[m_materials.size() - 1].SetID(m_materials.size() - 1);
}

void L3DS::ReadMap(const LChunk &chunk, LMap& map)
{
	LChunk child;
	char str[20];
	GotoChunk(chunk);
	child = ReadChunk();
	while (child.end <= chunk.end)
	{
		switch (child.id)
		{
		case INT_PERCENTAGE:
			map.strength = ReadPercentage(child);
			break;
		case MAT_MAPNAME:
			ReadASCIIZ(str, 20);
			strcpy(map.mapName, str);
			break;
		case MAT_MAP_USCALE:
			map.uScale = ReadFloat();
			break;
		case MAT_MAP_VSCALE:
			map.vScale = ReadFloat();
			break;
		case MAT_MAP_UOFFSET:
			map.uOffset = ReadFloat();
			break;
		case MAT_MAP_VOFFSET:
			map.vOffset = ReadFloat();
			break;
		case MAT_MAP_ANG:
			map.angle = ReadFloat();
			break;
		}
		SkipChunk(child);
		child = ReadChunk();
	}
}

void L3DS::ReadKeyframeData(const LChunk &parent)
{
	uint frames = 0;

	LChunk node_hdr;
	node_hdr.id = NODE_HDR;

	char str[20];
	LMesh *mesh;

	GotoChunk(parent);
	if (!FindChunk(node_hdr, parent))
		return;
	GotoChunk(node_hdr);
	ReadASCIIZ(str, 19);
	mesh = FindMesh(str);
	if (mesh == 0)
		return;
	GotoChunk(parent);

	// read the pivot
	LVector3 pivot = zero3;

	LChunk pivotchunk;
	pivotchunk.id = PIVOT;
	if (FindChunk(pivotchunk, parent))
	{
		GotoChunk(pivotchunk);
		pivot.x = ReadFloat();
		pivot.y = ReadFloat();
		pivot.z = ReadFloat();
	}
	GotoChunk(parent);

	// read frame 0 from the position track
	LVector3 pos = zero3;

	frames = 0;

	LChunk poschunk;
	poschunk.id = POS_TRACK_TAG;
	if (FindChunk(poschunk, parent))
	{
		GotoChunk(poschunk);
		// read the trackheader structure
		ReadShort();
		ReadInt();
		ReadInt();
		frames = ReadInt();
		if (frames > 0)
		{
			ReadKeyheader();
			pos.x = ReadFloat();
			pos.y = ReadFloat();
			pos.z = ReadFloat();
		}
	}
	GotoChunk(parent);

	// now read the rotation track
	LVector4 rot = zero4;

	LChunk rotchunk;
	rotchunk.id = ROT_TRACK_TAG;

	frames = 0;
	if (FindChunk(rotchunk, parent))
	{
		GotoChunk(rotchunk);
		// read the trackheader structure
		ReadShort();
		ReadInt();
		ReadInt();
		frames = ReadInt();
		if (frames > 0)
		{
			ReadKeyheader();
			rot.x = ReadFloat();
			rot.y = ReadFloat();
			rot.z = ReadFloat();
			rot.w = ReadFloat();
		}
	}
	GotoChunk(parent);

	// now read the scaling chunk
	LVector3 scale;
	scale.x = 1;
	scale.y = 1;
	scale.z = 1;

	LChunk scalechunk;
	scalechunk.id = SCL_TRACK_TAG;

	frames = 0;

	if (FindChunk(scalechunk, parent))
	{
		GotoChunk(scalechunk);
		// read the trackheader structure
		ReadShort();
		ReadInt();
		ReadInt();
		frames = ReadInt();
		if (frames > 0)
		{
			ReadKeyheader();
			scale.x = ReadFloat();
			scale.y = ReadFloat();
			scale.z = ReadFloat();
		}
	}
	GotoChunk(parent);
}

long L3DS::ReadKeyheader()
{
	long frame;
	frame = ReadInt();
	short opts = ReadShort();
	if (opts & 32768) // 32768 is 1000000000000000 binary
	{  // tension is present
		ReadFloat();
	}
	if (opts & 16384) // 16384 is 0100000000000000 binary
	{  // continuity is present
		ReadFloat();
	}
	if (opts & 8192)
	{  // bias info present
		ReadFloat();
	}
	if (opts & 4096)
	{  // "ease to" present
		ReadFloat();
	}
	if (opts & 2048)
	{  // "ease from" present
		ReadFloat();
	}
	return frame;
}
