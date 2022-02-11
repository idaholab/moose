//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADOneD3EqnMomentumGravity.h"

registerMooseObject("ThermalHydraulicsApp", ADOneD3EqnMomentumGravity);

InputParameters
ADOneD3EqnMomentumGravity::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<RealVectorValue>("gravity_vector", "Gravitational acceleration vector");
  params.addClassDescription("Computes gravity term for the momentum equation for 1-phase flow");
  return params;
}

ADOneD3EqnMomentumGravity::ADOneD3EqnMomentumGravity(const InputParameters & parameters)
  : ADKernel(parameters),
    _A(adCoupledValue("A")),
    _rho(getADMaterialProperty<Real>("rho")),
    _dir(getMaterialProperty<RealVectorValue>("direction")),
    _gravity_vector(getParam<RealVectorValue>("gravity_vector"))
{
}

ADReal
ADOneD3EqnMomentumGravity::computeQpResidual()
{
  return -_rho[_qp] * _A[_qp] * _gravity_vector * _dir[_qp] * _test[_i][_qp];
}
