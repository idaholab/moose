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
#include "MooseRandom.h"

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
  /// Function for element-wise multiplication of vectors
  std::vector<Real> elementwiseMultiply(std::vector<Real> & vec1, std::vector<Real> & vec2);
  /// Function to calculate delayed input signal
  Real getDelayedInputSignal();
  /// Function to calculate R vector
  virtual vector<Real> getRVector();
  /// Function to calculate integral term
  Real getIntegral(std::vector<Real> integrand);

  /// A postprocessor used as the sensor input signal
  const PostprocessorValue & _input_signal;
  /// The drift function to be evaluated and returned
  const Function & _drift_function;
  /// Size of vector to be stored
  const Real _vector_size;
  /// The old value of the postprocessor
  const PostprocessorValue & _pp_old;
  /// Efficiency function
  const Function & _efficiency_function;
  /// Noise standard deviation function
  const Function & _noise_std_dev_function;
  /// Delay function
  const Function & _delay_function;
  /// Signal to noise function
  const Function & _signalToNoise_function;
  /// Uncertainty std dev function
  const Function & _uncertainty_std_dev_function;
  /// Function R for integration term
  const Function & _R_function;
  /// A weighing factor for the proportional term
  const Real _proportional_weight;
  /// A weighing factor for the integral term
  const Real _integral_weight;
  /// Time vector for calculating delay
  std::vector<Real> & _time_values;
  /// Input Signal vector for calculating delay
  std::vector<Real> & _input_signal_values;
  /// Vector to store integrand data for numerical integration
  std::vector<Real> & _integrand;
  /// vector to store R function values
  std::vector<Real> & _R_function_values;
  /// To get fixed seed random numbers
  const unsigned int _seed;
  MooseRandom _rng;
  /// for getValue() output
  Real _sensor_value;
  /// the output of the integrand
  Real _integration_value;
  /// Variable to store delay value
  Real _delay_value;
};
