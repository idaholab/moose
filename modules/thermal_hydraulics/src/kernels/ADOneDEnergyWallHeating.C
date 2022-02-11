//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADOneDEnergyWallHeating.h"

registerMooseObject("ThermalHydraulicsApp", ADOneDEnergyWallHeating);

InputParameters
ADOneDEnergyWallHeating::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredCoupledVar("P_hf", "heat flux perimeter");
  params.addCoupledVar("T_wall", 0, "Wall temperature (const)");
  params.addRequiredParam<MaterialPropertyName>("Hw",
                                                "Convective heat transfer coefficient, W/m^2-K");
  params.addRequiredParam<MaterialPropertyName>("T", "Temperature material property");

  return params;
}

ADOneDEnergyWallHeating::ADOneDEnergyWallHeating(const InputParameters & parameters)
  : ADKernel(parameters),
    _temperature(getADMaterialProperty<Real>("T")),
    _Hw(getADMaterialProperty<Real>("Hw")),
    _T_wall(adCoupledValue("T_wall")),
    _P_hf(adCoupledValue("P_hf"))
{
}

ADReal
ADOneDEnergyWallHeating::computeQpResidual()
{
  return _Hw[_qp] * _P_hf[_qp] * (_temperature[_qp] - _T_wall[_qp]) * _test[_i][_qp];
}
