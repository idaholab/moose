//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HEVPRambergOsgoodHardening.h"

registerMooseObject("TensorMechanicsApp", HEVPRambergOsgoodHardening);

InputParameters
HEVPRambergOsgoodHardening::validParams()
{
  InputParameters params = HEVPStrengthUOBase::validParams();
  params.addParam<Real>("yield_stress", "Yield strength");
  params.addRequiredParam<Real>("reference_plastic_strain", "Reference plastic strain value");
  params.addRequiredParam<Real>("hardening_exponent", "The hardening exponent value");
  params.addClassDescription("User object for Ramberg-Osgood hardening power law hardening");

  return params;
}

HEVPRambergOsgoodHardening::HEVPRambergOsgoodHardening(const InputParameters & parameters)
  : HEVPStrengthUOBase(parameters),
    _sig0(getParam<Real>("yield_stress")),
    _peeq0(getParam<Real>("reference_plastic_strain")),
    _exponent(getParam<Real>("hardening_exponent"))
{
}

bool
HEVPRambergOsgoodHardening::computeValue(unsigned int qp, Real & val) const
{
  val = _sig0 * std::pow(_intvar[qp] / _peeq0 + 1.0, _exponent);
  return true;
}

bool
HEVPRambergOsgoodHardening::computeDerivative(unsigned int qp,
                                              const std::string & coupled_var_name,
                                              Real & val) const
{
  val = 0;

  if (_intvar_prop_name == coupled_var_name)
    val = _sig0 * _exponent / _peeq0 * std::pow(_intvar[qp] / _peeq0 + 1.0, _exponent - 1.0);

  return true;
}
