/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SimpleACInterface.h"

template <>
InputParameters
validParams<SimpleACInterface>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription(
      "Gradient energy for Allen-Cahn Kernel with constant Mobility and Interfacial parameter");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa_op", "The kappa used with the kernel");
  return params;
}

SimpleACInterface::SimpleACInterface(const InputParameters & parameters)
  : Kernel(parameters),
    _L(getMaterialProperty<Real>("mob_name")),
    _kappa(getMaterialProperty<Real>("kappa_name"))
{
}

Real
SimpleACInterface::computeQpResidual()
{
  return _grad_u[_qp] * _kappa[_qp] * _L[_qp] * _grad_test[_i][_qp];
}

Real
SimpleACInterface::computeQpJacobian()
{
  return _grad_phi[_j][_qp] * _kappa[_qp] * _L[_qp] * _grad_test[_i][_qp];
}
