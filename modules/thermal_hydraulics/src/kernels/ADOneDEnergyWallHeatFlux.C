//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADOneDEnergyWallHeatFlux.h"

registerMooseObject("ThermalHydraulicsApp", ADOneDEnergyWallHeatFlux);

InputParameters
ADOneDEnergyWallHeatFlux::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("q_wall", "Wall heat flux material property");
  params.addRequiredCoupledVar("P_hf", "heat flux perimeter");
  params.addClassDescription("Computes a heat flux term for the energy equation");
  return params;
}

ADOneDEnergyWallHeatFlux::ADOneDEnergyWallHeatFlux(const InputParameters & parameters)
  : ADKernel(parameters),
    _q_wall(getADMaterialProperty<Real>("q_wall")),
    _P_hf(adCoupledValue("P_hf"))
{
}

ADReal
ADOneDEnergyWallHeatFlux::computeQpResidual()
{
  return -_q_wall[_qp] * _P_hf[_qp] * _test[_i][_qp];
}
