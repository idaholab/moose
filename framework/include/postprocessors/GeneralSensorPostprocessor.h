//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

/**
 * A generalized sensor Postprocessor
 */
class GeneralSensorPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  GeneralSensorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  using Postprocessor::getValue;
  virtual PostprocessorValue getValue() const override;

protected:
  /// A postprocessor used as the sensor input signal
  const PostprocessorValue & _input_signal;
  /// The mean value of the noise parameter (mu)
  const Real _noise_mean;
  /// The standard deviation of the noise parameter (sigma)
  const Real _noise_std_dev;
  /// The time delay of the sensor, in seconds
  const Real _delay;
  /// The drift function to be evaluated and returned
  const Function & _drift_function;
  /// A scaling factor
  const Real _scaling_factor;
  /// The old value of the postprocessor
  const PostprocessorValue & _pp_old;
};
