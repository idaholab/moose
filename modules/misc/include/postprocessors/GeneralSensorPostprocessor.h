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
  // removes elements before t-tau-dt
  void removeValuesBeforeTime(std::vector<double>& _time_values, std::vector<double>& _input_signal_values, std::vector<Real>& exp_values, double t_desired) const;
  // Function to perform linear interpolation
  Real linearInterpolation(Real x1, Real x2, Real y1, Real y2, Real x);
  // Function that inearly interpolates from two vectors to find a desired value
  Real linearInterpolationInVectors(std::vector<Real>& time, std::vector<Real>& y, Real desired_time);
  // Function for element-wise multiplication of vectors
  std::vector<Real> elementwiseMultiply(std::vector<Real>& vec1, std::vector<Real>& vec2); 
  // Function to calculate drift
  Real getDrift();
  // Function to calculate efficiency
  Real getEfficiency();
  // Function to calculate signal to noise factor
  Real getSignalToNoiseFactor();
  // Function to calculate noise
  Real getNoise();
  // Function to calculate uncertainty
  Real getUncertainty();
  // Function to get delay value
  Real getDelay();
  // Function to calculate delayed input signal
  Real getDelayedInputSignal(Real _input_signal_initial, Real delay_value);
  // Function to calculate integral term
  Real getIntegral(Real for_int_initial, std::vector<Real> _for_int, std::vector<Real> _time_values, Real delay_value);


  /// A postprocessor used as the sensor input signal
  const PostprocessorValue & _input_signal;
  /// The drift function to be evaluated and returned
  const Function & _drift_function;
  /// A scaling factor
  const Real _scaling_factor;
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
    /// A weighing factor for the proportional term
  const Real _proportional_weight;
  /// A weighing factor for the integral term
  const Real _integral_weight;
  /// Time vector for calculating delay
  std::vector<Real> time_values;
  /// Input Signal vector for calculating delay
  std::vector<Real> _input_signal_values;
  /// Vector to store integrand data for numerical integration
  std::vector<Real> integrand;
  /// To get fixed seed random numbers
  unsigned int seed;
  MooseRandom _rng;
  /// for getValue() output
  Real sensor_value;
};
