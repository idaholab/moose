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
  return params;
}

ThermalSolidPropertiesMaterial::ThermalSolidPropertiesMaterial(const InputParameters & parameters)
  : SolidPropertiesMaterial(parameters),
    _temperature(coupledValue("temperature")),
    _temperature_name(getVar("temperature", 0)->name()),
    _cp(declareProperty<Real>("cp_solid")),
    _dcp_dT(declarePropertyDerivative<Real>("cp_solid", _temperature_name)),
    _k(declareProperty<Real>("k_solid")),
    _dk_dT(declarePropertyDerivative<Real>("k_solid", _temperature_name)),
    _rho(declareProperty<Real>("rho_solid")),
    _drho_dT(declarePropertyDerivative<Real>("rho_solid", _temperature_name))
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
