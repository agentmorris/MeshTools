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
	\author:    Emanuele Ruffaldi
	\version    1.0
	\date       01/2006
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CShaders.h"
#include "CImageLoader.h"
#include "CVBOMesh.h"

// Macros for safe pointer deletion
#define SAFE_DELETE(p) { if(p) { delete (p); (p)=NULL; } }
#define SAFE_ARRAY_DELETE(p) { if(p) { delete [] p; (p)=NULL; } }

// If defined, debugging output related to shader is sent to the console
#define DEBUG_SHADER_OUTPUT


//===========================================================================
/*!
	Constructor of cGenericShader.

	\fn         cGenericShader::cGenericShader()
*/
//===========================================================================
cGenericShader::cGenericShader()
{
	m_fragmentShaderInitialized = false;
	m_vertexShaderInitialized = false;
	m_vertexShaderFilename = 0;
	m_vertexShaderString = 0;
	m_fragmentShaderFilename = 0;
	m_fragmentShaderString = 0;
	m_shadingEnabled = true;
}


//===========================================================================
/*!
	Destructor of cGenericShader.

	\fn         cGenericShader::~cGenericShader()
*/
//===========================================================================
cGenericShader::~cGenericShader()
{
	uninitializeShaders();
	SAFE_ARRAY_DELETE(m_fragmentShaderFilename);
	SAFE_ARRAY_DELETE(m_fragmentShaderString);
	SAFE_ARRAY_DELETE(m_vertexShaderFilename);
	SAFE_ARRAY_DELETE(m_vertexShaderString);
}


//===========================================================================
/*!
	Called to clean up shader data; just calls the uninitialization methods
	for vertex and fragment shaders.

	\fn         void cGenericShader::uninitializeShaders()
*/
//===========================================================================
void cGenericShader::uninitializeShaders()
{
	uninitializeVertexShader();
	uninitializeFragmentShader();
}


//===========================================================================
/*!
	Called to initialize shader data; just calls the initialization methods
	for vertex and fragment shaders.

	\fn         void cGenericShader::initializeShaders()
*/
//===========================================================================
void cGenericShader::initializeShaders()
{
	initializeVertexShader();
	initializeFragmentShader();
}


//===========================================================================
/*!
	Users should call this method to load a vertex shader from a file.

	\fn         int cGenericShader::loadVertexShaderFromFile(const char* a_filename)
	\param      a_filename    The file to load.
	\return     >=0 for success, <0 for error
*/
//===========================================================================
int cGenericShader::loadVertexShaderFromFile(const char* a_filename)
{
	char* contents = (char*)readFile(a_filename, true);
	if (contents == 0) return -1;

	uninitializeVertexShader();
	SAFE_ARRAY_DELETE(m_vertexShaderFilename);
	SAFE_ARRAY_DELETE(m_vertexShaderString);

	m_vertexShaderFilename = new char[strlen(a_filename) + 1];
	strcpy(m_vertexShaderFilename, a_filename);
	m_vertexShaderString = contents;

	int retval = initializeVertexShader();

	// Don't necessarily delete; we may try again at render time...

	return retval;
}


//===========================================================================
/*!
	Users should call this method to load a fragment shader from a file.

	\fn         int cGenericShader::loadFragmentShaderFromFile(const char* a_filename)
	\param      a_filename    The file to load.
	\return     >=0 for success, <0 for error
*/
//===========================================================================
int cGenericShader::loadFragmentShaderFromFile(const char* a_filename)
{
	char* contents = (char*)readFile(a_filename, true);
	if (contents == 0) return -1;

	uninitializeFragmentShader();
	SAFE_ARRAY_DELETE(m_fragmentShaderFilename);
	SAFE_ARRAY_DELETE(m_fragmentShaderString);

	m_fragmentShaderFilename = new char[strlen(a_filename) + 1];
	strcpy(m_fragmentShaderFilename, a_filename);
	m_fragmentShaderString = contents;

	int retval = initializeFragmentShader();

	// Don't necessarily delete; we may try again at render time...

	return retval;
}


//===========================================================================
/*!
Users should call this method to create a fragment shader from a text string.

\fn         int cGenericShader::loadFragmentShaderFromText(const char* a_shaderText)
\param      a_shaderText    The contents of the shader to create and compile.
\return     >=0 for success, <0 for error
*/
//===========================================================================
int cGenericShader::loadFragmentShaderFromText(const char* a_shaderText)
{
	uninitializeFragmentShader();
	SAFE_ARRAY_DELETE(m_fragmentShaderFilename);
	SAFE_ARRAY_DELETE(m_fragmentShaderString);

	m_fragmentShaderString = new char[strlen(a_shaderText) + 1];
	strcpy(m_fragmentShaderString, a_shaderText);

	int retval = initializeFragmentShader();

	// Don't necessarily delete; we may try again at render time...

	return retval;
}


//===========================================================================
/*!
Users should call this method to create a vertex shader from a text string.

\fn         int cGenericShader::loadVertexShaderFromText(const char* a_shaderText)
\param      a_shaderText    The contents of the shader to create and compile.
\return     >=0 for success, <0 for error
*/
//===========================================================================
int cGenericShader::loadVertexShaderFromText(const char* a_shaderText)
{
	uninitializeVertexShader();
	SAFE_ARRAY_DELETE(m_vertexShaderFilename);
	SAFE_ARRAY_DELETE(m_vertexShaderString);

	m_vertexShaderString = new char[strlen(a_shaderText) + 1];
	strcpy(m_vertexShaderString, a_shaderText);

	int retval = initializeVertexShader();

	// Don't necessarily delete; we may try again at render time...

	return retval;
}


//===========================================================================
/*!
	Renders the scene graph starting at this object; see cGenericObject.cpp for
	more details.

	This version of the function enables the relevant shader before rendering
	and disables it afterwards.

	\fn         void cGenericShader::renderSceneGraph(const int a_renderMode)
	\param      a_renderMode    The current rendering pass; see cGenericObject.cpp
*/
//===========================================================================
void cGenericShader::renderSceneGraph(const int a_renderMode)
{
	if (m_shadingEnabled) enableShaders();
	cGenericObject::renderSceneGraph(a_renderMode);
	if (m_shadingEnabled) disableShaders();
}


//===========================================================================
/*!
	Called when the OpenGL context has been re-initialized; see cGenericObject.cpp
	for details.

	This function is responsible for un-initializing and re-initializing shader
	data.

	\fn         void cGenericShader::onDisplayReset(const bool a_affectChildren)
	\param      a_affectChildren    Should this call be recursively applied to my children?
*/
//===========================================================================
void cGenericShader::onDisplayReset(const bool a_affectChildren)
{
	uninitializeShaders();
	initializeShaders();
	cGenericObject::onDisplayReset(a_affectChildren);
}


/******************
cGLSLShader
******************/

// GL extension function pointers
PFNGLCREATESHADEROBJECTARBPROC    glCreateShaderObjectARB = 0;
PFNGLDETACHOBJECTARBPROC          glDetachObjectARB = 0;
PFNGLDELETEOBJECTARBPROC          glDeleteObjectARB = 0;
PFNGLCREATEPROGRAMOBJECTARBPROC   glCreateProgramObjectARB = 0;
PFNGLSHADERSOURCEARBPROC          glShaderSourceARB = 0;
PFNGLCOMPILESHADERARBPROC         glCompileShaderARB = 0;
PFNGLATTACHOBJECTARBPROC          glAttachObjectARB = 0;
PFNGLLINKPROGRAMARBPROC           glLinkProgramARB = 0;
PFNGLUSEPROGRAMOBJECTARBPROC      glUseProgramObjectARB = 0;
PFNGLGETOBJECTPARAMETERIVARBPROC  glGetObjectParameterivARB = 0;
PFNGLGETOBJECTPARAMETERFVARBPROC  glGetObjectParameterfvARB = 0;
PFNGLGETINFOLOGARBPROC            glGetInfoLogARB = 0;

//! Have we successfully initialized shader support?
static bool shader_support_initialized = false;


//===========================================================================
/*!
	A global function used to initialize shader support

\fn         bool InitShaderSupport()
\return     true if successful, false if vbo's are not supported or could not
			be initialized.
*/
//===========================================================================
bool InitShaderSupport()
{

	if (shader_support_initialized) return true;

	// Ask whether these extensions are supported...
	bool supported =
		IsExtensionSupported("GL_ARB_fragment_shader") &&
		IsExtensionSupported("GL_ARB_vertex_shader") &&
		IsExtensionSupported("GL_ARB_shader_objects");

	if (supported == false) {
#ifdef DEBUG_SHADER_OUTPUT
		CHAI_DEBUG_PRINT("Shader extensions not supported\n");
#endif
		return false;
	}

	// Get proc addresses for all the relevant extension functions
	glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
	glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)wglGetProcAddress("glDetachObjectARB");
	glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
	glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
	glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
	glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
	glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
	glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
	glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
	glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
	glGetObjectParameterfvARB = (PFNGLGETOBJECTPARAMETERFVARBPROC)wglGetProcAddress("glGetObjectParameterfvARB");
	glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");

	if (!glCreateShaderObjectARB || !glDetachObjectARB || !glDeleteObjectARB ||
		!glCreateProgramObjectARB || !glShaderSourceARB || !glCompileShaderARB ||
		!glAttachObjectARB || !glLinkProgramARB || !glUseProgramObjectARB ||
		!glGetObjectParameterivARB || !glGetObjectParameterfvARB || !glGetInfoLogARB) {
#ifdef DEBUG_VBO_OUTPUT
		CHAI_DEBUG_PRINT("Could not initialize shader support...\n";
#endif
		return false;
	}

#ifdef DEBUG_VBO_OUTPUT		
	CHAI_DEBUG_PRINT("Initialized shader support...\n");
#endif
	shader_support_initialized = true;

	return true;
}



//===========================================================================
/*!
	Print debugging information about a shader or program object to the console

	\fn         void static printShaderLog(GLhandleARB obj)
	\param      obj   The shader or program object about which output should
					  be printed
*/
//===========================================================================
void static printShaderLog(GLhandleARB obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
		&infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
		if (charsWritten != 0)
			CHAI_DEBUG_PRINT("%s\n", infoLog);
		free(infoLog);
	}
}


//===========================================================================
/*!
	Constructor of cGLSLShader

	\fn         cGLSLShader::cGLSLShader()
*/
//===========================================================================
cGLSLShader::cGLSLShader()
{
	m_programInitialized = false;
	m_hFShader = m_hVShader = m_hPObject = 0;
}


//===========================================================================
/*!
	Destructor of cGLSLShader

	\fn         cGLSLShader::~cGLSLShader()
*/
//===========================================================================
cGLSLShader::~cGLSLShader()
{
	uninitializeShaders();
}


//===========================================================================
/*!
	Uninitializes the ARB program object associated w/this shader

	\fn         void cGLSLShader::uninitializeProgram()
*/
//===========================================================================
void cGLSLShader::uninitializeProgram()
{
	if (m_hPObject != 0)
	{
		glDetachObjectARB(m_hPObject, m_hVShader);
		glDetachObjectARB(m_hPObject, m_hFShader);
		glDeleteObjectARB(m_hPObject);
		m_hPObject = 0;
	}
	m_programInitialized = false;
}


//===========================================================================
/*!
	Uninitializes the ARB handles associated w/this object by calling
	the uninit methods for the program, fragment, and vertex handles

	\fn         void cGLSLShader::uninitializeShaders()
*/
//===========================================================================
void cGLSLShader::uninitializeShaders()
{
	uninitializeProgram();
	uninitializeFragmentShader();
	uninitializeVertexShader();
}


//===========================================================================
/*!
	Uninitializes the ARB fragment shader handle associated w/this object

	\fn         void cGLSLShader::uninitializeFragmentShader()
	\return     >=0 for success, <0 for error
*/
//===========================================================================
int cGLSLShader::uninitializeFragmentShader()
{
	if (m_programInitialized) uninitializeProgram();

	if (m_hFShader != 0)
	{
		glDeleteObjectARB(m_hFShader);
		m_hFShader = 0;
	}
	m_fragmentShaderInitialized = false;
	return 0;
}


//===========================================================================
/*!
	Uninitializes the ARB vertex shader handle associated w/this object

	\fn         void cGLSLShader::uninitializeVertexShader()
	\return     >=0 for success, <0 for error
*/
//===========================================================================
int cGLSLShader::uninitializeVertexShader()
{
	if (m_programInitialized) uninitializeProgram();

	if (m_hVShader != 0)
	{
		glDeleteObjectARB(m_hVShader);
		m_hVShader = 0;
	}
	m_vertexShaderInitialized = 0;
	return 0;
}


//===========================================================================
/*!
	Initializes the ARB program object associated w/this shader

	\fn         void cGLSLShader::initializeProgram()
	\return     >=0 for success, <0 for error
*/
//===========================================================================
int cGLSLShader::initializeProgram()
{

	uninitializeProgram();

	// We currently require both a v and f shader
	// if (m_vertexShaderInitialized == false || m_fragmentShaderInitialized == false) return -1;

	m_hPObject = glCreateProgramObjectARB();

	if (m_fragmentShaderInitialized && m_hFShader)
		glAttachObjectARB(m_hPObject, m_hFShader);

	if (m_vertexShaderInitialized && m_hVShader)
		glAttachObjectARB(m_hPObject, m_hVShader);

	glLinkProgramARB(m_hPObject);

#ifdef DEBUG_SHADER_OUTPUT
	printShaderLog(m_hPObject);
#endif

	m_programInitialized = true;

	return 0;
}


//===========================================================================
/*!
	Initializes the ARB fragment shader handle associated w/this shader

	\fn         void cGLSLShader::initializeFragmentShader()
	\return     >=0 for success, <0 for error
*/
//===========================================================================
int cGLSLShader::initializeFragmentShader()
{
	if (shader_support_initialized == false) InitShaderSupport();
	if (shader_support_initialized == false) return -1;
	if (m_fragmentShaderString == 0 || strlen(m_fragmentShaderString) == 0) return -1;

	uninitializeFragmentShader();

	// Create a shader
	m_hFShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	const GLcharARB* c = (const GLcharARB*)(m_fragmentShaderString);

	// Give him my shader source
	glShaderSourceARB(m_hFShader, 1, &c, 0);

	// Compile the shader
	glCompileShaderARB(m_hFShader);

	m_fragmentShaderInitialized = true;

#ifdef DEBUG_SHADER_OUTPUT
	printShaderLog(m_hFShader);
#endif

	// Compile the program
	int program_result = initializeProgram();

	return program_result;
}


//===========================================================================
/*!
	Initializes the ARB vertex shader handle associated w/this shader

	\fn         void cGLSLShader::initializeVertexShader()
	\return     >=0 for success, <0 for error
*/
//===========================================================================
int cGLSLShader::initializeVertexShader()
{
	if (shader_support_initialized == false) InitShaderSupport();
	if (shader_support_initialized == false) return -1;
	if (m_vertexShaderString == 0 || strlen(m_vertexShaderString) == 0) return -1;

	uninitializeVertexShader();

	// Create a shader
	m_hVShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

	const GLcharARB* c = (const GLcharARB*)(m_vertexShaderString);

	// Give him my shader source
	glShaderSourceARB(m_hVShader, 1, &c, 0);

	// Compile the shader
	glCompileShaderARB(m_hVShader);

#ifdef DEBUG_SHADER_OUTPUT
	printShaderLog(m_hVShader);
#endif

	m_vertexShaderInitialized = true;

	initializeProgram();

	return 0;
}


//! Called at the beginning of a rendering pass to enable shaders, must be
//! over-ridden by subclasses
void cGLSLShader::enableShaders()
{
	if (m_hPObject != 0) glUseProgramObjectARB(m_hPObject);
}


//! Called at the end of a rendering pass to disable shaders, must be
//! over-ridden by subclasses
void cGLSLShader::disableShaders()
{
	if (m_hPObject != 0) glUseProgramObjectARB(0);
}

