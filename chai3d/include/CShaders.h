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
#ifndef CGenericShaderH
#define CGenericShaderH
//---------------------------------------------------------------------------
#include "CGenericObject.h"
#include "glext.h"
#include <string>
/*!
    \file   CShaders.h
*/
//===========================================================================
/*!
      \class      cGenericShader
      \brief      This class defines a generic vertex/fragment shader, which will be
                  subclassed by particular implementations.  Shaders in CHAI are objects
                  in the scenegraph, whose children are rendered with this shader enabled.

                  Specific implementations can be found later in this file.
*/
//===========================================================================
class cGenericShader : public cGenericObject
{    

  public:

    //! Constructor of cGenericShader
    cGenericShader();

    //! Destructor of cGenericShader
    virtual ~cGenericShader(); 

    //! Load a vertex shader from a file
    virtual int loadVertexShaderFromFile(const char* a_filename);

    //! Load a fragment shader from a file
    virtual int loadFragmentShaderFromFile(const char* a_filename);

    //! Load a vertex shader from a text string
    virtual int loadVertexShaderFromText(const char* a_shaderText);

    //! Load a fragment shader from a text string
    virtual int loadFragmentShaderFromText(const char* a_shaderText);
   
    //! Enable the shader before rendering children, then render children, then disable 
    virtual void renderSceneGraph(const int a_renderMode=CHAI_RENDER_MODE_RENDER_ALL);

    //! This function should get called when it's necessary to re-initialize the OpenGL context
    virtual void onDisplayReset(const bool a_affectChildren = true);

    //! Used to enable and disable shading
    virtual void setShadingEnabled(const bool a_shadingEnabled) { m_shadingEnabled = a_shadingEnabled; }

    //! Is shading currently enabled?
    virtual bool getShadingEnabled() const { return m_shadingEnabled; }

	protected:

    //! Called on display context switches or when new shader text has been loaded
    virtual void initializeShaders();

    //! Called when we need to clean up shaders; e.g. when new shaders are being loaded
    virtual void uninitializeShaders();

    //! Called to create a fragment shader from m_fragmentShaderString
    virtual int initializeFragmentShader() { return -1; }

    //! Called to clean up a fragment shader
    virtual int uninitializeFragmentShader() { return 0; }

    //! Called to create a vertex shader from m_vertexShaderString
    virtual int initializeVertexShader() { return -1; }

    //! Called to clean up a vertex shader
    virtual int uninitializeVertexShader() { return 0; }

    //! Called at the beginning of a rendering pass to enable shaders
    virtual void enableShaders() = 0;

    //! Called at the end of a rendering pass to disable shaders, must be
    //! over-ridden by subclasses
    virtual void disableShaders() = 0;
    
    //! A string representing the vertex shader filename; empty if the shader was initialized from text
    char* m_vertexShaderFilename;

    //! A string representing the vertex shader itself
    char* m_vertexShaderString;

    //! A string representing the vertex shader filename; empty if the shader was initialized from text
    char* m_fragmentShaderFilename;

    //! A string representing the vertex shader itself
    char* m_fragmentShaderString;

    //! Have we initialized our shaders yet?
    bool m_fragmentShaderInitialized;
    bool m_vertexShaderInitialized;

    //! Is shading currently enabled?
    bool m_shadingEnabled;
    
};



//===========================================================================
/*!
    \class      cGLSLShader
    \brief      This class is a GLSL-specific implementation of cGenericShader
*/
//===========================================================================
class cGLSLShader : public cGenericShader
{    

public:

    //! Constructor of cGLSLShader
    cGLSLShader();

    //! Destructor of cGLSLShader
    ~cGLSLShader();

    //! Returns the program handle (for setting uniform variables)
    GLhandleARB getProgramHandle() const { return m_hPObject; }

protected:

    //! Called when we need to clean up shaders; e.g. when new shaders are being loaded
    virtual void uninitializeShaders();

    //! Called to create a fragment shader from m_fragmentShaderString
    virtual int initializeFragmentShader();

    //! Called to clean up a fragment shader
    virtual int uninitializeFragmentShader();

    //! Called to create a vertex shader from m_vertexShaderString
    virtual int initializeVertexShader();

    //! Called to clean up a vertex shader
    virtual int uninitializeVertexShader();

    //! Creates the shader program object
    int initializeProgram();

    //! Cleans up the shader program object (detaches the shaders and deletes the object)
    void uninitializeProgram();

    //! Called at the beginning of a rendering pass to enable shaders, must be
    //! over-ridden by subclasses
    virtual void enableShaders();

    //! Called at the end of a rendering pass to disable shaders, must be
    //! over-ridden by subclasses
    virtual void disableShaders();

    //! Has my program object been initialized?
    bool m_programInitialized;
    
    //! Handles for the fragment and vertex shaders and the program object
    GLhandleARB m_hFShader,m_hVShader,m_hPObject;	
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

