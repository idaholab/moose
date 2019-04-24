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

registerADMooseObject("MooseApp", EqualValueConstraint);

defineADValidParams(EqualValueConstraint,
                    MortarConstraint,
                    params.addClassDescription(
                        "EqualValueConstraint enforces solution continuity between slave and "
                        "master sides of a mortar interface using lagrange multipliers"););

template <ComputeStage compute_stage>
EqualValueConstraint<compute_stage>::EqualValueConstraint(const InputParameters & parameters)
  : MortarConstraint<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
ADReal
EqualValueConstraint<compute_stage>::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Slave:
      return -_lambda[_qp] * _test_slave[_i][_qp];
    case Moose::MortarType::Master:
      return _lambda[_qp] * _test_master[_i][_qp];
    case Moose::MortarType::Lower:
      return (_u_master[_qp] - _u_slave[_qp]) * _test[_i][_qp];
    default:
      return 0;
  }
}
