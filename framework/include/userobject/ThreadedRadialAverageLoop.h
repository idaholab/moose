/**********************************************************************/
/*                     DO NOT MODIFY THIS HEADER                      */
/* MAGPIE - Mesoscale Atomistic Glue Program for Integrated Execution */
/*                                                                    */
/*            Copyright 2017 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

#pragma once

#include "RadialAverage.h"

#include "libmesh/nanoflann.hpp"

using QPDataRange =
    StoredRange<std::vector<RadialAverage::QPData>::const_iterator, RadialAverage::QPData>;

/**
 * RadialAverage threaded loop
 */
class ThreadedRadialAverageLoop
{
public:
  ThreadedRadialAverageLoop(RadialAverage &);

  /// Splitting constructor
  ThreadedRadialAverageLoop(const ThreadedRadialAverageLoop & x, Threads::split split);

  /// dummy virtual destructor
  virtual ~ThreadedRadialAverageLoop() {}

  /// parens operator with the code that is executed in threads
  void operator()(const QPDataRange & range);

  /// thread join method
  virtual void join(const ThreadedRadialAverageLoop & /*x*/) {}

protected:
  /// rasterizer to manage the sample data
  RadialAverage & _radavg;

  /// ID number of the current thread
  THREAD_ID _tid;

private:
};
