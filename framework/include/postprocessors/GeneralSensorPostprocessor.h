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
#include <iostream>
using namespace std;

/**
 * A generalized sensor Postprocessor
 */
class GeneralSensorPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  GeneralSensorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  using Postprocessor::getValue;
  virtual PostprocessorValue getValue() const override;

protected:
  /// function I need
  // removes elements before t-tau-dt
  void removeValuesBeforeTime(std::vector<double>& _time_values, std::vector<double>& _input_signal_values, double t_desired) const;
  // Function to perform linear interpolation
  Real linearInterpolation(Real x1, Real x2, Real y1, Real y2, Real x);
  // describe
  Real linearInterpolationInVectors(std::vector<Real>& time, std::vector<Real>& y, Real desired_time);

  /// A postprocessor used as the sensor input signal
  const PostprocessorValue & _input_signal;
  // **** const PostprocessorValue & _t;
  /// The mean value of the noise parameter (mu)
  //const Real _noise_mean;
  /// The standard deviation of the noise parameter (sigma)
  //const Real _noise_std_dev;
  /// The time delay of the sensor, in seconds
  //const Real _delay;
  /// The drift function to be evaluated and returned
  const Function & _drift_function;
  /// A scaling factor
  const Real _scaling_factor;
  /// Size of vector to be stored
  const Real _vector_size;
  /// End time of simulation
  const Real _end_time;
  /// The old value of the postprocessor
  const PostprocessorValue & _pp_old;
  /// New stuff
  /// Signal to noise factor
  //const Real _signalToNoise_value;

  /// new stuff
  /// Efficiency function
  const Function & _efficiency_function;
  /// Noise std dev function
  const Function & _noise_std_dev_function;
  /// Delay function
  const Function & _delay_function;
  /// Signal to noise function
  const Function & _signalToNoise_function;
  /// Uncertainty std dev function
  const Function & _uncertainty_std_dev_function;
  /// for delay
  std::vector<Real> _time_values;
  std::vector<Real> _input_signal_values;
  std::vector<Real> _time_values_again;
  std::vector<Real> _for_int;
  /// for getValue()
  Real sensor_value;
};
