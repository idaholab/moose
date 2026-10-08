//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosHeatConductionMaterial.h"

registerKokkosMaterial("HeatTransferApp", KokkosHeatConductionMaterial);

InputParameters
KokkosHeatConductionMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addCoupledVar("temperature", "Coupled Temperature");

  params.addParam<Real>("thermal_conductivity", "The thermal conductivity value");
  params.addParam<FunctionName>("thermal_conductivity_temperature_function",
                                "",
                                "Thermal conductivity as a function of temperature.");
  params.addParam<Real>("min_T",
                        "Minimum allowable value for temperature for evaluating properties "
                        "when provided by functions");

  params.addParam<Real>("specific_heat", "The specific heat value");
  params.addParam<FunctionName>(
      "specific_heat_temperature_function", "", "Specific heat as a function of temperature.");

  params.addClassDescription("General-purpose material model for heat conduction");

  return params;
}

KokkosHeatConductionMaterial::KokkosHeatConductionMaterial(const InputParameters & parameters)
  : Material(parameters),
    _has_temp(isCoupled("temperature")),
    _temperature(_has_temp ? kokkosCoupledValue("temperature") : kokkosZeroValue()),
    _my_thermal_conductivity(
        isParamValid("thermal_conductivity") ? getParam<Real>("thermal_conductivity") : 0),
    _my_specific_heat(isParamValid("specific_heat") ? getParam<Real>("specific_heat") : 0),
    _thermal_conductivity(declareKokkosProperty<Real>("thermal_conductivity")),
    _thermal_conductivity_dT(declareKokkosProperty<Real>("thermal_conductivity_dT")),
    _thermal_conductivity_temperature_function(
        getParam<FunctionName>("thermal_conductivity_temperature_function") != ""
            ? getKokkosFunction("thermal_conductivity_temperature_function")
            : Moose::Kokkos::Function()),
    _specific_heat(declareKokkosProperty<Real>("specific_heat")),
    _specific_heat_temperature_function(
        getParam<FunctionName>("specific_heat_temperature_function") != ""
            ? getKokkosFunction("specific_heat_temperature_function")
            : Moose::Kokkos::Function()),
    _specific_heat_dT(declareKokkosProperty<Real>("specific_heat_dT")),
    _has_min_T(isParamValid("min_T")),
    _min_T(_has_min_T ? getParam<Real>("min_T") : 0)
{
  if (_thermal_conductivity_temperature_function && !_has_temp)
    paramError("thermal_conductivity_temperature_function",
               "Must couple with temperature if using thermal conductivity function");

  if (isParamValid("thermal_conductivity") && _thermal_conductivity_temperature_function)
    mooseError(
        "Cannot define both thermal conductivity and thermal conductivity temperature function");

  if (_specific_heat_temperature_function && !_has_temp)
    paramError("specific_heat_temperature_function",
               "Must couple with temperature if using specific heat function");

  if (isParamValid("specific_heat") && _specific_heat_temperature_function)
    mooseError("Cannot define both specific heat and specific heat temperature function");
}
