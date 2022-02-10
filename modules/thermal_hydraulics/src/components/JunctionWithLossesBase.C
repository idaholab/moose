//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JunctionWithLossesBase.h"

InputParameters
JunctionWithLossesBase::validParams()
{
  InputParameters params = FlowJunction::validParams();
  params.addRequiredParam<std::vector<Real>>("K", "Form loss coefficients [-]");
  // use same values in K for K_reverse if not provided
  params.addParam<std::vector<Real>>("K_reverse", "Reverse form loss coefficients [-]");
  params.addRequiredParam<Real>("A_ref", "Junction Reference area [m^2]");

  return params;
}

JunctionWithLossesBase::JunctionWithLossesBase(const InputParameters & parameters)
  : FlowJunction(parameters),
    _k_coeffs(getParam<std::vector<Real>>("K")),
    _kr_coeffs(isParamValid("K_reverse") ? getParam<std::vector<Real>>("K_reverse")
                                         : getParam<std::vector<Real>>("K")),
    _ref_area(getParam<Real>("A_ref"))
{
  checkSizeEqualsNumberOfConnections<Real>("K");
  if (isParamValid("K_reverse"))
    checkSizeEqualsNumberOfConnections<Real>("K_reverse");
}
