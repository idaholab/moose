//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PowerLawCreepExceptionTest.h"

#include "NonlinearSystemBase.h"

registerMooseObject("TensorMechanicsTestApp", PowerLawCreepExceptionTest);

InputParameters
PowerLawCreepExceptionTest::validParams()
{
  InputParameters params = PowerLawCreepStressUpdate::validParams();
  params.addClassDescription(
      "This class duplicates the PowerLawCreepStressUpdate, except at 2nd time step and the 1st "
      "iteration, at which time a high residual is computed, forcing an exception.");
  return params;
}

PowerLawCreepExceptionTest::PowerLawCreepExceptionTest(const InputParameters & parameters)
  : PowerLawCreepStressUpdate(parameters)
{
}

Real
PowerLawCreepExceptionTest::computeResidual(const Real & effective_trial_stress,
                                            const Real & scalar)
{
  if (_fe_problem.getNonlinearSystemBase().getCurrentNonlinearIterationNumber() == 1 &&
      _t_step == 1 && _dt > 0.9)
    return 1.0;

  return PowerLawCreepStressUpdate::computeResidual(effective_trial_stress, scalar);
}

Real
PowerLawCreepExceptionTest::computeDerivative(const Real & effective_trial_stress,
                                              const Real & scalar)
{
  if (_fe_problem.getNonlinearSystemBase().getCurrentNonlinearIterationNumber() == 1 &&
      _t_step == 1 && _dt > 0.9)
    return 1.0;

  return PowerLawCreepStressUpdate::computeDerivative(effective_trial_stress, scalar);
}
