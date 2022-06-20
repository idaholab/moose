//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalSolidPropertiesMaterial.h"
#include "ThermalSolidProperties.h"

registerMooseObject("SolidPropertiesApp", ThermalSolidPropertiesMaterial);

InputParameters
ThermalSolidPropertiesMaterial::validParams()
{
  InputParameters params = SolidPropertiesMaterial::validParams();
  params.addRequiredCoupledVar("temperature", "Temperature");
  params.addRequiredParam<UserObjectName>("sp", "The name of the user object for solid properties");
  params.addParam<std::string>(
      "cp_name", "cp_solid", "Name to be used for the isobaric specific heat");
  params.addParam<std::string>("k_name", "k_solid", "Name to be used for the thermal conductivity");
  params.addParam<std::string>("rho_name", "rho_solid", "Name to be used for the density");
  params.addClassDescription("Computes solid thermal properties as a function of temperature");
  return params;
}

ThermalSolidPropertiesMaterial::ThermalSolidPropertiesMaterial(const InputParameters & parameters)
  : SolidPropertiesMaterial(parameters),
    _temperature(coupledValue("temperature")),

    _cp_name(getParam<std::string>("cp_name")),
    _k_name(getParam<std::string>("k_name")),
    _rho_name(getParam<std::string>("rho_name")),

    _cp(declareProperty<Real>(_cp_name)),
    _k(declareProperty<Real>(_k_name)),
    _rho(declareProperty<Real>(_rho_name)),
    _dcp_dT(declareProperty<Real>("d" + _cp_name + "_dT")),
    _dk_dT(declareProperty<Real>("d" + _k_name + "_dT")),
    _drho_dT(declareProperty<Real>("d" + _rho_name + "_dT")),

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
