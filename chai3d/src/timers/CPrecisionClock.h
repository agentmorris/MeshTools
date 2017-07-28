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
	\author:    Francois Conti
	\version    1.1
	\date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CPrecisionClockH
#define CPrecisionClockH
//---------------------------------------------------------------------------
#ifdef _WIN32
#include "windows.h"
#endif
//---------------------------------------------------------------------------

//===========================================================================
/*!
	\file   CPrecisionClock.h
	\class	cPrecisionClock
	\brief	cPrecisionClock provides a class to manage high precision
			time measurments. Units are micro seconds.
*/
//===========================================================================
class cPrecisionClock
{
public:
	// CONSTRUCTOR & DESTRUCTOR:
	//! Constructor of cPrecisionClock.
	cPrecisionClock();

	//! Destructor of cPrecisionClock.
	~cPrecisionClock();

	// METHODS:
	//! reset clock to zero.
	void initialize();

	//! start counting from current time of clock.
	long start();

	//! stop clock counting.
	long stop();

	//! return \b true if timer is currently \b on, else return \b false.
	bool on() { return (m_on); };

	//! set current time of clock. Units are \e microseconds.
	void setCurrentTime(long a_currentTime);

	//! read current time of clock. Units are \e microseconds.
	long getCurrentTime();

	//! set the period in microseconds before timeout occurs.
	void setTimeoutPeriod(long a_timeoutPeriod);

	//! Read the programmed timeout period.
	long getTimeoutPeriod() { return (m_timeoutPeriod); }

	//! return \b true if timeout has occurred.
	bool timeoutOccurred();

	//! return \b true if high resolution timers are available on this computer.
	bool highResolution() { return (m_highres); };

	//! If all you want is something that tells you the time, this is your function...
	double getCPUtime();

private:

#ifndef _POSIX
	//! Stores information about CPU high precision clock.
	LARGE_INTEGER m_freq;
#endif

	//! Current time of clock.
	long m_timeCurrent;

	//! CPU time when clock was started.
	long m_timeStart;

	//! Timeout period
	long m_timeoutPeriod;

	//! CPU time when timer was started.
	long m_timeoutStart;

	//! If \b true, then high precision CPU clock is available.
	bool m_highres;

	//! If \b true, clock is \b on.
	bool m_on;

	//! Read CPU clock in microseconds.
	long getCount();
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
