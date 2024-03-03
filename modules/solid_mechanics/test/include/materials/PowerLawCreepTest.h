//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PowerLawCreepStressUpdate.h"

template <bool is_ad>
class PowerLawCreepTestTempl : public PowerLawCreepStressUpdateTempl<is_ad>
{
public:
  PowerLawCreepTestTempl(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> computeResidual(const GenericReal<is_ad> & effective_trial_stress,
                                             const GenericReal<is_ad> & scalar) override;
  virtual GenericReal<is_ad> computeDerivative(const GenericReal<is_ad> & effective_trial_stress,
                                               const GenericReal<is_ad> & scalar) override;

  virtual GenericReal<is_ad>
  maximumPermissibleValue(const GenericReal<is_ad> & /*effective_trial_stress*/) const override
  {
    return 4.0;
  }

  virtual GenericReal<is_ad>
  minimumPermissibleValue(const GenericReal<is_ad> & /*effective_trial_stress*/) const override
  {
    return 0.0;
  }

  virtual GenericReal<is_ad>
  initialGuess(const GenericReal<is_ad> & /*effective_trial_stress*/) override
  {
    return _initial_guess;
  }

  const int _failure_step;
  const Real _initial_guess;

  using PowerLawCreepStressUpdateTempl<is_ad>::_check_range;
  using PowerLawCreepStressUpdateTempl<is_ad>::_dt;
  using PowerLawCreepStressUpdateTempl<is_ad>::_fe_problem;
  using PowerLawCreepStressUpdateTempl<is_ad>::_t_step;
};

typedef PowerLawCreepTestTempl<false> PowerLawCreepTest;
typedef PowerLawCreepTestTempl<true> ADPowerLawCreepTest;
