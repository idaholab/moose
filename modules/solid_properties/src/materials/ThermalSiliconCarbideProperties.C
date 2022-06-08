//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalSiliconCarbideProperties.h"

registerMooseObject("SolidPropertiesApp", ThermalSiliconCarbideProperties);

InputParameters
ThermalSiliconCarbideProperties::validParams()
{
  InputParameters params = ThermalSolidPropertiesMaterial::validParams();
  params.addClassDescription("Material defining silicon carbide thermal properties.");

  MooseEnum ThermalSiliconCarbidePropertiesKModel("snead parfume", "snead");
  params.addParam<MooseEnum>("thermal_conductivity_model",
                             ThermalSiliconCarbidePropertiesKModel,
                             "Thermal conductivity model to be used");
  params.addRangeCheckedParam<Real>("density", 3216.0, "density > 0.0", "(Constant) density");
  return params;
}

ThermalSiliconCarbideProperties::ThermalSiliconCarbideProperties(const InputParameters & parameters)
  : ThermalSolidPropertiesMaterial(parameters),
    _k_model(getParam<MooseEnum>("thermal_conductivity_model")
                 .getEnum<ThermalSiliconCarbidePropertiesKModel>()),
    _rho_const(getParam<Real>("density"))
{
}

Real
ThermalSiliconCarbideProperties::molarMass() const
{
  return 40.0962e-3;
}

void
ThermalSiliconCarbideProperties::computeIsobaricSpecificHeat()
{
  _cp[_qp] = 925.65 + 0.3772 * _temperature[_qp] - 7.9259e-5 * std::pow(_temperature[_qp], 2.0) -
             3.1946e7 * std::pow(_temperature[_qp], -2.0);
}

void
ThermalSiliconCarbideProperties::computeIsobaricSpecificHeatDerivatives()
{
  _dcp_dT[_qp] = 0.3772 - 7.9259e-5 * 2.0 * _temperature[_qp] -
                 3.1946e7 * -2.0 * std::pow(_temperature[_qp], -3.0);
}

void
ThermalSiliconCarbideProperties::computeThermalConductivity()
{
  if (_k_model == snead)
    _k[_qp] = 1.0 / (-0.0003 + 1.05e-5 * _temperature[_qp]);
  else if (_k_model == parfume)
    _k[_qp] = 17885.0 / _temperature[_qp] + 2.0;
  else
    mooseError(name(), ": Unhandled MooseEnum in ThermalSiliconCarbideProperties!");
}

void
ThermalSiliconCarbideProperties::computeThermalConductivityDerivatives()
{
  if (_k_model == snead)
    _dk_dT[_qp] = -1.0 / std::pow(-0.0003 + 1.05e-5 * _temperature[_qp], 2.0) * 1.05e-5;
  else if (_k_model == parfume)
    _dk_dT[_qp] = -17885.0 / std::pow(_temperature[_qp], 2.0);
  else
    mooseError(name(), ": Unhandled MooseEnum in ThermalSiliconCarbideProperties!");
}

void
ThermalSiliconCarbideProperties::computeDensity()
{
  _rho[_qp] = _rho_const;
}

void
ThermalSiliconCarbideProperties::computeDensityDerivatives()
{
  _drho_dT[_qp] = 0.0;
}
