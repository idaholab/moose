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
 *  Weak form contribution corresponding to k*div(u)
 */
class DivField : public Kernel
{
public:
  static InputParameters validParams();

  DivField(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpOffDiagJacobian(unsigned jvar) override;

  /// coupled vector variable
  const VectorMooseVariable & _u_var;
  unsigned int _u_var_num;

  /// div of the coupled vector variable
  const VectorVariableDivergence & _div_u;

  /// div of the shape function of the coupled vector variable
  const VectorVariablePhiDivergence & _div_phi;

  /// scalar coefficient
  Real _coeff;
};
