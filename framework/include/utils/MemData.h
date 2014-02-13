/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MEMDATA_H
#define MEMDATA_H

// System headers needed for MacOSX
#include <sys/resource.h> // getrusage, TODO configure for this

// System headers needed for Linux
#include <sys/types.h> // pid_t
#include <unistd.h> // getpid()

// C++ headers
#include <fstream>

/**
 * Object for gathering memory usage data in a platform-independent way.
 *
 * @author John W. Peterson, 2011
 */
class MemData
{
public:
  /**
   * Constructor.
   */
  MemData();

  /**
   * Destructor.
   */
  virtual ~MemData();

  /**
   * Get an initial memory count at the beginning of an operation.
   */
  void start();

  /**
   * Get a final memory count at the end of an operation.
   */
  void stop();

  /**
   * Computes the change in memory, as logged by this object between
   * calls to start() and stop()
   */
  long delta();

private:

  /**
   * This function has different meanings on different platforms...
   *
   * .) On Mac OSX it returns the ru_maxrss value reported by getrusage,
   * which is actually in bytes rather than kilobytes as the man page says.
   *
   * .) On linux, it returns the VmSize entry from the /proc/PID/status file
   * for the current process.
   *
   * Obviously this should not be used to compare memory usage across different
   * platforms...
   */
  void get_current_mem_usage(long& mem_in_kB, struct rusage& r_usage);

  /// Used on both linux and Mac to store the current memory usage in kilobytes.
  long _mem_in_kB_at_start, _mem_in_kB_at_stop;

  /// Used only on Mac OS, but rusage struct should be at least defined everywhere...
  struct rusage _r_usage_at_start, _r_usage_at_stop;

  /**
   * If true, this object has started logging memory usage.  Note, you can't "stop()"
   * unless you've _started!
   */
  bool _started;
};


#endif // MEMDATA_H
