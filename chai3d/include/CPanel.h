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
    \date       3/2005
*/
//===========================================================================

#ifndef C2DPanelH
#define C2DPanelH

#include "CMesh.h"
#include "CVertex.h"
#include "CTriangle.h"

//===========================================================================
/*!
\file       CPanel.h
\class      cPanel
\brief      This class represents a rectangular panel that can be displayed
            within a CHAI viewport, and it is intended to be added to the 
            front and back 2d rendering trees within the camera class.
            It generally behaves like a cMesh (from which it subclasses),
            you can give it colors, materials, textures, etc.  It can also be
            used as a 3D rectangle, although its primary purpose is for 2D
            rendering.
*/
//===========================================================================
class cPanel : public cMesh
{
public:

    //! Constructor of cPanel
    cPanel(cWorld* a_world, const bool a_enableCollisionDetecion=false);
   
    //! Destructor of cPanel
    virtual ~cPanel();

    //! Returns true if a solid rectangle is being rendered to represent the panel
    bool getDisplayRect() { return m_showRect; };
    //! Controls the rendering of a solid rectangle at the location of the panel
    void setDisplayRect(const bool& a_showRect) { m_showRect = a_showRect; }
    
    //! Returns true if a rectangle outline is being rendered around the panel
    bool getDisplayEdges() { return m_showEdges; }
    //! Controls the rendering of a rectangle outline around the panel
    void setDisplayEdges(const bool& a_showEdges) { m_showEdges = a_showEdges; }

    //! Set the width and height of the panel (in pixels) (z is ignored)
    void setSize(const cVector3d& a_size);
    //! Get the width and height of the panel (in pixels) (z is ignored)
    cVector3d getSize() { return m_size; }

    //! Set the width used to render the panel outline
    void setEdgeWidth(const float& a_width) { m_edgeWidth = a_width; }
    //! Get the width used to render the panel outline
    float getEdgeWidth() { return m_edgeWidth; }

    //! Render the panel (overrides the parent render function)
    virtual void render(const int a_renderMode=0);  

    //! The color of the background rectangle.  Has no effect if m_showRect is false, or
    //! if m_useColors or m_useMaterials are true.  By default, this is the color used
    //! to represent the rectangle.
    //!
    //! If texturing is enabled, you can use the alpha component of this color to control
    //! the alpha of an opaque texture.  This is especially useful if you set the texture
    //! itself to render in GL_DECAL mode, so the color comes entirely from the texture
    //! and the alpha comes from here.
    cColorf m_rectColor;

    //! The color of the rectangle outline.  Has no effect if m_showEdges is false.
    cColorf m_edgeColor;

    // The next few members are used to solve specific problems that come up 
    // with mixing 2D and 3D rendering... you probably don't need to mess with
    // them if you're not having problems.

    //! Should clipping be explicitly disabled for this panel?
    bool m_disableClipping;

    //! Should I use GL lighting?  Usually you want this off for 'true 2d'
    bool m_useLighting;

    //! Should depth-testing be explicitly disabled for this panel?
    bool m_disableDepthTest;

    //! Should polygon-offseting be used for this panel?
    bool m_usePolygonOffset;

protected:

    //! Controls the rendering of a solid rectangle at the location of the panel
    bool m_showRect;

    //! Controls the rendering of a rectangle outline around the panel
    bool m_showEdges;

    //! The width used to render the panel outline
    float m_edgeWidth;

    //! The size of the panel (in 2D mode, this is in pixels and z is ignored)
    cVector3d m_size;
  
};
#endif
