//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalSolidPropertiesMaterial.h"

template <>
InputParameters
validParams<ThermalSolidPropertiesMaterial>()
{
  InputParameters params = validParams<SolidPropertiesMaterial>();
  params.addClassDescription("Material providing solid thermal properties");
  params.addRequiredCoupledVar("temperature", "Temperature");
  params.addParam<std::string>(
      "cp_name", "cp_solid", "Name to be used for the isobaric specific heat");
  params.addParam<std::string>("k_name", "k_solid", "Name to be used for the thermal conductivity");
  params.addParam<std::string>("rho_name", "rho_solid", "Name to be used for the density");
  return params;
}

ThermalSolidPropertiesMaterial::ThermalSolidPropertiesMaterial(const InputParameters & parameters)
  : SolidPropertiesMaterial(parameters),
    _temperature(coupledValue("temperature")),
    _temperature_name(getVar("temperature", 0)->name()),
    _cp_name(getParam<std::string>("cp_name")),
    _k_name(getParam<std::string>("k_name")),
    _rho_name(getParam<std::string>("rho_name")),
    _cp(declareProperty<Real>(_cp_name)),
    _dcp_dT(declarePropertyDerivative<Real>(_cp_name, _temperature_name)),
    _k(declareProperty<Real>(_k_name)),
    _dk_dT(declarePropertyDerivative<Real>(_k_name, _temperature_name)),
    _rho(declareProperty<Real>(_rho_name)),
    _drho_dT(declarePropertyDerivative<Real>(_rho_name, _temperature_name))
{
}

void
ThermalSolidPropertiesMaterial::computeQpProperties()
{
  computeIsobaricSpecificHeat();
  computeThermalConductivity();
  computeDensity();

  computeIsobaricSpecificHeatDerivatives();
  computeThermalConductivityDerivatives();
  computeDensityDerivatives();
}
