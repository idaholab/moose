/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HEVPLinearHardening.h"

template <>
InputParameters
validParams<HEVPLinearHardening>()
{
  InputParameters params = validParams<HEVPStrengthUOBase>();
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
