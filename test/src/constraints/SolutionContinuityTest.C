//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolutionContinuityTest.h"

registerADMooseObject("MooseTestApp", SolutionContinuityTest);

defineADValidParams(SolutionContinuityTest, MortarConstraint, );

template <ComputeStage compute_stage>
SolutionContinuityTest<compute_stage>::SolutionContinuityTest(const InputParameters & parameters)
  : MortarConstraint<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
ADResidual
SolutionContinuityTest<compute_stage>::computeQpResidual()
{
  return _has_master ? _test[_i][_qp] * (_u_slave[_qp] - _u_master[_qp]) : ADReal(0);
}

template <ComputeStage compute_stage>
ADResidual
SolutionContinuityTest<compute_stage>::computeQpResidualSide(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::Slave:
      return _test_slave[_i][_qp] * _lambda[_qp];

    case Moose::Master:
      return -_test_master[_i][_qp] * _lambda[_qp];

    default:
      return 0;
  }
}
