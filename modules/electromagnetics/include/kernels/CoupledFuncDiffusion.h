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
 *  Represents a coupled Laplacian term with sign and function coefficients
 */
class CoupledFuncDiffusion : public Kernel
{
public:
  static InputParameters validParams();

  CoupledFuncDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  /// Function coefficient
  const Function & _func;

  /// Scalar coefficient representing the sign of the residual contribution
  Real _sign;

  /// Coupled field variable
  const VariableGradient & _coupled_grad;
};
