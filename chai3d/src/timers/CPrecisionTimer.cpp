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
	\version    1.1
	\date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CPrecisionTimer.h"
#include "CPrecisionClock.h"
//---------------------------------------------------------------------------

// Initialize static variables
long cPrecisionTimer::m_numActiveTimers = 0;
long cPrecisionTimer::m_currentMinimumTimerInterval = 0;

// This was omitted from the VC6 SDK; it ensures that timers
// don't tick once you've called timeKillTimer()

// This flag appears to only work on XP, so we'll skip it for now...
// #define USE_TIME_KILL_SYNCHRONOUS

#ifdef USE_TIME_KILL_SYNCHRONOUS
#ifndef TIME_KILL_SYNCHRONOUS
#define TIME_KILL_SYNCHRONOUS 0x0100
#endif
#endif

#ifdef _POSIX
#include <sys/time.h>
#endif


//===========================================================================
/*!
	  Constructor of cPrecisionTimer.

	  \fn       cPrecisionTimer::cPrecisionTimer()
*/
//===========================================================================
cPrecisionTimer::cPrecisionTimer()
{
	// initialization
	defaults();

#if defined(_CYGWIN)

	m_threadID = (pthread_t)(-1);
	m_mutex = (pthread_mutex_t)(-1);
	m_condition = (pthread_cond_t)(-1);
	m_cancelThread = false;

	#elsif defined(_LINUX)

		m_threadID = (-1);
	m_mutex = (-1);
	m_condition = (-1);
	m_cancelThread = false;

#endif

}


//===========================================================================
/*!
	Destructor of cPrecisionTimer.

	\fn         cPrecisionTimer::~cPrecisionTimer()
*/
//===========================================================================
cPrecisionTimer::~cPrecisionTimer()
{
	// stop timer
	stop();

#if defined(_CYGWIN)
	if (m_mutex != -1) pthread_mutex_destroy(&m_mutex);
	if (m_condition - 1) pthread_cond_destroy(&m_condition);
	#elsif defined(_LINUX)
		if (m_mutex != (pthread_mutex_t)-1) pthread_mutex_destroy(&m_mutex);
	if (m_condition != (pthread_cond_t)-1) pthread_cond_destroy(&m_condition);
#endif
}


//===========================================================================
/*!
	Assign default values to variables; used for internal initialization
	only.

	\fn         void cPrecisionTimer::defaults()
*/
//===========================================================================
void cPrecisionTimer::defaults()
{
	m_averageCallbackInterval = 0;
	m_averageExecutionTime = 0;
	m_elapsedSinceLastCallback = 0;
	m_userData = 0;
	m_userCallback = 0;
	m_totalCallbacksCompleted = 0;
	m_totalCallbackTime = 0;
	m_previousCallbackStart = 0;
	m_interval = 0;

#ifndef _POSIX
	m_timer = 0;
#endif

}




#ifdef _POSIX

void *timer_thread_func(void *cptimer)
{
	cPrecisionTimer* cpt = (cPrecisionTimer*)cptimer;

	cPrecisionClock clock;

	// Until we're notified to do otherwise...
	while (cpt->m_cancelThread == false)
	{

		// Get the current time
		double start_time = clock.getCPUtime();

		// Run the user's callback
		internal_timer_callback(0, 0, (DWORD)(cpt), 0, 0);

		// Found out how much time we have left
		double end_time = clock.getCPUtime();

		double interval_s = ((double)(cpt->m_interval)) / 1000.0;

		double sleep_time = interval_s - (end_time - start_time);

		// If a whole interval has elapsed, keep going...
		if (sleep_time < 0)
		{
			// Sleep to be nice if we shouldn't be continuously computing...
			if (cpt->m_interval > 0)
			{
				// pthread_yield();
			}
			continue;
		}

		// Otherwise sleep for a bit
		pthread_mutex_lock(&cpt->m_mutex);

		timeval curt;
		gettimeofday(&curt, 0);

		// Convert current time to a timespec
		timespec current_time;
		current_time.tv_sec = curt.tv_sec;
		current_time.tv_nsec = curt.tv_usec * 1000;

		// How far into the future should we sleep?
		timespec sleep_end_time;
		sleep_end_time.tv_sec = current_time.tv_sec + (long)(sleep_time);

		// Leftover sleeping time...
		double sleep_duration_remainder = sleep_time - (long)(sleep_time);

		// Add our leftover sleeping time to the current time (in nanoseconds)
		sleep_end_time.tv_nsec = (long)(current_time.tv_nsec + (sleep_duration_remainder * 1000000000.0));

		// Move necessary nanoseconds over to seconds
		while (sleep_end_time.tv_nsec >= 1000000000) {
			sleep_end_time.tv_nsec -= 1000000000;
			sleep_end_time.tv_sec += 1;
		}

		pthread_cond_timedwait(&cpt->m_condition, &cpt->m_mutex, &sleep_end_time);
	}

	cpt->m_cancelThread = false;

}

#endif


//===========================================================================
/*!
	Request that a callback function be queued for repeated callbacks.
	Calling this function replaces any timer previously associated with
	this object (i.e., you need two cPrecisionTimers if you want two
	separate callbacks).

	\fn       int cPrecisionTimer::set(int a_interval,  PRECISION_TIMER_CALLBACK*
			  a_fpCallback, void* a_pUserData)

	\param    a_interval   The interval (in milliseconds) between successive
						  callbacks.

						  If iInterval is zero, the callback occurs
						  at the fastest rate supported by the system.

	\param    a_fpCallback  Pointer to a callback function.  The callback
						  should be a function of type PRECISION_TIMER_CALLBACK,
						  which is defined above.  Your function should look like :

						  void callback(void* user_data);

						  When your function gets called, you'll get back whatever
						  value you supplied as the user_data value in the set(...)
						  function.

	\param    a_pUserData   This value is handed back to your callback function
						  every time it gets called.

						  One useful thing to supply here might be the timer itself,
						  so you can easily access timing statistics during your
						  callback.

	\return   Returns 0 if all goes well.  If the specified interval
			  is not supported by the system, the return value is the
			  minimum interval (in milliseconds) supported by the system.
			  Any other error results in a return value of -1.

*/
//===========================================================================
int cPrecisionTimer::set(int a_interval, PRECISION_TIMER_CALLBACK* a_fpCallback, void* a_pUserData)
{

	// For Windows, set up the multimedia timer system
#ifndef _POSIX

	TIMECAPS tc;

	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
	{
		m_lastErrorMessage = "Could not get timer resolution.\n";
		return -1;
	}

	int min_interval = tc.wPeriodMin;
	int interval = a_interval;


	if ((a_interval == 0) || (a_interval < min_interval))
	{
		interval = min_interval;
	}

	m_interval = interval;

	// If we're the _first_ timer to be activated, or if this is a reduction in
	// the minimum timer interval, we need to pump up the system timer resolution.
	int timer_count = InterlockedIncrement(&m_numActiveTimers);

	if ((timer_count == 1) || (interval < m_currentMinimumTimerInterval))
	{
		// Erase the previous system timer period...
		if (timer_count > 1) timeEndPeriod(m_currentMinimumTimerInterval);

		// Set a new system timer period...
		m_currentMinimumTimerInterval = interval;
		MMRESULT result = timeBeginPeriod(interval);

		// Make sure it actually worked
		if (result != TIMERR_NOERROR)
		{
			m_lastErrorMessage = "Error setting system timer period.";
			return -1;
		}
	}
#else

	// POSIX-specific initialization
	m_currentMinimumTimerInterval = 0;
	m_numActiveTimers++;

#endif

	// Okay, the timing subsystem is set up, let's set up _this_ timer

	// Reset timing variables
	defaults();

	m_userData = a_pUserData;
	m_userCallback = a_fpCallback;

	// For Windows, set up a multimedia timer
#ifndef _POSIX

	// Start the timer
	m_timer = timeSetEvent(interval,
		0,
		internal_timer_callback,
		(unsigned long)(this),
		TIME_PERIODIC | TIME_CALLBACK_FUNCTION
#ifdef USE_TIME_KILL_SYNCHRONOUS
		| TIME_KILL_SYNCHRONOUS
#endif
	);

	if (m_timer == 0)
	{
		m_lastErrorMessage = "Error starting timer.";
		return -1;
	}

#else

	// POSIX-specific initialization

	m_interval = a_interval;

	// Create a mutex and condition variable if necessary
#if defined(_CYGWIN)
	if (m_mutex == (pthread_mutex_t)-1) pthread_mutex_init(&m_mutex, NULL);
	if (m_condition == (pthread_cond_t)-1) pthread_cond_init(&m_condition, NULL);
	#elsif defined(_LINUX)
		if (m_mutex == -1) pthread_mutex_init(&m_mutex, NULL);
	if (m_condition == -1) pthread_cond_init(&m_condition, NULL);
#endif

	m_cancelThread = false;

	// Spawn a new thread
	pthread_create(&m_threadID, 0, timer_thread_func, (void*)this);


#endif

	return 0;
}


//===========================================================================
/*!
	  Stop an active timer.

	  \fn       bool cPrecisionTimer::stop()
*/
//===========================================================================
bool cPrecisionTimer::stop()
{

	// Windows
#ifndef _POSIX

	// Make sure there _is_ a timer to stop
	if (m_timer == 0) return (false);

	// Stop the timer
	timeKillEvent(m_timer);

	int active_timers = InterlockedDecrement(&m_numActiveTimers);

	// If we're the only timer around, shut down the multimedia timers
	if (active_timers == 0)
	{
		timeEndPeriod(m_currentMinimumTimerInterval);
		m_currentMinimumTimerInterval = 0;
	}

	return (true);

#else

	m_cancelThread = true;

	return true;

#endif

}


//===========================================================================
/*!
	  Our global callback function, which triggers the user-defined callback
	  function for a given cPrecisionTimer, and maintains timing stats

	  \fn       void CALLBACK internal_timer_callback(UINT uTimerID,
				UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
*/
//===========================================================================
#ifdef _WIN32
void CALLBACK internal_timer_callback(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
#else
void internal_timer_callback(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
#endif
{
	cPrecisionTimer* timer = (cPrecisionTimer*)(dwUser);

	// The current time, in seconds, since this timer started running
	double curtime;

	// If this is our first callback, initialize some variables
	if (timer->m_totalCallbacksCompleted == 0)
	{
		timer->m_tickCounter.start();
		timer->m_elapsedSinceLastCallback = 0;
	}

	// Otherwise update the "elapsed" time and the average timer interval
	else
	{
		curtime = ((double)(timer->m_tickCounter.stop())) / 1000.0;
		timer->m_elapsedSinceLastCallback = curtime - timer->m_previousCallbackStart;
		timer->m_averageCallbackInterval = curtime / timer->m_totalCallbacksCompleted;
	}

	// Call the user-defined callback and time it
	double start_time = timer->m_tickCounter.getCPUtime();

	if (timer->m_userCallback)
	{
		timer->m_userCallback(timer->m_userData);
	}
	double end_time = timer->m_tickCounter.getCPUtime();

	// Compute some statistics about user callbacks
	timer->m_totalCallbackTime += end_time - start_time;
	timer->m_totalCallbacksCompleted++;
	timer->m_averageExecutionTime = timer->m_totalCallbackTime / timer->m_totalCallbacksCompleted;
}
