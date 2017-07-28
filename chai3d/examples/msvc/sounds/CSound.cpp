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
#include "stdafx.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <conio.h>
#include "CSound.h"

//---------------------------------------------------------------------------

// Change these values if your computer is slow and the sound is jerky:

// Sample rate
#define SAMPLE_RATE 44100
//---------------------------------------------------------------------------


//===========================================================================
//  - PUBLIC METHOD -
/*!
      This method takes as an input one of the pre-defined sound types
      (enumerated in CSound.h), and reads in the corresponding .sy file
      which contains the modes and their amplitudes and decay coefficents
      (data from Kies van den Doel's JASS program).  It also precomputes
      some values.  Reference: "The AHI: An Audio and Haptic Interface
      for Contact Interactions" by DiFilippo and Pai.

      \fn         void cSound::setParams(sounds choice)
*/
//===========================================================================
void cSound::setParams(sounds a_choice)
{
	int i, j;

  // select .sy data file based on input sound option
  FILE*  dfile;
  switch (a_choice)
  {
    case BELL:
      dfile = fopen("resources\\sounds\\handbell.sy", "r");
      scale = 1.0;
      break;
    case TEAPOT:
      dfile = fopen("resources\\sounds\\teapot.sy", "r");
      scale = 6.0;
      break;
  }

  // read in the number of modes, amplitudes, and decay coefficients
  char file_buffer[32];
  fgets(file_buffer, 256, dfile);
  fgets(file_buffer, 256, dfile);
  n = atoi(file_buffer);

  a.clear();
  f.clear();
  d.clear();

  // ignore header
  for (i=0; i<11; i++)
    fgets(file_buffer, 256, dfile);

  // read in the modes
  for (i=0; i<n; i++)
  {
    fgets(file_buffer, 256, dfile);
    f.push_back(atof(file_buffer));
  }

  // read in the decay coefficients
  fgets(file_buffer, 256, dfile);
  for (i=0; i<n; i++)
  {
    fgets(file_buffer, 256, dfile);
    d.push_back(atof(file_buffer));
  }

  // read in the amplitudes
  fgets(file_buffer, 256, dfile);
  for (i=0; i<n; i++)
  {
    fgets(file_buffer, 256, dfile);
    a.push_back(atof(file_buffer));
  }
  fclose(dfile);

  // allocate space for sound calculations
  treal = new double[n];
  timag = new double[n];
  yreal = new double[n];
  yimag = new double[n];
  tyreal = new double[n];
  tyimag = new double[n];

  // precompute real and imaginary parts of e^(i*omegan/Fs)
  for (j=0; j<n; j++)
  {
    treal[j] = exp(-d[j]/SAMPLE_RATE)*cos(2*3.14159*f[j]/SAMPLE_RATE);
    timag[j] = exp(-d[j]/SAMPLE_RATE)*sin(2*3.14159*f[j]/SAMPLE_RATE);
  }

  // equation 2 from "Active Haptics Interfaces" paper: yn(0) = an
  for (j=0; j<n; j++)
  {
    yreal[j] = 0.0;
    yimag[j] = 0.0;
  }

	m_counter = 0;
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      This method resets the sound data and should be called when a new
			contact is made.

      \fn         void cSound::reset()
*/
//===========================================================================
void cSound::reset()
{
	for (int j=0; j<n; j++)
  {
    yreal[j] = a[j];
    yimag[j] = 0.0;
  }
  m_counter = 0;
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      This method generates a sound wave based on the sound properties
      of the material and contact interactions, and writes it to a sound
      device to be played.  The sound model is based on "The AHI: An Audio
      and Haptic Interface For Contact Interactions" by DiFilippo and Pai.
			It returns one sample at a time and should be called repeatedly
			to fill the buffer in a BASS callback function.

      \fn         int cSound::play()
*/
//===========================================================================
int cSound::play()
{

  // attenuate the normal force so that the contact force decays gradually
  // rather than as a square function (which would produce a spurious
  // second hit)
	double audioForce = pow(0.85, m_counter++)*m_normalForce.length() + m_tangentialForce.length();

	// clamp the amplitude
	if (audioForce > 2.0) audioForce = 2.0;
	
  // convolve the haptic normal force with the sound modes; see the Pai papers
  // for an explanation of this calculation
  double lpTemp = 0.0;
  for (int j=0; j<n; j++)
  {
    tyreal[j] = treal[j]*yreal[j] - timag[j]*yimag[j];
    tyimag[j] = treal[j]*yimag[j] + timag[j]*yreal[j];
    yreal[j] = tyreal[j];
    yimag[j] = tyimag[j];
    yreal[j] += audioForce*a[j];
    lpTemp += yreal[j];
  }

  int value = (int)(lpTemp/scale) + 128; 
	if (value > 255) value = 255;
	return value;
}


//===========================================================================
//  END OF FILE
//===========================================================================

