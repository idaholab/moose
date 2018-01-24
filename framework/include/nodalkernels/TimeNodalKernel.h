//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TIMENODALKERNEL_H
#define TIMENODALKERNEL_H

#include "NodalKernel.h"

// Forward Declarations
class TimeNodalKernel;
class Function;

template <>
InputParameters validParams<TimeNodalKernel>();

/**
 * Represents a simple ODE of du/dt - rate = 0
 */
class TimeNodalKernel : public NodalKernel
{
public:
  /**
   * Constructor initializes the rate
   */
  TimeNodalKernel(const InputParameters & parameters);

protected:
  virtual void computeResidual() override;
};

#endif
