//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUHeatConductionMaterial.h"

registerMooseObject("HeatTransferApp", GPUHeatConductionMaterial);

InputParameters
GPUHeatConductionMaterial::validParams()
{
  InputParameters params = GPUMaterial::validParams();

  params.addCoupledVar("temp", "Coupled Temperature");

  params.addParam<Real>("thermal_conductivity", 0, "The thermal conductivity value");
  params.addParam<Real>("min_T",
                        -std::numeric_limits<Real>::max(),
                        "Minimum allowable value for temperature for evaluating properties "
                        "when provided by functions");
  params.addParam<Real>("specific_heat", 0, "The specific heat value");

  params.addClassDescription("General-purpose material model for heat conduction");

  return params;
}

GPUHeatConductionMaterial::GPUHeatConductionMaterial(const InputParameters & parameters)
  : GPUMaterial(parameters),
    _has_temp(isCoupled("temp")),
    _temperature(_has_temp ? GPUCoupledValue("temp") : GPUZeroValue()),
    _my_thermal_conductivity(getParam<Real>("thermal_conductivity")),
    _my_specific_heat(getParam<Real>("specific_heat")),
    _thermal_conductivity(declareGPUProperty<Real>("thermal_conductivity")),
    _thermal_conductivity_dT(declareGPUProperty<Real>("thermal_conductivity_dT")),
    _specific_heat(declareGPUProperty<Real>("specific_heat")),
    _specific_heat_dT(declareGPUProperty<Real>("specific_heat_dT")),
    _min_T(getParam<Real>("min_T"))
{
}
