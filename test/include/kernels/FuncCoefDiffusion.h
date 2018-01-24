//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCCOEFDIFFUSION_H
#define FUNCCOEFDIFFUSION_H

// MOOSE includes
#include "Kernel.h"
#include "Function.h"

// Forward Declarations
class FuncCoefDiffusion;

template <>
InputParameters validParams<FuncCoefDiffusion>();

/**
 * A kernel for testing the MooseParsedFunctionInterface
 */
class FuncCoefDiffusion : public Kernel
{
public:
  FuncCoefDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  Function & _function;
};

#endif // FUNCCOEFDIFFUSION_H
