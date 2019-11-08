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
 * Computes the residual and Jacobian contribution for the weak form
 * of the biharmonic equation:
 *
 * \int Laplacian(u) * Laplacian(v) dx
 */
class Biharmonic : public Kernel
{
public:
  static InputParameters validParams();

  Biharmonic(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const VariableSecond & _second_u;
  const VariablePhiSecond & _second_phi;
  const VariableTestSecond & _second_test;
};
