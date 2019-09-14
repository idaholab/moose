//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

class EmptyKernel;

template <>
InputParameters validParams<EmptyKernel>();

/**
 * Empty kernel class for element whose residual does not need to be calculated
 */
class EmptyKernel : public Kernel
{
public:
  EmptyKernel(const InputParameters & parameters);

  virtual void computeResidual() override{};
  virtual void computeJacobian() override{};
  virtual Real computeQpResidual() override { return 0.0; }
};
