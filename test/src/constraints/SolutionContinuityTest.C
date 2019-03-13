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

defineADValidParams(SolutionContinuityTest, RealMortarConstraint, );

template <ComputeStage compute_stage>
SolutionContinuityTest<compute_stage>::SolutionContinuityTest(const InputParameters & parameters)
  : RealMortarConstraint<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
ADResidual
SolutionContinuityTest<compute_stage>::computeLMQpResidual()
{
  return _has_master ? _u_primal_slave(_qp) - _u_primal_master(_qp) : ADReal(0);
}
