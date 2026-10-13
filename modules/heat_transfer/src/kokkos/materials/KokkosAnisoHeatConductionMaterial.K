//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosAnisoHeatConductionMaterial.h"

#include "RankTwoTensor.h"

registerKokkosMaterial("HeatTransferApp", KokkosAnisoHeatConductionMaterial);

InputParameters
KokkosAnisoHeatConductionMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addCoupledVar("temperature", "Coupled variable for temperature.");
  params.addParam<Real>(
      "reference_temperature", 293.0, "Reference temperature for thermal conductivity in Kelvin.");
  params.addParam<std::string>("base_name", "Material property base name.");
  params.addRequiredParam<std::vector<Real>>("thermal_conductivity",
                                             "The thermal conductivity tensor values");
  params.addParam<FunctionName>(
      "thermal_conductivity_temperature_coefficient_function",
      "",
      "Temperature coefficient for thermal conductivity as a function of temperature.");
  params.addRequiredParam<FunctionName>("specific_heat",
                                        "Specific heat as a function of temperature.");
  params.addClassDescription("General-purpose material model for anisotropic heat conduction");
  return params;
}

KokkosAnisoHeatConductionMaterial::KokkosAnisoHeatConductionMaterial(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _ref_temp(getParam<Real>("reference_temperature")),
    _T(kokkosCoupledValue("temperature")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _user_provided_thermal_conductivity(
        RankTwoTensor(getParam<std::vector<Real>>("thermal_conductivity"))),
    _thermal_conductivity(declareKokkosPropertyByName<Real33>(_base_name + "thermal_conductivity")),
    _dthermal_conductivity_dT(declareKokkosPropertyDerivative<Real33>(
        _base_name + "thermal_conductivity", {coupledName("temperature", 0)})),
    _thermal_conductivity_temperature_coefficient_function(
        getParam<FunctionName>("thermal_conductivity_temperature_coefficient_function") != ""
            ? getKokkosFunction("thermal_conductivity_temperature_coefficient_function")
            : Moose::Kokkos::Function()),
    _specific_heat(declareKokkosPropertyByName<Real>(_base_name + "specific_heat")),
    _dspecific_heat_dT(declareKokkosPropertyDerivative<Real>(_base_name + "specific_heat",
                                                             {coupledName("temperature", 0)})),
    _specific_heat_function(getKokkosFunction("specific_heat"))
{
}
