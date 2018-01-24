//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DarcyFluxPressure.h"

template <>
InputParameters
validParams<DarcyFluxPressure>()
{
  InputParameters params = validParams<Kernel>();
  RealVectorValue g(0, 0, 0);
  params.addParam<RealVectorValue>("gravity", g, "Gravity vector (default is (0, 0, 0))");
  params.addClassDescription("");
  return params;
}

DarcyFluxPressure::DarcyFluxPressure(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _cond(getMaterialProperty<Real>("conductivity")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _density(getDefaultMaterialProperty<Real>("density"))
{
}

Real
DarcyFluxPressure::computeQpResidual()
{
  return _grad_test[_i][_qp] * _cond[_qp] * (_grad_u[_qp] - _density[_qp] * _gravity);
}

Real
DarcyFluxPressure::computeQpJacobian()
{
  return _grad_test[_i][_qp] * _cond[_qp] * _grad_phi[_j][_qp];
}
