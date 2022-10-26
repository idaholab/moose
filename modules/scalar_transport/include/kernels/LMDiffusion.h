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

/**
 * Adds the strong diffusive term of the primal equation to stabilization of the Lagrange multiplier
 * equation
 */
class LMDiffusion : public Kernel
{
public:
  LMDiffusion(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  Real computeQpResidual() override;
  Real computeQpJacobian() override;
  Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  /// The primal variable number
  const unsigned int _primal_var;

  /// The matrix of second spatial derivatives of the primal variable
  const VariableSecond & _second_primal;

  /// The matrix of second spatial derivatives of the basis functions of the primal variable
  const VariablePhiSecond & _second_primal_phi;

  /// The sign of the Lagrange multiplier (the 'variable' of this kernel) in the primal equation
  const Real _lm_sign;

  /// The primal variable diffusivity
  const Real _diffusivity;
};
