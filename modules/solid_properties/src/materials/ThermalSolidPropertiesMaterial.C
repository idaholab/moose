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
    _cp(declareProperty<Real>("cp_solid")),
    _k(declareProperty<Real>("k_solid")),
    _rho(declareProperty<Real>("rho_solid"))
{
}

void
ThermalSolidPropertiesMaterial::computeQpProperties()
{
  _cp[_qp] = cp();
  _k[_qp] = k();
  _rho[_qp] = rho();
}
