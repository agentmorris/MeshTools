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

    The sound card output is based on a tutorial by David Overton, at
    http://www.planet-source-code.com/vb/scripts/ShowCode.asp?txtCodeId=4422&lngWId=3.
    Sound generation is based on the algorithm by Kies Van den Doel K and
    Dinesh Pai in "The sounds of physical shapes". Presence 1998, 7(4): 382-395,
    and "The AHI: An Audio and Haptic Interface For Contact Interactions" by
    DiFilippo and Pai.

    \author:    <http://www.chai3d.org>
    \author:    Chris Sewell
    \version    1.1
    \date       06/2005
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CSoundH
#define CSoundH
//---------------------------------------------------------------------------
#include <mmsystem.h>
#include <windowsx.h>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "CMatrix3d.h"
#include "CVector3d.h"
#include "bass.h"

// Materials for which .sy files with sound parameter values are available
enum sounds { BELL, TEAPOT };

//---------------------------------------------------------------------------


//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      cSound
      \brief      cSound plays sounds based on frequencies, amplitudes,
                  and attenuations from a data file, based on the algorithm
                  by Kies Van den Doel K and Dinesh Pai in "The sounds of
                  physical shapes". Presence 1998, 7(4): 382-395, and
                  "The AHI: An Audio and Haptic Interface For Contact
                  Interactions" by DiFilippo and Pai.  Sample
                  data files for a bell and a teapot are in this directory.
*/
//===========================================================================
class cSound
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cSound.
    cSound() { m_counter = 0; m_normalForce.set(0,0,0); m_tangentialForce.set(0,0,0);};
    //! Destructor of cSound.
    ~cSound() {};

    // METHODS:
    //! Set material sound parameters
    void setParams(sounds a_choice);
    //! Get the next sound sample
    int play();
    //! Reset the sound computations when a new contact is made
    void reset();
    //! Get function for current normal contact force on this sound object
    cVector3d getNormalForce() { return m_normalForce; }
    //! Get function for current tangential contact force on this sound object
    cVector3d getTangentialForce() { return m_tangentialForce; }
    //! Get function for current previous normal force on this sound object
    cVector3d getPreviousForce() { return m_previousForce; }
    //! Set function for current contact force on this object; also sets m_previousForce to former value
    void setContactForce(cVector3d a_normalForce, cVector3d a_tangentialForce) {
         m_previousForce = m_normalForce; m_normalForce = a_normalForce;
         m_tangentialForce = a_tangentialForce; }
    //! BASS stream associated with this sound
    HSTREAM stream;

  private:
    // PROPERTIES:
    //! Number of modes
    int n;
    //! Frequency, amplitude, and decay coefficient material sound properties
    std::vector<double> a, f, d;
    //! Arrays for calculating sound data
    double *yreal, *yimag, *treal, *timag, *tyreal, *tyimag;
    //! Value to scale sound samples by to be in 0-256 range
    double scale;
    //! Counter; used to attenuate normal force over time
    int m_counter;
    //! Previous contact force on this object; used to determine when a new contact has been made
    cVector3d m_previousForce;
    //! Current normal force on this object
    cVector3d m_normalForce;
    //! Current tangential force on this object
    cVector3d m_tangentialForce;
};

#endif
//===========================================================================
//  END OF FILE
//===========================================================================
