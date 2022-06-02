//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PowerLawCreepTest.h"

#include "NonlinearSystemBase.h"

registerMooseObject("TensorMechanicsTestApp", PowerLawCreepTest);
registerMooseObject("TensorMechanicsTestApp", ADPowerLawCreepTest);

template <bool is_ad>
InputParameters
PowerLawCreepTestTempl<is_ad>::validParams()
{
  InputParameters params = PowerLawCreepStressUpdateTempl<is_ad>::validParams();
  params.addClassDescription(
      "This class duplicates the PowerLawCreepStressUpdate, except at a specificed time step and "
      "the 1st iteration, at which time a high residual is computed, forcing an exception. "
      "Optionally, a high or low initial guess can be used to test the check_rage limits, which "
      "will also force an exception.");
  params.addParam<int>("failure_step", 2, "Time step for which to inject a high residual.");
  params.addParam<Real>("initial_guess", 0.0, "Initial guess for inner Newton solve.");

  return params;
}

template <bool is_ad>
PowerLawCreepTestTempl<is_ad>::PowerLawCreepTestTempl(const InputParameters & parameters)
  : PowerLawCreepStressUpdateTempl<is_ad>(parameters),
    _failure_step(this->template getParam<int>("failure_step")),
    _initial_guess(this->template getParam<Real>("initial_guess"))
{
  _check_range = true;
}

template <bool is_ad>
GenericReal<is_ad>
PowerLawCreepTestTempl<is_ad>::computeResidual(const GenericReal<is_ad> & effective_trial_stress,
                                               const GenericReal<is_ad> & scalar)
{
  if (_fe_problem.getNonlinearSystemBase().getCurrentNonlinearIterationNumber() == 1 &&
      _t_step == _failure_step && _dt == 1.0)
    return 1.0;

  return PowerLawCreepStressUpdateTempl<is_ad>::computeResidual(effective_trial_stress, scalar);
}

template <bool is_ad>
GenericReal<is_ad>
PowerLawCreepTestTempl<is_ad>::computeDerivative(const GenericReal<is_ad> & effective_trial_stress,
                                                 const GenericReal<is_ad> & scalar)
{
  if (_fe_problem.getNonlinearSystemBase().getCurrentNonlinearIterationNumber() == 1 &&
      _t_step == _failure_step && _dt == 1.0)
    return 1.0;

  return PowerLawCreepStressUpdateTempl<is_ad>::computeDerivative(effective_trial_stress, scalar);
}
