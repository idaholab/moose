//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADOneD3EqnMomentumAreaGradient.h"

registerMooseObject("ThermalHydraulicsApp", ADOneD3EqnMomentumAreaGradient);

InputParameters
ADOneD3EqnMomentumAreaGradient::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>("p", "Pressure");
  params.addClassDescription(
      "Computes the area gradient term in the momentum equation for single phase flow.");
  return params;
}

ADOneD3EqnMomentumAreaGradient::ADOneD3EqnMomentumAreaGradient(const InputParameters & parameters)
  : ADKernel(parameters),
    _area_grad(coupledGradient("A")),
    _dir(getMaterialProperty<RealVectorValue>("direction")),
    _pressure(getADMaterialProperty<Real>("p"))
{
}

ADReal
ADOneD3EqnMomentumAreaGradient::computeQpResidual()
{
  return -_pressure[_qp] * _area_grad[_qp] * _dir[_qp] * _test[_i][_qp];
}
