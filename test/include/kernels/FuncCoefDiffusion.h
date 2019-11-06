//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Kernel.h"
#include "Function.h"

/**
 * A kernel for testing the MooseParsedFunctionInterface
 */
class FuncCoefDiffusion : public Kernel
{
public:
  static InputParameters validParams();

  FuncCoefDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  const Function & _function;
};
