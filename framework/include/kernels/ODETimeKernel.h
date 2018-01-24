//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ODETIMEKERNEL_H
#define ODETIMEKERNEL_H

#include "ODEKernel.h"

// Forward Declarations
class ODETimeKernel;

template <>
InputParameters validParams<ODETimeKernel>();

/**
 * Base class for ODEKernels that contribute to the time residual
 * vector.
 */
class ODETimeKernel : public ODEKernel
{
public:
  ODETimeKernel(const InputParameters & parameters);

  virtual void computeResidual() override;
};

#endif
