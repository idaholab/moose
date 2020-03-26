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

registerMooseObject("TensorMechanicsTestApp", ADPowerLawCreepExceptionTest);

InputParameters
ADPowerLawCreepExceptionTest::validParams()
{
  InputParameters params = ADPowerLawCreepStressUpdate::validParams();
  params.addClassDescription(
      "This class duplicates the ADPowerLawCreepStressUpdate, except at the 2nd time step and the "
      "1st iteration, at which time a high residual is computed, forcing an exception.");
  return params;
}

ADPowerLawCreepExceptionTest::ADPowerLawCreepExceptionTest(const InputParameters & parameters)
  : ADPowerLawCreepStressUpdate(parameters)
{
}

ADReal
ADPowerLawCreepExceptionTest::computeResidual(const ADReal & effective_trial_stress,
                                              const ADReal & scalar)
{
  if (_fe_problem.getNonlinearSystemBase().getCurrentNonlinearIterationNumber() == 1 &&
      _t_step == 1 && _dt > 0.9)
    return 1.0;

  return ADPowerLawCreepStressUpdate::computeResidual(effective_trial_stress, scalar);
}

ADReal
ADPowerLawCreepExceptionTest::computeDerivative(const ADReal & effective_trial_stress,
                                                const ADReal & scalar)
{
  if (_fe_problem.getNonlinearSystemBase().getCurrentNonlinearIterationNumber() == 1 &&
      _t_step == 1 && _dt > 0.9)
    return 1.0;

  return ADPowerLawCreepStressUpdate::computeDerivative(effective_trial_stress, scalar);
}
