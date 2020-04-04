//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleCHInterface.h"

registerMooseObject("PhaseFieldApp", SimpleCHInterface);

InputParameters
SimpleCHInterface::validParams()
{
  InputParameters params = Kernel::validParams();
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
