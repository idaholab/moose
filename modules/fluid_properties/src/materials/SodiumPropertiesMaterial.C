//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SodiumPropertiesMaterial.h"

registerMooseObject("FluidPropertiesApp", SodiumPropertiesMaterial);

InputParameters
SodiumPropertiesMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Material properties for liquid sodium sampled from SodiumProperties.");
  params.addRequiredCoupledVar("temperature", "temperature (K)");
  params.addParam<UserObjectName>(
      "fp", "sodium", "The name of the user object with fluid properties");
  return params;
}

SodiumPropertiesMaterial::SodiumPropertiesMaterial(const InputParameters & parameters)
  : Material(parameters),
    _temperature(coupledValue("temperature")),
    _k(declareProperty<Real>("k")),
    _h(declareProperty<Real>("h")),
    _cp(declareProperty<Real>("cp")),
    _T_from_h(declareProperty<Real>("T_from_h")),
    _rho(declareProperty<Real>("rho")),
    _drho_dT(declareProperty<Real>("drho_dT")),
    _drho_dh(declareProperty<Real>("drho_dh")),

    _sodium(getUserObject<SodiumProperties>("fp"))
{
}

void
SodiumPropertiesMaterial::computeQpProperties()
{
  _k[_qp] = _sodium.k(_temperature[_qp]);
  _h[_qp] = _sodium.h(_temperature[_qp]);
  _cp[_qp] = _sodium.heatCapacity(_temperature[_qp]);
  _T_from_h[_qp] = _sodium.temperature(_h[_qp]);
  _rho[_qp] = _sodium.rho(_temperature[_qp]);
  _drho_dT[_qp] = _sodium.drho_dT(_temperature[_qp]);
  _drho_dh[_qp] = _sodium.drho_dh(_h[_qp]);
}
