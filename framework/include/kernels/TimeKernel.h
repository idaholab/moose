//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TIMEKERNEL_H
#define TIMEKERNEL_H

#include "Kernel.h"

// Forward Declaration
class TimeKernel;

template <>
InputParameters validParams<TimeKernel>();

/**
 * All time kernels should inherit from this class
 *
 */
class TimeKernel : public Kernel
{
public:
  TimeKernel(const InputParameters & parameters);

  virtual void computeResidual() override;
};

#endif // TIMEKERNEL_H
