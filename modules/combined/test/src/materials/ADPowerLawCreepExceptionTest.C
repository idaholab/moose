//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPowerLawCreepExceptionTest.h"

#include "NonlinearSystemBase.h"

registerADMooseObject("TensorMechanicsTestApp", ADPowerLawCreepExceptionTest);

defineADLegacyParams(ADPowerLawCreepExceptionTest);

template <ComputeStage compute_stage>
InputParameters
ADPowerLawCreepExceptionTest<compute_stage>::validParams()
{
  InputParameters params = ADPowerLawCreepStressUpdate<compute_stage>::validParams();
  params.addClassDescription(
      "This class duplicates the ADPowerLawCreepStressUpdate, except at the 2nd time step and the "
      "1st iteration, at which time a high residual is computed, forcing an exception.");
  return params;
}

template <ComputeStage compute_stage>
ADPowerLawCreepExceptionTest<compute_stage>::ADPowerLawCreepExceptionTest(
    const InputParameters & parameters)
  : ADPowerLawCreepStressUpdate<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
ADReal
ADPowerLawCreepExceptionTest<compute_stage>::computeResidual(const ADReal & effective_trial_stress,
                                                             const ADReal & scalar)
{
  if (_fe_problem.getNonlinearSystemBase().getCurrentNonlinearIterationNumber() == 1 &&
      _t_step == 1 && _dt > 0.9)
    return 1.0;

  return ADPowerLawCreepStressUpdate<compute_stage>::computeResidual(effective_trial_stress,
                                                                     scalar);
}

template <ComputeStage compute_stage>
ADReal
ADPowerLawCreepExceptionTest<compute_stage>::computeDerivative(
    const ADReal & effective_trial_stress, const ADReal & scalar)
{
  if (_fe_problem.getNonlinearSystemBase().getCurrentNonlinearIterationNumber() == 1 &&
      _t_step == 1 && _dt > 0.9)
    return 1.0;

  return ADPowerLawCreepStressUpdate<compute_stage>::computeDerivative(effective_trial_stress,
                                                                       scalar);
}
