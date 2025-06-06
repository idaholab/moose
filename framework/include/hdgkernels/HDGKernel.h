//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

/**
 * A kernel for hybridized finite element formulations
 */
class HDGKernel : public Kernel
{
public:
  static InputParameters validParams();

  HDGKernel(const InputParameters & parameters);

  virtual void computeResidualOnSide() = 0;
  virtual void computeJacobianOnSide() = 0;
  virtual void computeResidualAndJacobianOnSide();

protected:
  virtual Real computeQpResidual() override { mooseError("this should never be called"); }
};
