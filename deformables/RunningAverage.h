#ifndef _RUNAV_H_
#define _RUNAV_H_

/***********

  This class defines a simple running-average filter, used for
  low-pass filtering.  It has no concept of time; it just averages over
  a specified number of samples.
  
  It is derived from the running_average class provided with SPUC :

  http://spuc.sourceforge.net/

  ...but has been modified significantly since then.

  Copyright information for the original class :

  //! version="$Revision: 1.1 $"  
  //! Copyright(c) 1993-1996 Tony Kirke
  //! author="Tony Kirke" *

************/

template <class Numeric> class CRunningAverage {
public:

  // The current 'value' of the filter
  Numeric result;

  // When the number of samples received so far does not fill
  // up the filter's buffer, this is the sum of the values received
  // so far.  Keeping track of this lets us produce useful values
  // right after we start receiving data, instead of incurring a long
  // initialization delay.
  Numeric partial_result;

  // The most recent sample that was sent to the filter
  Numeric lastSample;

  // The vector of values we've received in the most recent time window
  // of the filter.  The buffer stores values that have been divided by
  // the length of the filter (so the sum of all the values in 'z' is 
  // the value of the running average).
  Numeric* z; 

  // The size of the 'z' buffer (the length of the filter, in samples)
	long size;

  // 'z' is a circular buffer; this represents the index in the buffer
  // where we should insert our next value.
  int curpos;

  // Have we received 'size' samples yet?  (i.e. is the filter 'full' yet?)
  int initialized;
  
  // A constructor that creates a new running-average filter of length
  // 'n' samples.
  CRunningAverage(long n=1) {
    if (n<1) n = 1;
    z = 0;
    set_size(n);
    lastSample = 0;
    partial_result = 0;
	}
		
  // Sets the size of the filter to 'n' samples, allocating new memory
  // for the incoming data.  Clears any history previously stored in the
  // filter's buffer.
  void set_size(long n) {

    // Don't do anything if we're already set to this size
    if (n == size) return;

	  if (z) delete [] z;
	  size = n;
    curpos = 0;
    initialized = 0;
    result = 0;
    partial_result = 0;
    z = new Numeric[size];
    memset(z,0,size*sizeof(Numeric));
	}

	~CRunningAverage(void) { if (z) delete [] z;}
	
  // Clears any history stored in the filter's buffer
  void reset(void) {
    curpos = 0;
    initialized = 0;
    partial_result = 0;
    result = 0;
  }

  // Get the current value of the filter
	Numeric get_result() { 

    // In normal operation, 'result' stores the value of the filter
    if (initialized) return(result);

    // Before we've received 'size' samples, we return the average of the
    // values we've received so far.
    else return (partial_result / curpos);
  }
	
  // Send a sample to the filter
  Numeric update(Numeric in) {
    lastSample = in;

    // Update the value of the filter
    in /= (float)size;
    result += in;

    // Since 'z' stores values that have already been divided by the
    // filter length, we just subtract off the oldest value from the
    // filter's value to make it 'disappear' from our history.
    if (initialized) {
      result -= z[curpos];
    }

    // Before we've initialized the buffer, we keep track of the
    // _sum_ of the values received so far.
    else {
      partial_result += lastSample;
    }

    // Insert the new value into the buffer
		z[curpos] = in;
    curpos++;

    // Roll around the end of our circular buffer if necessary
    if (curpos == size) {
      initialized = 1;
      curpos = 0;
    }

    return get_result();

  }
};

#endif
