/**********************************************************************/
/*                     DO NOT MODIFY THIS HEADER                      */
/* MAGPIE - Mesoscale Atomistic Glue Program for Integrated Execution */
/*                                                                    */
/*            Copyright 2017 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

#pragma once

#include "PointListAdaptor.h"
#include "RadialGreensConvolution.h"

#include "libmesh/nanoflann.hpp"

using QPDataRange = StoredRange<std::vector<RadialGreensConvolution::QPData>::const_iterator,
                                RadialGreensConvolution::QPData>;

/**
 * RadialGreensConvolution threaded loop
 */
class ThreadedRadialGreensConvolutionLoop
{
public:
  ThreadedRadialGreensConvolutionLoop(RadialGreensConvolution &);

  /// Splitting constructor
  ThreadedRadialGreensConvolutionLoop(const ThreadedRadialGreensConvolutionLoop & x,
                                      Threads::split split);

  /// dummy virtual destructor
  virtual ~ThreadedRadialGreensConvolutionLoop() {}

  /// parens operator with the code that is executed in threads
  void operator()(const QPDataRange & range);

  /// thread join method
  virtual void join(const ThreadedRadialGreensConvolutionLoop & x)
  {
    _convolution_integral += x._convolution_integral;
  }

  /// return the convolution integral
  Real convolutionIntegral() { return _convolution_integral; }

protected:
  /// rasterizer to manage the sample data
  RadialGreensConvolution & _green;

  /// name of the Green's Function
  FunctionName _function_name;

  /// integral over the convolution contribution
  Real _convolution_integral;

  /// ID number of the current thread
  THREAD_ID _tid;

private:
};
