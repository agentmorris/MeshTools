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
    \author:    Chris Sewell
    \version    1.1
    \date       06/2005
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CSoundMeshH
#define CSoundMeshH
//---------------------------------------------------------------------------
#include "CMesh.h"
#include "CSound.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
      \class      cSoundMesh
      \brief      cSoundMesh subclasses cMesh and includes a pointer to a
                  CSound object for playing contact sounds.
*/
//===========================================================================
class cSoundMesh : public cMesh
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cPhantom3dofPointer.
    cSoundMesh(cWorld* a_world) : cMesh(a_world) { m_sound = new cSound(); };
    //! Destructor of cPhantom3dofPointer.
    virtual ~cSoundMesh() { };
    //! Get sound object.
    cSound* getSound() { return m_sound; }

  private:
    //! Pointer to sound object.
    cSound* m_sound;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

