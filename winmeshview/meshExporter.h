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

#ifndef _MESH_EXPORTER_H_
#define _MESH_EXPORTER_H_

#include "CMesh.h"
#include "meshImporter.h"

// A function used to pop up a dialog box and ask the user to select
// a file (for selecting game files) (last five parameters are optional).
int FileBrowse(char* buffer, int length, int save = 0, char* forceExtension = 0,
	char* extension_string = 0, char* title = 0, int* chosenFilter = 0);

// Lets calling classes insert data before or after the main file contents
class ExportHelper {
public:
	virtual void preExport(FILE* f, int filetype) {};
	virtual void postExport(FILE* f, int filetype) {};

	int m_nTets;
	int m_nBadTets;
};

void getShortFilename(char* dest, const char* source);

typedef enum {
	FILETYPE_NODE = 0, FILETYPE_ANODE, FILETYPE_SMESH, FILETYPE_OBJ,
	FILETYPE_PLY_ASCII, FILETYPE_PLY_BINARY, FILETYPE_ABAQUS_INP, FILETYPE_INVALID
} output_filetypes;

extern const char* mesh_export_extensions[];

// If 'xform' is supplied, it should contain the transformation that
// was applied to this model when it was loaded.  I.e., the _reverse_
// of xform will be applied during export.
int ExportModel(cMesh* object, const char* in_filename = 0,
	const mesh_xform_information* xform = 0, const char* original_filename = 0,
	ExportHelper* export_helper = 0);

#endif