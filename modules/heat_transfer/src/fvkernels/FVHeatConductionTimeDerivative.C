//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVHeatConductionTimeDerivative.h"

registerMooseObject("HeatConductionApp", FVHeatConductionTimeDerivative);

InputParameters
FVHeatConductionTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "AD Time derivative term $\\rho c_p \\frac{\\partial T}{\\partial t}$ of "
      "the heat equation for quasi-constant specific heat $c_p$ and the density $\\rho$.");
  params.addParam<MaterialPropertyName>(
      "specific_heat", "specific_heat", "Property name of the specific heat material property");
  params.addParam<MaterialPropertyName>(
      "density_name", "density", "Property name of the density material property");
  return params;
}

FVHeatConductionTimeDerivative::FVHeatConductionTimeDerivative(const InputParameters & parameters)
  : FVTimeKernel(parameters),
    _specific_heat(getADMaterialProperty<Real>("specific_heat")),
    _density(getADMaterialProperty<Real>("density_name"))
{
}

ADReal
FVHeatConductionTimeDerivative::computeQpResidual()
{
  return _specific_heat[_qp] * _density[_qp] * FVTimeKernel::computeQpResidual();
}
