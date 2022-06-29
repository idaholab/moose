//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Diffusion.h"

/**
 *  The Laplacian operator with a function coefficient
 */
class FunctionDiffusion : public Diffusion
{
public:
  static InputParameters validParams();

  FunctionDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  /// Function coefficient
  const Function & _function;

  /// Gradient of the concentration variable for kernel to operate on
  const VariableGradient & _grad_v;

  /// Optional coupled concentration variable
  const MooseVariable * _v_var;

  /// Gradient of the shape function
  const VariablePhiGradient & _grad_v_phi;
};
