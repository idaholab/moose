//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleACInterface.h"

registerMooseObject("PhaseFieldApp", SimpleACInterface);

InputParameters
SimpleACInterface::validParams()
{
  InputParameters params = Kernel::validParams();
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
