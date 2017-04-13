/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SimpleCHInterface.h"

template <>
InputParameters
validParams<SimpleCHInterface>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Gradient energy for Cahn-Hilliard equation with constant Mobility "
                             "and Interfacial parameter");
  params.addRequiredParam<MaterialPropertyName>(
      "kappa_name", "The kappa used with the kernel, should be constant value");
  params.addRequiredParam<MaterialPropertyName>(
      "mob_name", "The mobility used with the kernel, should be constant value");
  return params;
}

SimpleCHInterface::SimpleCHInterface(const InputParameters & parameters)
  : Kernel(parameters),
    _second_u(second()),
    _second_test(secondTest()),
    _second_phi(secondPhi()),
    _M(getMaterialProperty<Real>("mob_name")),
    _kappa_c(getMaterialProperty<Real>("kappa_name"))
{
}

Real
SimpleCHInterface::computeQpResidual()
{
  return _kappa_c[_qp] * _second_u[_qp].tr() * _M[_qp] * _second_test[_i][_qp].tr();
}

Real
SimpleCHInterface::computeQpJacobian()
{
  return _kappa_c[_qp] * _second_phi[_j][_qp].tr() * _M[_qp] * _second_test[_i][_qp].tr();
}
