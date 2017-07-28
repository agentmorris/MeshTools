/***********

  This class provides a nice wrapper around the Intel high-performance
  counter, so all times in TG2 can be represented in seconds.  This class
  is used both to access relative times (using Begin() and End(); e.g. the
  time since the beginning of an experiment) and to access absolute time
  (using GetTime()).

***********/

#include <conio.h>

#ifndef _C_ELAPSED_H_
#define _C_ELAPSED_H_

class CElapsed {
private :
  int Initialized;
  __int64 Frequency;
  __int64 BeginTime;
  
  static bool m_timeIsHijacked;
  static double m_hijackedTimeValue;
  static double m_hijackedBeginValue;

public :

  // take over the clock so it's user-controlled
  //
  // static: affects _all_ CElapsed objects
  static void hijackTime(bool hijack, double time=0.0) {
    m_timeIsHijacked = hijack;
    m_hijackedTimeValue = time;
    m_hijackedBeginValue = 0.0;
  }

  // constructor
  CElapsed() {

    // get the frequency of the counter
    Initialized = QueryPerformanceFrequency( (LARGE_INTEGER *)&Frequency );
    if (Initialized == 0) {
      _cprintf("Very bad error: could not initialize celapsed clock\n");
    }
    BeginTime = 0;
  }

  // start timing
  inline int Begin() {
    if( ! Initialized )
      return 0;   // error - couldn't get frequency

    if (m_timeIsHijacked) {
      m_hijackedBeginValue = m_hijackedTimeValue;
      return m_hijackedBeginValue;
    }

    // get the starting counter value
    else return QueryPerformanceCounter( (LARGE_INTEGER *)&BeginTime );
  }

  // stop timing and get elapsed time in seconds
  inline double End() {

    if( ! Initialized )
      return 0.0; // error - couldn't get frequency

    if (m_timeIsHijacked) return m_hijackedTimeValue - m_hijackedBeginValue;

    // get the ending counter value
    __int64 endtime;
    QueryPerformanceCounter( (LARGE_INTEGER *)&endtime );

    // determine the elapsed counts
    __int64 elapsed = endtime - BeginTime;

    // convert counts to time in seconds and return it
    return (double)elapsed / (double)Frequency;
  }

  inline int Available()  // returns true if the perf counter is available
    { return Initialized; }

  inline __int64 GetFreq() // return perf counter frequency as large int
    { return Frequency; }

  inline double GetTime() {
    if( !Initialized ) {
      _cprintf("Accessing uninitialized celapsed...\n");
      return 0.0; // error - couldn't get frequency
    }

    if (m_timeIsHijacked) return m_hijackedTimeValue;

    // get the ending counter value
    __int64 curtime;
    QueryPerformanceCounter( (LARGE_INTEGER *)&curtime );

    return (double)curtime / (double)Frequency;
  }

  inline double GetStartTime() {
    if( ! Initialized )
      return 0.0;

    if (m_timeIsHijacked) return m_hijackedBeginValue;

    return (double)(BeginTime) / (double)(Frequency);
  }

};

#endif