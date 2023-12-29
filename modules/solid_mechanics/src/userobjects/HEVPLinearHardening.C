//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HEVPLinearHardening.h"

registerMooseObject("TensorMechanicsApp", HEVPLinearHardening);

InputParameters
HEVPLinearHardening::validParams()
{
  InputParameters params = HEVPStrengthUOBase::validParams();
  params.addParam<Real>("yield_stress", "Yield strength");
  params.addParam<Real>("slope", "Linear hardening slope");
  params.addClassDescription("User Object for linear hardening");
  return params;
}

HEVPLinearHardening::HEVPLinearHardening(const InputParameters & parameters)
  : HEVPStrengthUOBase(parameters),
    _sig0(getParam<Real>("yield_stress")),
    _slope(getParam<Real>("slope"))
{
}

bool
HEVPLinearHardening::computeValue(unsigned int qp, Real & val) const
{
  val = _sig0 + _slope * _intvar[qp];
  return true;
}

bool
HEVPLinearHardening::computeDerivative(unsigned int /*qp*/,
                                       const std::string & coupled_var_name,
                                       Real & val) const
{
  val = 0;

  if (_intvar_prop_name == coupled_var_name)
    val = _slope;

  return true;
}
