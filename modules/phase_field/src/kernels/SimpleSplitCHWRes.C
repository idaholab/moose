/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SimpleSplitCHWRes.h"

template <>
InputParameters
validParams<SimpleSplitCHWRes>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription(
      "Gradient energy for split Cahn-Hilliard equation with constant Mobility");
  params.addParam<MaterialPropertyName>(
      "mob_name", "M", "The mobility used with the kernel, should be a constant value");
  return params;
}

SimpleSplitCHWRes::SimpleSplitCHWRes(const InputParameters & parameters)
  : Kernel(parameters), _M(getMaterialProperty<Real>("mob_name"))
{
}

Real
SimpleSplitCHWRes::computeQpResidual()
{
  return _M[_qp] * _grad_u[_qp] * _grad_test[_i][_qp];
}

Real
SimpleSplitCHWRes::computeQpJacobian()
{
  return _M[_qp] * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
