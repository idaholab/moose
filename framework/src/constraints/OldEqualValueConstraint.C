//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OldEqualValueConstraint.h"
#include "SubProblem.h"
#include "FEProblem.h"

registerMooseObject("MooseApp", OldEqualValueConstraint);

defineLegacyParams(OldEqualValueConstraint);

InputParameters
OldEqualValueConstraint::validParams()
{
  InputParameters params = MortarConstraint::validParams();
  params.addClassDescription(
      "OldEqualValueConstraint enforces solution continuity between secondary and "
      "primary sides of a mortar interface using lagrange multipliers");
  return params;
}

OldEqualValueConstraint::OldEqualValueConstraint(const InputParameters & parameters)
  : MortarConstraint(parameters)
{
}

Real
OldEqualValueConstraint::computeQpResidual(Moose::MortarType mortar_type)
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

Real
OldEqualValueConstraint::computeQpJacobian(Moose::ConstraintJacobianType jacobian_type,
                                           unsigned int jvar)
{
  typedef Moose::ConstraintJacobianType JType;

  switch (jacobian_type)
  {
    case JType::SlaveLower:
      if (jvar == _var->number())
        return -(*_phi)[_j][_qp] * _test_secondary[_i][_qp];
      break;

    case JType::MasterLower:
      if (jvar == _var->number())
        return (*_phi)[_j][_qp] * _test_primary[_i][_qp];
      break;

    case JType::LowerSlave:
      if (jvar == _secondary_var.number())
        return -(*_phi)[_j][_qp] * _test[_i][_qp];
      break;

    case JType::LowerMaster:
      if (jvar == _primary_var.number())
        return (*_phi)[_j][_qp] * _test[_i][_qp];
      break;

    default:
      return 0;
  }

  return 0;
}
