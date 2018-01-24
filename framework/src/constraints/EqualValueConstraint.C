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

template <>
InputParameters
validParams<EqualValueConstraint>()
{
  InputParameters params = validParams<FaceFaceConstraint>();
  return params;
}

EqualValueConstraint::EqualValueConstraint(const InputParameters & parameters)
  : FaceFaceConstraint(parameters)
{
}

Real
EqualValueConstraint::computeQpResidual()
{
  return (_u_master[_qp] - _u_slave[_qp]) * _test[_i][_qp];
}

Real
EqualValueConstraint::computeQpResidualSide(Moose::ConstraintType res_type)
{
  switch (res_type)
  {
    case Moose::Master:
      return _lambda[_qp] * _test_master[_i][_qp];
    case Moose::Slave:
      return -_lambda[_qp] * _test_slave[_i][_qp];
    default:
      return 0;
  }
}

Real
EqualValueConstraint::computeQpJacobianSide(Moose::ConstraintJacobianType jac_type)
{
  switch (jac_type)
  {
    case Moose::MasterMaster:
    case Moose::SlaveMaster:
      return _phi[_j][_qp] * _test_master[_i][_qp];

    case Moose::MasterSlave:
    case Moose::SlaveSlave:
      return -_phi[_j][_qp] * _test_slave[_i][_qp];

    default:
      return 0;
  }
}
