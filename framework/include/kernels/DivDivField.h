//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorKernel.h"

/**
 *  Weak form contribution corresponding to -grad(k*div(u))
 */
class DivDivField : public VectorKernel
{
public:
  static InputParameters validParams();

  DivDivField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// div of the test function
  const VectorVariableTestDivergence & _div_test;

  /// div of the shape function
  const VectorVariablePhiDivergence & _div_phi;

  /// div of the variable
  const VectorVariableDivergence & _div_u;

  /// scalar coefficient
  Real _coeff;
};
