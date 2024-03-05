//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include <vector>
#include <cmath>
#include <algorithm>

#include "GeneralSensorPostprocessor.h"
#include "Function.h"
#include "MooseRandom.h"
#include "LinearInterpolation.h"
#include "libmesh/dense_vector.h"

registerMooseObject("MooseApp", GeneralSensorPostprocessor);

InputParameters
GeneralSensorPostprocessor::validParams()   
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<PostprocessorName>("input_signal", "The input signal postprocessor");
  params.addParam<FunctionName>("drift_function", 0.0,
                                "Drift function describing signal drift over time");
  params.addParam<Real>("vector_size", 500.0, "The maximum size of vector to be saved");
  params.addParam<Real>("scaling_factor", 1.0, "The scaling factor");
  params.addClassDescription("This is a GeneralSensorPostprocessor, and functions as a base class "
                             "for other sensor postprocessors");
  params.addParam<FunctionName>("efficiency_function", 0.8,
                                  "Efficiency function describing efficiency over time");
  params.addParam<FunctionName>("noise_std_dev_function", 0.5,
                                "Standard deviation of noise function describing noise std dev over time");
  params.addParam<FunctionName>("signalToNoise_function", 0.2,
                                "Conversion of signal to noise function describing the conversion rate");
  params.addParam<FunctionName>("delay_function", 0.0,
                                "Delay function describing the delay over time");
  params.addParam<FunctionName>("uncertainty_std_dev_function", 0.1,
                                "Standard deviation of uncertainty function describing uncertainty std dev over time");
  params.addParam<Real>("proportional_weight", 0.5, "The weight assigned to the proportional term");
  params.addParam<Real>("integral_weight", 0.5, "The weight assigned to the integral term");
  params.addParam<unsigned int>("seed", 1, "Seed for the random number generator");
  return params;
}

GeneralSensorPostprocessor::GeneralSensorPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _input_signal(getPostprocessorValue("input_signal")),
    _drift_function(getFunction("drift_function")),
    _scaling_factor(getParam<Real>("scaling_factor")),
    _vector_size(getParam<Real>("vector_size")),
    _pp_old(getPostprocessorValueOld("input_signal")),
    _efficiency_function(getFunction("efficiency_function")),
    _noise_std_dev_function(getFunction("noise_std_dev_function")),
    _delay_function(getFunction("delay_function")),
    _signalToNoise_function(getFunction("signalToNoise_function")),
    _uncertainty_std_dev_function(getFunction("uncertainty_std_dev_function")),
    _proportional_weight(getParam<Real>("proportional_weight")),
    _integral_weight(getParam<Real>("integral_weight")),
    seed(getParam<unsigned int>("seed"))
{
}

void GeneralSensorPostprocessor::initialize() 
{ 
  time_values.push_back(_t);
  _input_signal_values.push_back(_input_signal);

  // Check if the size is greater than 500
  if (time_values.size() > 500 && _input_signal_values.size() > _vector_size) 
  {
    // Remove the first 10 elements
    time_values.erase(time_values.begin(), time_values.begin() + 10);
    _input_signal_values.erase(_input_signal_values.begin(), _input_signal_values.begin() + 10);
  }

  Real drift_value = getDrift();
  Real efficiency_value = getEfficiency();
  Real signalToNoise_value = getSignalToNoiseFactor();
  // setting seed for random number generator
  _rng.seed(0, seed);
  Real noise_value = getNoise();
  Real uncertainty_value = getUncertainty();

  //-------------Misc-------------
  Real _input_signal_initial;

  if (_t == 0)
    _input_signal_initial = _input_signal;

  Real delay_value = getDelay();
  Real _input_signal_delayed = getDelayedInputSignal(_input_signal_initial, delay_value);

   //-------------Misc-------------
  Real for_int_initial;

  if (_t == 0)
    for_int_initial = (_input_signal_delayed + signalToNoise_value * 1) ; // + signalToNoise_value * normal_value;

 //-------------INTEGRAL-------------
  Real for_int = (_input_signal_delayed + signalToNoise_value * 1) ; // + signalToNoise_value * normal_value;
  integrand.push_back(for_int);
  Real integration_value = getIntegral(for_int_initial, integrand, time_values, delay_value);

  //-------------OUTPUT-------------
  Real proportional_value = _input_signal_delayed + signalToNoise_value * 1;
  sensor_value = drift_value + efficiency_value * (_proportional_weight * proportional_value + _integral_weight * integration_value);
}

PostprocessorValue
GeneralSensorPostprocessor::getValue() const
{  
  return sensor_value;
}

void GeneralSensorPostprocessor::removeValuesBeforeTime(std::vector<Real>& time_values, std::vector<Real>& _input_signal_values, std::vector<Real>& exp_values, Real t_desired) const 
{
  auto it = time_values.begin();

    // Find the iterator pointing to the element immediately before t_desired
    auto lastBeforeTDesired = time_values.begin();

    while (it != time_values.end())
    {
        if (*it < t_desired)
        {
            lastBeforeTDesired = it;
            ++it;
        }
        else
            // Break the loop when we find the first element greater than or equal to t_desired
            break;
    }

    // Erase elements from the beginning after (and including) the iterator pointing to the element before t_desired
    time_values.erase(time_values.begin(), lastBeforeTDesired+1);
    _input_signal_values.erase(_input_signal_values.begin(), _input_signal_values.begin() + std::distance(time_values.begin(), lastBeforeTDesired+1));
    exp_values.erase(exp_values.begin(), lastBeforeTDesired+1);
}

Real GeneralSensorPostprocessor::linearInterpolation(Real x1, Real x2, Real y1, Real y2, Real x)
{
    Real y;
    y = y1 + (x - x1) * (y2 - y1) / (x2 - x1);
    return y;
}

Real GeneralSensorPostprocessor::linearInterpolationInVectors(std::vector<Real>& time, std::vector<Real>& y, Real desired_time)
{
    Real desired_y;
    //----------begin-----------
    // finding the index of t_1
    Real t_1 = floor(desired_time/_dt)*_dt;
    // Find the iterator for t_1
    auto it_1 = find(time.begin(), time.end(), t_1);
    // Find the index_1
    size_t index_1 = distance(time.begin(), it_1);
    Real fi_1 =  y[index_1];
    // finding the index of t_2
    Real t_2 = ceil(desired_time/_dt)*_dt;
    // Find the iterator for t_2
    auto it_2 = find(time.begin(), time.end(), t_2);
    // Find the index_2
    size_t index_2 = distance(time.begin(), it_2);
    Real fi_2 =  y[index_2];
    // Linear interpolation to calculate delayed input signal
    desired_y = fi_1 + (desired_time - t_1) * (fi_2 - fi_1) / (t_2-t_1);
    //------------end------------
    return desired_y;
}

std::vector<Real> GeneralSensorPostprocessor::elementwiseMultiply(std::vector<Real>& vec1, std::vector<Real>& vec2) 
{
    // Ensure both vectors have the same size
    if (vec1.size() != vec2.size()) 
    {
        throw std::runtime_error("Vectors must have the same size for element-wise multiplication.");
    }
    // Create a result vector
    std::vector<Real> result;
    // Perform element-wise multiplication
    for (size_t i = 0; i < vec1.size(); ++i) 
    {
        result.push_back(vec1[i] * vec2[i]);
    }
    return result;
}

Real GeneralSensorPostprocessor::getDrift()
{
  Real drift_value;
  // Evaluate drift function at the current time
  drift_value = _drift_function.value(_t);
  return drift_value;
}

Real GeneralSensorPostprocessor::getEfficiency()
{
  Real efficiency_value;
  // Evaluate drift function at the current time
  efficiency_value = _efficiency_function.value(_t);
  return efficiency_value;
}

Real GeneralSensorPostprocessor::getSignalToNoiseFactor()
{
  // Signal to noise factor as a function of time
  Real signalToNoise_value;
  signalToNoise_value = _signalToNoise_function.value(_t);
  return signalToNoise_value;
}

Real GeneralSensorPostprocessor::getNoise()
{
  // Sample a normal distribution centered around the noise mean, with a width of the noise standard deviation
  Real noise_value = 0.0;
  // noise std dev as a function of time
  Real noise_std_dev;
  noise_std_dev = _noise_std_dev_function.value(_t);
  noise_value = _rng.randNormal(0, 0, noise_std_dev);
  return noise_value;
}

Real GeneralSensorPostprocessor::getUncertainty()
{
  // Uncertainty term
  Real uncertainty_value;
  Real uncertainty_std_dev;
  uncertainty_std_dev = _uncertainty_std_dev_function.value(_t);
  uncertainty_value = _rng.randNormal(0, 0, uncertainty_std_dev);
  return uncertainty_value;
}

Real GeneralSensorPostprocessor::getDelay()
{
  // delay (tau) as a function of time
  Real delay_value = 0.0;
  delay_value = _delay_function.value(_t);
  return delay_value;
}

Real GeneralSensorPostprocessor::getDelayedInputSignal(Real _input_signal_initial, Real delay_value)
{ 
  // delayed input signal
  Real t_desired = _t - delay_value;
  Real _input_signal_delayed;

  if (t_desired < 0)
    _input_signal_delayed = 0;

  else if (t_desired == 0)
    _input_signal_delayed = _input_signal_initial;

  // linear interpolation
  else if (t_desired > 0 && t_desired <= _t && t_desired >= _t - _dt)
    _input_signal_delayed = _input_signal + (t_desired - _t) * (_pp_old - _input_signal) / (-_dt);

  else if (t_desired > 0 && t_desired < _t - _dt && floor(t_desired/_dt)*_dt == 0)
    _input_signal_delayed = _input_signal_initial + t_desired * (_input_signal_values[0] - _input_signal_initial) / time_values[0];

  else if (t_desired > 0 && t_desired < _t - _dt && floor(t_desired/_dt)*_dt != 0)
    _input_signal_delayed = linearInterpolationInVectors(time_values, _input_signal_values, t_desired);

  else
    cout << "Some error in interpolation values." <<endl;

  return _input_signal_delayed;
}

Real GeneralSensorPostprocessor::getIntegral(Real for_int_initial, std::vector<Real> integrand, std::vector<Real> time_values, Real delay_value)
{
  std::vector<Real> exponential_term;
  Real integration_value;
  // computing vector of exponential term
  for (size_t i = 0; i < time_values.size(); ++i) 
        exponential_term.push_back(exp(-(_t - time_values[i])/delay_value)); 
    // exact exponential at delay time
    Real exponential_term_at_delay = exp(-(_t - delay_value)/delay_value);

  if (_t > delay_value && delay_value > _dt)
  {
    // linearly interpolation of integrand at delay time
    Real integrand_at_delay = linearInterpolationInVectors(time_values, integrand, delay_value);
    // push the delay and interpolated signal value into the new vector
    integrand.push_back(integrand_at_delay);
    time_values.push_back(delay_value);
    exponential_term.push_back(exponential_term_at_delay);
    // sort the vector in aascending order
    std::sort(integrand.begin(), integrand.end());
    std::sort(time_values.begin(), time_values.end());
    std::sort(exponential_term.begin(), exponential_term.end());
    // New vector for integration with all elements before delay removed from __for_int and exponential_term
    removeValuesBeforeTime(time_values, integrand, exponential_term, delay_value); 
    // calculate the total integrand vector including previous integrand and exponential terms
    integrand = elementwiseMultiply(integrand, exponential_term);
    //integrand = integrand.dot(exponential_term);
    //LinearInterpolation object with two vectors
    LinearInterpolation integral(time_values, integrand);
    //integrate the vectors
    integration_value = integral.integrate(); 
  }

  else if (_t > delay_value && delay_value < _dt)
  {
    // linear interpolation of for_int at delay time
    Real integrand_at_delay = linearInterpolation(0, time_values[0], for_int_initial, integrand[0], delay_value);
    // push the delay and interpolated signal value into the new vector
    integrand.push_back(integrand_at_delay);
    time_values.push_back(delay_value);
    exponential_term.push_back(exponential_term_at_delay);
    // sort the vector in aascending order
    std::sort(integrand.begin(), integrand.end());
    std::sort(time_values.begin(), time_values.end());
    std::sort(exponential_term.begin(), exponential_term.end());
    integrand = elementwiseMultiply(integrand, exponential_term);
    //integrand = integrand.dot(exponential_term);
    //LinearInterpolation object with two vectors
    LinearInterpolation integral(time_values, integrand);
    //integrate the vectors
    integration_value = integral.integrate();
  }

  else 
    integration_value = 0;

  return integration_value;
}