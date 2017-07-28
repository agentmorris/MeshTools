/***********

  This class provides a nice wrapper around the Intel high-performance
  counter, so all times in can be represented in seconds.  This class
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

  public :
    CElapsed() {

      // get the frequency of the counter
      Initialized = QueryPerformanceFrequency( (LARGE_INTEGER *)&Frequency );
      
    }

    int Begin() {
      if(Initialized == 0) return 0;

      // get the starting counter value
      return QueryPerformanceCounter( (LARGE_INTEGER *)&BeginTime );
    }

    double End() {

      if(Initialized == 0) return 0.0;

      // get the ending counter value
      __int64 endtime;
      QueryPerformanceCounter( (LARGE_INTEGER *)&endtime );

      // determine the elapsed counts
      __int64 elapsed = endtime - BeginTime;

      // convert counts to time in seconds and return it
      return (double)elapsed / (double)Frequency;
    }

    // returns true if the perf counter is available      
    int Available() {
      return Initialized;
    }

    // return perf counter frequency as large int
    __int64 GetFreq() {
      return Frequency;
    }

    double GetTime() {
      if(Initialized == 0) {
        return 0.0;
      }

      // get the ending counter value
      __int64 curtime;
      QueryPerformanceCounter( (LARGE_INTEGER *)&curtime );

      return (double)curtime / (double)Frequency;
    }

    double GetStartTime() {
      if(Initialized == 0) return 0.0;
      return (double)(BeginTime) / (double)(Frequency);
    }
};

#endif