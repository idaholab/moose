//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalSolidPropertiesMaterial.h"

registerMooseObject("SolidPropertiesApp", ThermalSolidPropertiesMaterial);

template <>
InputParameters
validParams<ThermalSolidPropertiesMaterial>()
{
  InputParameters params = validParams<SolidPropertiesMaterial>();
  params.addRequiredCoupledVar("temperature", "Temperature");
  params.addRequiredParam<UserObjectName>("sp", "The name of the user object for solid properties");
  params.addClassDescription("Material providing solid thermal properties");
  return params;
}

ThermalSolidPropertiesMaterial::ThermalSolidPropertiesMaterial(const InputParameters & parameters)
  : SolidPropertiesMaterial(parameters),
    _temperature(coupledValue("temperature")),
    _cp(declareProperty<Real>("cp_solid")),
    _k(declareProperty<Real>("k_solid")),
    _rho(declareProperty<Real>("rho_solid")),

    _sp(getUserObject<ThermalSolidProperties>("sp"))
{
}

void
ThermalSolidPropertiesMaterial::computeQpProperties()
{
  _cp[_qp] = _sp.cp_from_T(_temperature[_qp]);
  _k[_qp] = _sp.k_from_T(_temperature[_qp]);
  _rho[_qp] = _sp.rho_from_T(_temperature[_qp]);
}
