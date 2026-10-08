//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosElectricalConductivity.h"

registerKokkosMaterial("HeatTransferApp", KokkosElectricalConductivity);

InputParameters
KokkosElectricalConductivity::validParams()
{
  InputParameters params = Material::validParams();
  params.addCoupledVar("temperature", 293.0, "Coupled variable for temperature.");
  params.addParam<std::string>("base_name", "Material property base name.");
  params.addParam<Real>("reference_resistivity",
                        1.68e-8,
                        "Electrical resistivity of the material at reference temperature "
                        "(default is copper resistivity in ohm-m).");
  params.addParam<Real>("temperature_coefficient",
                        0.00386,
                        "Coefficient for calculating dependence of resistivity on temperature "
                        "(with copper as the default material).");
  params.addParam<Real>("reference_temperature",
                        293.0,
                        "Reference temperature for Electrical resistivity in Kelvin.");
  params.addClassDescription("Calculates resistivity and electrical conductivity as a function of "
                             "temperature, using copper for parameter defaults.");
  return params;
}

KokkosElectricalConductivity::KokkosElectricalConductivity(const InputParameters & parameters)
  : Material(parameters),
    _ref_resis(getParam<Real>("reference_resistivity")),
    _temp_coeff(getParam<Real>("temperature_coefficient")),
    _ref_temp(getParam<Real>("reference_temperature")),
    _T(kokkosCoupledValue("temperature")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _electric_conductivity(
        declareKokkosPropertyByName<Real>(_base_name + "electrical_conductivity")),
    _delectric_conductivity_dT(
        declareKokkosPropertyByName<Real>(_base_name + "electrical_conductivity_dT"))
{
}
