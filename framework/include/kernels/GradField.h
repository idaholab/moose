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
 *  Weak form contribution corresponding to -k*grad(p)
 */
class GradField : public VectorKernel
{
public:
  static InputParameters validParams();

  GradField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpOffDiagJacobian(unsigned jvar) override;

  /// coupled scalar variable
  MooseVariable & _p_var;
  unsigned int _p_var_num;

  /// value of the coupled scalar variable
  const VariableValue & _p;

  /// shape function of the coupled scalar variable
  const VariablePhiValue & _p_phi;

  /// div of the test function
  const VectorVariableTestDivergence & _div_test;

  /// scalar coefficient
  Real _coeff;
};
