/******
*
* Written by Dan Morris
* dmorris@cs.stanford.edu
* http://cs.stanford.edu/~dmorris
*
* You can do anything you want with this file as long as this header
* stays on it and I am credited when it's appropriate.
*
******/

#include "StdAfx.h"
#include "meshExporter.h"
#include "CVertex.h"
#include "CTriangle.h"

const char* mesh_export_extensions[] = {
  "node","anode","smesh","obj","ply","ply","inp"
};


bool IsTetValid(cVector3d vertices[4], int tetNumber, bool& verySmall) {

	// Find the mean of vertices 0,1,2
	cVector3d mean = (vertices[0] + vertices[1] + vertices[2]) / 3.0;

	// Find a vector from the face center to the other vertex
	cVector3d face_to_top = vertices[3] - mean;

	// Find the face normal of the triangle formed by vertices 0,1,2
	cVector3d v01 = vertices[1] - vertices[0];
	cVector3d v02 = vertices[2] - vertices[0];
	cVector3d n = cCross(v01, v02);

	// Dot them
	double d = cDot(n, face_to_top);

	// Are they pointed away from each other?
	bool valid_by_dot = (d > 0);

	// Compute _signed_ tet volume
	double v = (1.0 / 6.0) *
		(vertices[1] - vertices[0]).dot(
		((vertices[2] - vertices[0]).crossAndReturn(vertices[3] - vertices[0]))
		);

	if (fabs(v) < SMALL_TET_VOLUME) {
		_cprintf("Warning: very small tet volume %lf for tet %d\n", v, tetNumber);
		verySmall = true;
	}
	else verySmall = false;

	bool valid_by_cross = (v > 0);

	if (valid_by_cross != valid_by_dot) {
		_cprintf("Warning: tet %d, valid_by_cross is %d and valid_by_dot is %d\n",
			tetNumber, (int)valid_by_cross, (int)valid_by_dot);
	}

	return (valid_by_dot && valid_by_cross);

}


int ExportModel(cMesh* object, const char* in_filename,
	const mesh_xform_information* xform,
	const char* original_filename, ExportHelper* helper) {

	char filename[_MAX_PATH];

	int chosenFilter = -1;

	if (in_filename == 0) {

		// Extensions need to be in the same order as the filetypes
		int result = FileBrowse(filename, _MAX_PATH, 1, 0,
			"tetgen surface (*.node, *.face)|*.node|tetgen nodes only (*.anode)|*.anode|tetgen PLC (*.smesh)|*.smesh|alias obj file (*.obj)|*.obj|ply (ascii)|*.ply|ply (binary)|*.ply|ABAQUS input file (*.inp)|*.inp|All Files (*.*)|*.*||",
			"Choose a face file for export", &chosenFilter);

		if (result == -1) return -1;
	}
	else {
		strcpy(filename, in_filename);
	}

	// Build the node and face filenames
	char* extension = find_extension(filename);

	// If we didn't find an extension, see whether the user
	// specified a file type in the dialog box...
	if (extension == 0) {

		if (chosenFilter < 0 || chosenFilter >= FILETYPE_INVALID) {
			_cprintf("Could not determine requested file type for filename %s...\n", filename);
			return -1;
		}

		else {
			strcat(filename, ".");
			strcat(filename, mesh_export_extensions[chosenFilter]);
		}

		extension = find_extension(filename);
	}

	_cprintf("Chose filter %d\n", chosenFilter);

	int filetype = -1;
	if (strcmp(extension, "node") == 0) filetype = FILETYPE_NODE;
	else if (strcmp(extension, "anode") == 0) filetype = FILETYPE_ANODE;
	else if (strcmp(extension, "smesh") == 0) filetype = FILETYPE_SMESH;
	else if (strcmp(extension, "obj") == 0) filetype = FILETYPE_OBJ;
	else if (strcmp(extension, "ply") == 0) {
		if (chosenFilter == FILETYPE_PLY_ASCII) filetype = FILETYPE_PLY_ASCII;
		else if (chosenFilter == FILETYPE_PLY_BINARY) filetype = FILETYPE_PLY_BINARY;
		else {
			_cprintf("Warning: ply extension but no ply filter chosen...\n");
		}
	}
	else if (strcmp(extension, "inp") == 0) filetype = FILETYPE_ABAQUS_INP;

	if (filetype == -1) {
		_cprintf("Could not determine requested file type...\n");
		return -1;
	}

	char node_filename[_MAX_PATH];
	char face_filename[_MAX_PATH];
	char smesh_filename[_MAX_PATH];
	char obj_filename[_MAX_PATH];
	char obj_mtl_filename[_MAX_PATH];
	char ply_filename[_MAX_PATH];
	char inp_filename[_MAX_PATH];

	// Node filename is used for node or anode files
	strcpy(extension, filetype == FILETYPE_NODE ? "node" : "anode");
	strcpy(node_filename, filename);
	strcpy(extension, "face");
	strcpy(face_filename, filename);
	strcpy(extension, "smesh");
	strcpy(smesh_filename, filename);
	strcpy(extension, "obj");
	strcpy(obj_filename, filename);
	strcpy(extension, "mtl");
	strcpy(obj_mtl_filename, filename);
	strcpy(extension, "ply");
	strcpy(ply_filename, filename);
	strcpy(extension, "inp");
	strcpy(inp_filename, filename);

	FILE* nodef = 0;
	FILE* facef = 0;
	FILE* smeshf = 0;
	FILE* objf = 0;
	FILE* obj_mtl_f = 0;
	FILE* plyf = 0;
	FILE* inpf = 0;

	int curMaterialIndex = 0;

	if (filetype == FILETYPE_NODE) {
		_cprintf("Writing faces to %s\n", face_filename);
		_cprintf("Writing vertices to %s\n", node_filename);
	}
	if (filetype == FILETYPE_ANODE) {
		_cprintf("Writing vertices (only) to %s\n", node_filename);
	}
	else if (filetype == FILETYPE_SMESH) _cprintf("Writing smesh to %s\n", smesh_filename);
	else if (filetype == FILETYPE_OBJ) _cprintf("Writing obj to %s\n", obj_filename);
	else if (filetype == FILETYPE_PLY_BINARY || filetype == FILETYPE_PLY_ASCII)
		_cprintf("Writing ply (%s) to %s\n",
			filetype == FILETYPE_PLY_BINARY ? "binary" : "ascii", ply_filename);

	if (filetype == FILETYPE_NODE || filetype == FILETYPE_ANODE) {
		nodef = fopen(node_filename, "wb");

		if (nodef == 0) {
			_cprintf("Could not open output node file %s\n", node_filename);
			return -1;
		}

		// Don't open a face file for anode files
		if (filetype == FILETYPE_NODE) {
			facef = fopen(face_filename, "wb");

			if (facef == 0) {
				_cprintf("Could not open output face file %s\n", face_filename);
				_cprintf("\nWarning: not deleting node file %s...\n\n", node_filename);
				fclose(nodef);
				return -1;
			}
		}
	}

	else if (filetype == FILETYPE_SMESH) {

		smeshf = fopen(smesh_filename, "wb");

		if (smeshf == 0) {
			_cprintf("Could not open output smesh file %s\n", smesh_filename);
			return -1;
		}

	}

	else if (filetype == FILETYPE_OBJ) {
		objf = fopen(obj_filename, "wb");

		if (objf == 0) {
			_cprintf("Could not open output obj file %s\n", obj_filename);
			return -1;
		}

		obj_mtl_f = fopen(obj_mtl_filename, "wb");

		if (obj_mtl_f == 0) {
			_cprintf("Warning: could not open material file %s\n", obj_mtl_filename);
		}

	}

	else if (filetype == FILETYPE_PLY_ASCII || filetype == FILETYPE_PLY_BINARY) {

		plyf = fopen(ply_filename, "wb");

		if (plyf == 0) {
			_cprintf("Could not open output ply file %s\n", ply_filename);
			return -1;
		}

	}

	else if (filetype == FILETYPE_ABAQUS_INP) {

		inpf = fopen(inp_filename, "wb");

		if (inpf == 0) {
			_cprintf("Could not open output abaqus inp file %s\n", inp_filename);
		}

	}

	std::list<cGenericObject*> meshes;
	object->enumerateChildren(meshes);

	std::list<cGenericObject*>::iterator iter;
	unsigned int nMeshes = 0;
	unsigned int nVertices = 0;
	unsigned int nFaces = 0;
	unsigned int nTets = 0;

	for (iter = meshes.begin(); iter != meshes.end(); iter++) {
		cMesh* curMesh = dynamic_cast<cMesh*>(*iter);
		if (curMesh) {
			nMeshes++;
			nVertices += curMesh->getNumVertices(false);
			nFaces += curMesh->getNumTriangles(false);
			cTetMesh* ctm = dynamic_cast<cTetMesh*>(*iter);
			if (ctm) nTets += ctm->m_nTets;
		}
	}

	_cprintf("Exporting %u vertices and %u triangles (and maybe %u tets) from %u meshes\n",
		nVertices, nFaces, nTets, nMeshes);

	// Okay, this is getting silly...
	FILE* f;
	switch (filetype) {
	case FILETYPE_NODE:
	case FILETYPE_ANODE:
		f = nodef; break;
	case FILETYPE_SMESH:
		f = smeshf; break;
	case FILETYPE_PLY_BINARY:
	case FILETYPE_PLY_ASCII:
		f = plyf; break;
	case FILETYPE_ABAQUS_INP:
		f = inpf; break;
	case FILETYPE_OBJ:
		f = objf; break;
	default:
		f = 0;
	}

	// Files have now been opened
	if (helper && f) helper->preExport(f, filetype);

#define NUM_NODE_BOUNDARY_MARKERS 0
#define NUM_FACE_BOUNDARY_MARKERS 0
#define DEFAULT_NODE_BOUNDARY_MARKER 1

	// Set up file headers
	int nDim = 3;
	int nAttributes = 0;
	int nNodeBoundaryMarkers = NUM_NODE_BOUNDARY_MARKERS;
	int nFaceBoundaryMarkers = NUM_FACE_BOUNDARY_MARKERS;

	if (nodef) {
		if (original_filename && filetype == FILETYPE_ANODE) {
			//char short_original_filename[_MAX_PATH];
			//find_filename(short_original_filename,original_filename,false);
			fprintf(nodef, "# BASE_FILENAME %s\n", original_filename);
		}
		fprintf(nodef, "# Generated by WinMeshView ( http://cs.stanford.edu/~dmorris/projects/winmeshview )\n");
		fprintf(nodef, "%d %d %d %d\n",
			nVertices, nDim, nAttributes, nNodeBoundaryMarkers);
	}

	if (facef) {
		fprintf(facef, "# Generated by WinMeshView ( http://cs.stanford.edu/~dmorris/projects/winmeshview )\n");
		fprintf(facef, "%d %d\n", nFaces, nFaceBoundaryMarkers);
	}

	if (objf) {
		fprintf(objf, "# Generated by WinMeshView ( http://cs.stanford.edu/~dmorris/projects/winmeshview )\n");
		char obj_mtl_reference_name[_MAX_PATH];
		getShortFilename(obj_mtl_reference_name, obj_mtl_filename);
		_cprintf("Writing mtl filename %s\n", obj_mtl_reference_name);
		fprintf(objf, "mtllib %s\n", obj_mtl_reference_name);
		if (obj_mtl_f)
			fprintf(obj_mtl_f, "# Generated by WinMeshView ( http://cs.stanford.edu/~dmorris/projects/winmeshview )\n");
	}

	if (plyf) {
		fprintf(plyf, "ply\n");
		fprintf(plyf, "format %s 1.0\n", filetype == FILETYPE_PLY_ASCII ? "ascii" : "binary_little_endian");
		fprintf(plyf, "comment Generated by WinMeshView ( http://cs.stanford.edu/~dmorris/projects/winmeshview )\n");
		fprintf(plyf, "element vertex %d\n", nVertices);
		fprintf(plyf, "property float x\nproperty float y\nproperty float z\n");
		fprintf(plyf, "element face %d\n", nFaces);
		//fprintf(plyf,"property int face_index\n");
		fprintf(plyf, "property list uchar int vertex_index\n");
		fprintf(plyf, "end_header\n");
	}

	if (inpf) {
		fprintf(inpf, "*Heading\n");
		fprintf(inpf, "** Generated by WinMeshView ( http://cs.stanford.edu/~dmorris/projects/winmeshview )\n");
		fprintf(inpf, "*Preprint, echo=NO, model=NO, history=NO, contact=NO\n");
		fprintf(inpf, "*Part, name=winmeshview_exported_part\n");
	}

#define PASS_WRITE_VERTICES 0
#define PASS_WRITE_FACES    1
#define PASS_WRITE_TETS     2

	int nodeBoundaryMarker = DEFAULT_NODE_BOUNDARY_MARKER;

	int nBadTets = 0;

	// We do two whole passes over the set of meshes, because
	// the .smesh format doesn't like mixing nodes and faces
	for (int curPass = PASS_WRITE_VERTICES; curPass <= PASS_WRITE_TETS; curPass++) {

		// Only the abaqus file format supports tets right now...
		if (filetype != FILETYPE_ABAQUS_INP && curPass == PASS_WRITE_TETS) continue;

		// If we are using the abaqus format, we do _either_ the face pass or the
		// tet pass...
		if (filetype == FILETYPE_ABAQUS_INP) {
			if (curPass == PASS_WRITE_FACES && nTets > 0) continue;
			if (curPass == PASS_WRITE_TETS && nTets == 0) continue;
		}
		if (filetype == FILETYPE_ANODE && curPass != PASS_WRITE_VERTICES)
			break;

		// Set up smesh headers
		if (smeshf) {
			if (curPass == PASS_WRITE_VERTICES) {
				fprintf(smeshf, "# Generated by WinMeshView ( http://cs.stanford.edu/~dmorris/projects/winmeshview/ )\n");
				fprintf(smeshf, "%d %d %d %d\n",
					nVertices, nDim, nAttributes, nNodeBoundaryMarkers);
			}
			else if (curPass == PASS_WRITE_FACES) {
				fprintf(smeshf, "# Generated by WinMeshView ( http://cs.stanford.edu/~dmorris/projects/winmeshview/ )\n");
				fprintf(smeshf, "%d %d\n", nFaces, nFaceBoundaryMarkers);
			}
		}

		// Set up inp headers
		if (inpf) {
			if (curPass == PASS_WRITE_VERTICES) {
				fprintf(inpf, "** Writing out %d vertices\n", nVertices);
				fprintf(inpf, "*Node\n");
			}
			if (curPass == PASS_WRITE_FACES) {
				fprintf(inpf, "** Writing out %d faces\n", nFaces);
				fprintf(inpf, "*Element, type=SFM3D3\n");
			}
			if (curPass == PASS_WRITE_TETS) {
				fprintf(inpf, "** Writing out %d tets\n", nVertices);
				fprintf(inpf, "*Element, type=C3D4\n");
			}

		}

		unsigned int curPoint = 0;
		unsigned int curFace = 0;
		unsigned int curTet = 0;

		// Which point in the output file corresponds to vertex
		// zero in this mesh?
		unsigned int pointOffset = 0;

		// Which tet in the output file corresponds to tet zero in this mesh?
		unsigned int tetOffset = 0;

		// The current index for output meshes (used for tagging faces); incremented
		// when we find a mesh that _has_ faces
		int curMeshIndex = 0;

		// Loop over each mesh
		for (iter = meshes.begin(); iter != meshes.end(); iter++) {
			cMesh* curMesh = dynamic_cast<cMesh*>(*iter);

			// Only go on if this is a mesh...
			if (curMesh == 0) continue;

			pointOffset = curPoint;

			int local_nVertices = curMesh->getNumVertices(false);

			// Vertices only go anywhere on the first pass...
			if (curPass == PASS_WRITE_VERTICES) {

				// Write each of his vertices to the node file
				for (int i = 0; i < local_nVertices; i++) {
					cVertex* raw_v = curMesh->getVertex(i, false);
					cVertex* v;
					cVertex outv(0, 0, 0);

					if (xform == 0)
						v = raw_v;
					else {
						outv = *raw_v;
						v = &outv;

						// Scale and offset this vertex
						v->m_localPos /= xform->model_scale_factor;
						v->m_localPos -= xform->model_offset;
					}

					if (nodef) {
						fprintf(nodef, "%u %f %f %f",
							curPoint,
							(float)(v->getPos().x),
							(float)(v->getPos().y),
							(float)(v->getPos().z));

						if (nNodeBoundaryMarkers) fprintf(nodef, " %d", nodeBoundaryMarker);
						fprintf(nodef, "\n");
					}

					if (objf) {
						fprintf(objf, "v %f %f %f\n",
							(float)(v->getPos().x),
							(float)(v->getPos().y),
							(float)(v->getPos().z));

						fprintf(objf, "vt %f %f\n",
							(float)(v->getTexCoord().x),
							(float)(v->getTexCoord().y));

						fprintf(objf, "vn %f %f %f\n",
							(float)(v->getNormal().x),
							(float)(v->getNormal().y),
							(float)(v->getNormal().z));
					}

					if (smeshf) {
						fprintf(smeshf, "%u %f %f %f",
							curPoint,
							(float)(v->getPos().x),
							(float)(v->getPos().y),
							(float)(v->getPos().z));

						if (nNodeBoundaryMarkers) fprintf(smeshf, " %d", nodeBoundaryMarker);
						fprintf(smeshf, "\n");
					}

					if (plyf) {
						if (filetype == FILETYPE_PLY_ASCII) {
							fprintf(plyf, "%f %f %f\r\n",
								(float)(v->getPos().x),
								(float)(v->getPos().y),
								(float)(v->getPos().z));
						}
						else {
							float pos[3];
							pos[0] = (float)(v->getPos().x);
							pos[1] = (float)(v->getPos().y);
							pos[2] = (float)(v->getPos().z);
							fwrite(pos, sizeof(float), 3, plyf);
						}
					}

					if (inpf) {
						fprintf(inpf, "%u, %lf, %lf, %lf\n", curPoint + 1,
							(double)(v->getPos().x),
							(double)(v->getPos().y),
							(double)(v->getPos().z));
					}
					curPoint++;
				} // For each vertex

			} // if this is the vertex-only pass

			// We still need to keep track of offsets on the face-writing and tet-writing passes
			else {
				curPoint += local_nVertices;
			}

			if (curPass == PASS_WRITE_FACES) {

				int local_nFaces = curMesh->getNumTriangles(false);

				// If he _probably_ references another mesh for his vertices,
				// assume it's the top-level mesh...
				if (local_nFaces > 0 && local_nVertices == 0)
					pointOffset = 0;

				if (local_nFaces > 0) curMeshIndex++;

				// Write out a group/mesh name for this object before
				// writing any faces
				if (objf) {

					fprintf(objf, "g %s\n", curMesh->m_objectName);

					// And write the relevant material to the material file if necessary
					if (curMesh->getMaterialEnabled() && obj_mtl_f) {
						char material_name[100];
						_snprintf(material_name, 100, "mat%d", curMaterialIndex);
						curMaterialIndex++;
						fprintf(objf, "usemtl %s\n", material_name);
						fprintf(obj_mtl_f, "newmtl %s\n", material_name);
						cMaterial m = curMesh->m_material;
						fprintf(obj_mtl_f, "Ka %6.3f %6.3f %6.3f\n", m.m_ambient.getR(), m.m_ambient.getG(), m.m_ambient.getB());
						fprintf(obj_mtl_f, "Kd %6.3f %6.3f %6.3f\n", m.m_diffuse.getR(), m.m_diffuse.getG(), m.m_diffuse.getB());
						fprintf(obj_mtl_f, "Ks %6.3f %6.3f %6.3f\n", m.m_specular.getR(), m.m_specular.getG(), m.m_specular.getB());
						fprintf(obj_mtl_f, "Tr %6.3f\n", m.m_diffuse.getA());
						fprintf(obj_mtl_f, "Ns %6.3f\n", ((float)(m.m_shininess)) / 255.0);
						if (curMesh->getTextureEnabled() && curMesh->getTexture()) {
							char texfilename[_MAX_PATH];
							getShortFilename(texfilename, curMesh->getTexture()->m_image.getFilename());
							fprintf(obj_mtl_f, "map_Kd %s\n", texfilename);
						}
						fprintf(obj_mtl_f, "\n");
					}
				}

				for (int i = 0; i < local_nFaces; i++) {
					cTriangle* t = curMesh->getTriangle(i, false);

					if (facef) {
						fprintf(facef, "%u %u %u %u",
							curFace,
							t->getVertexIndex(0) + pointOffset,
							t->getVertexIndex(1) + pointOffset,
							t->getVertexIndex(2) + pointOffset);

						if (nFaceBoundaryMarkers) fprintf(facef, " %d", curMeshIndex);
						fprintf(facef, "\n");
					}

					if (objf) {
						fprintf(objf, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
							t->getVertexIndex(0) + pointOffset + 1,
							t->getVertexIndex(0) + pointOffset + 1,
							t->getVertexIndex(0) + pointOffset + 1,
							t->getVertexIndex(1) + pointOffset + 1,
							t->getVertexIndex(1) + pointOffset + 1,
							t->getVertexIndex(1) + pointOffset + 1,
							t->getVertexIndex(2) + pointOffset + 1,
							t->getVertexIndex(2) + pointOffset + 1,
							t->getVertexIndex(2) + pointOffset + 1);
					}

					if (smeshf) {
						fprintf(smeshf, "%d %u %u %u",
							3,
							t->getVertexIndex(0) + pointOffset,
							t->getVertexIndex(1) + pointOffset,
							t->getVertexIndex(2) + pointOffset);

						if (nFaceBoundaryMarkers) fprintf(smeshf, " %d", curMeshIndex);
						fprintf(smeshf, "\n");
					}

					if (plyf) {
						if (filetype == FILETYPE_PLY_ASCII) {
							fprintf(plyf, "3 %d %d %d",
								//curFace,
								t->getVertexIndex(0) + pointOffset,
								t->getVertexIndex(1) + pointOffset,
								t->getVertexIndex(2) + pointOffset);
							// A hack to handle an rply loading bug
							fprintf(plyf, "%s", curFace == nFaces - 1 ? " " : "\r\n");
						}
						else {
							//fwrite(&curFace,sizeof(int),1,plyf);
							unsigned char npts = 3;
							fwrite(&npts, sizeof(unsigned char), 1, plyf);
							int points[3];
							points[0] = t->getVertexIndex(0) + pointOffset;
							points[1] = t->getVertexIndex(1) + pointOffset;
							points[2] = t->getVertexIndex(2) + pointOffset;
							fwrite(points, sizeof(int), 3, plyf);
						}
					}

					if (inpf) {
						fprintf(inpf, "%u,%u,%u,%u\n", curFace + 1,
							t->getVertexIndex(0) + pointOffset + 1,
							t->getVertexIndex(1) + pointOffset + 1,
							t->getVertexIndex(2) + pointOffset + 1);
					}

					curFace++;
				} // For each triangle

			} // If this is the face-writing pass

			if (curPass == PASS_WRITE_TETS) {

				cTetMesh* ctm = dynamic_cast<cTetMesh*>(curMesh);

				if (ctm == 0) continue;

				int local_nTets = ctm->m_nTets;

				if (local_nTets == 0) continue;

				// If he _probably_ references another mesh for his vertices,
				// assume it's the top-level mesh...
				if (local_nVertices == 0) pointOffset = 0;

				for (int i = 0; i < local_nTets; i++) {

					unsigned int* t = ctm->m_tets + 4 * i;

					if (inpf) {

						// Tets need to be re-ordered to please Abaqus...

						// Grab all 4 vertices
						cVector3d vertices[4];
						for (int j = 0; j < 4; j++) {
							cVector3d v = ctm->getVertex(t[j], true)->getPos();
							vertices[j] = v;
						}

						/*
						// Used this to track down a particular tet...
						int search_vertices[3] = {267,113,241};
						// 240 v 241
						int found_vertices = 0;
						for(int s=0; s<4; s++) {
						  for(int u=0; u<4; u++) {
							if (t[s] == search_vertices[u]) {
							  found_vertices++;
							  continue;
							}
						  }
						}
						if (found_vertices == 3) {
						  _cprintf("Tet %d matches: %d,%d,%d,%d\n",i,t[0],t[1],t[2],t[3]);
						}
						*/

						bool verySmall = false;
						bool valid = (IsTetValid(vertices, i, verySmall));

						if (verySmall) {
							nBadTets++;
							_cprintf("Warning: tet %d is very small, so I'm skipping it\n", i);
							continue;
						}

						bool flip = false;

						if (valid == false) {

							flip = true;
							cVector3d tmp = vertices[0];
							vertices[0] = vertices[1];
							vertices[1] = tmp;

							if (i <= 10) _cprintf("Flipping an invalid tet %d\n", i);

							valid = (IsTetValid(vertices, i, verySmall));

							if (valid == false) {
								_cprintf("Warning: I swapped tet %d and it's still not valid\n", i);
							}
						}

						fprintf(inpf, "%u, %u, %u, %u, %d\n", i + 1 - nBadTets, t[flip ? 1 : 0] + 1, t[flip ? 0 : 1] + 1, t[2] + 1, t[3] + 1);
					}

				} // For each tet

			} // If this is the tet-writing pass

		} // For each mesh

	} // For each of two passes

	// Write the rest of the smesh stuff
	if (smeshf) {
		fprintf(smeshf, "# Holes\n%d\n", 0);
		fprintf(smeshf, "# Region attributes\n%d\n", 0);
	}

	if (inpf) {

		// This is all done in the export helper if one is available, to allow
		// for multiple sets...

		if (helper) {
			// the helper will handle this in postExport
		}

		// close up the part...
		else {

			// Define an element set that includes every tet
			fprintf(inpf, "*Elset, elset=WMV-ELSET, generate\n");
			fprintf(inpf, "1,%d,1\n", nTets - nBadTets);

			// Define a section to include that element set
			fprintf(inpf, "*Solid Section, elset=WMV-ELSET, material=WMV-MATERIAL\n");
			fprintf(inpf, "1.0,\n");

			fprintf(inpf, "*End part\n");

		}
	}

	if (helper && f) {
		helper->m_nTets = nTets;
		helper->m_nBadTets = nBadTets;
		helper->postExport(f, filetype);
	}

	// Clean up
	if (facef) fclose(facef);
	if (nodef) fclose(nodef);
	if (smeshf) fclose(smeshf);
	if (objf) fclose(objf);
	if (obj_mtl_f) fclose(obj_mtl_f);
	if (plyf) fclose(plyf);
	if (inpf) fclose(inpf);

	_cprintf("Finished file output...\n");

	return 0;
}


// A function used to pop up a dialog box and ask the user to select
// a file (for selecting game files) (last five parameters are optional).
int FileBrowse(char* buffer, int length, int save, char* forceExtension,
	char* extension_string, char* title, int* chosenFilter) {

	CFileDialog cfd(1 - save, 0, 0, 0, extension_string, 0);

	if (forceExtension != 0) {
		cfd.m_ofn.lpstrDefExt = forceExtension;
	}

	if (title != 0) {
		cfd.m_ofn.lpstrTitle = title;
	}

	if (save) {
		cfd.m_ofn.Flags |= OFN_OVERWRITEPROMPT;
	}

	// We don't want to change the working path
	cfd.m_ofn.Flags |= OFN_NOCHANGEDIR;

	if (cfd.DoModal() != IDOK) return -1;

	int chosenFilterIndex = cfd.m_ofn.nFilterIndex - 1;
	if (chosenFilter) *chosenFilter = chosenFilterIndex;

	CString result = cfd.GetPathName();

	// This should never happen, since I always pass in _MAX_PATH
	// for the length.
	if (result.GetLength() > length) {
		_cprintf("Warning: truncating filename to %d characters\n", length);
	}

	strncpy(buffer, (LPCSTR)(result), length);

	// strncpy needs help with the null termination
	buffer[length - 1] = '\0';

	return 0;
}

void getShortFilename(char* dest, const char* source) {

	const char* last_slash = source + strlen(source);
	while (
		(last_slash != source) &&
		((*last_slash) != '\\') &&
		((*last_slash) != '/')
		) {
		last_slash--;
	}
	if (last_slash != source && (*last_slash == '/' || *last_slash == '\\')) last_slash++;
	strcpy(dest, last_slash);
}