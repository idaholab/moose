//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EqualValueConstraint.h"
#include "SubProblem.h"
#include "FEProblem.h"

registerMooseObject("MooseApp", EqualValueConstraint);

InputParameters
EqualValueConstraint::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addClassDescription(
      "EqualValueConstraint enforces solution continuity between secondary and "
      "primary sides of a mortar interface using lagrange multipliers");
  return params;
}

EqualValueConstraint::EqualValueConstraint(const InputParameters & parameters)
  : ADMortarConstraint(parameters)
{
}

ADReal
EqualValueConstraint::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Slave:
      return -_lambda[_qp] * _test_secondary[_i][_qp];
    case Moose::MortarType::Master:
      return _lambda[_qp] * _test_primary[_i][_qp];
    case Moose::MortarType::Lower:
      return (_u_primary[_qp] - _u_secondary[_qp]) * _test[_i][_qp];
    default:
      return 0;
  }
}
