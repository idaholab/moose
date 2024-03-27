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
#include <numeric>

#include "GeneralSensorPostprocessor.h"
#include "Function.h"
#include "MooseRandom.h"
#include "LinearInterpolation.h"
#include "SplineInterpolation.h"

registerMooseObject("MooseApp", GeneralSensorPostprocessor);

InputParameters
GeneralSensorPostprocessor::validParams()   
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<PostprocessorName>("input_signal", "The input signal postprocessor");
  params.addParam<FunctionName>("drift_function", 0.0,
                                "Drift function describing signal drift over time");
  params.addParam<Real>("vector_size", 1000000000.0, "The maximum size of vector to be saved");
  params.addClassDescription("This is a GeneralSensorPostprocessor, and functions as a base class "
                             "for other sensor postprocessors");
  params.addParam<FunctionName>("efficiency_function", 0.8,
                                  "Efficiency function describing efficiency over time");
  params.addParam<FunctionName>("noise_std_dev_function", 0.5,
                                "Standard deviation of noise function describing noise std dev over time");
  params.addParam<FunctionName>("signalToNoise_function", 0.2,
                                "Conversion of signal to noise function describing the conversion rate");
  params.addParam<FunctionName>("delay_function", 0.1,
                                "Delay function describing the delay over time");
  params.addParam<FunctionName>("uncertainty_std_dev_function", 0.1,
                                "Standard deviation of uncertainty function describing uncertainty std dev over time");
  params.addParam<FunctionName>("R_function", 1, "The function R for the integration term.");                              
  params.addParam<Real>("proportional_weight", 0.5, "The weight assigned to the proportional term");
  params.addParam<Real>("integral_weight", 0.5, "The weight assigned to the integral term");
  params.addParam<unsigned int>("seed", 1, "Seed for the random number generator");
  return params;
}

GeneralSensorPostprocessor::GeneralSensorPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _input_signal(getPostprocessorValue("input_signal")),
    _drift_function(getFunction("drift_function")),
    _vector_size(getParam<Real>("vector_size")),
    _pp_old(getPostprocessorValueOld("input_signal")),
    _efficiency_function(getFunction("efficiency_function")),
    _noise_std_dev_function(getFunction("noise_std_dev_function")),
    _delay_function(getFunction("delay_function")),
    _signalToNoise_function(getFunction("signalToNoise_function")),
    _uncertainty_std_dev_function(getFunction("uncertainty_std_dev_function")),
    _R_function(getFunction("R_function")),
    _proportional_weight(getParam<Real>("proportional_weight")),
    _integral_weight(getParam<Real>("integral_weight")),
    _seed(getParam<unsigned int>("seed"))
{
}

void GeneralSensorPostprocessor::initialize() 
{ 
  _time_values.push_back(_t);
  _input_signal_values.push_back(_input_signal);

  // Check if the size is greater than _vector_size
  if (_time_values.size() > _vector_size && _input_signal_values.size() > _vector_size) 
  {
    // Remove the first 10 elements if size is larger
    _time_values.erase(_time_values.begin(), _time_values.begin() + 10);
    _input_signal_values.erase(_input_signal_values.begin(), _input_signal_values.begin() + 10);
  }

  Real drift_value = getDrift();
  Real efficiency_value = getEfficiency();
  Real signalToNoise_value = getSignalToNoiseFactor();
  // setting seed for random number generator
  _rng.seed(0, _seed); 
  Real noise_std_dev = getNoiseStdDev();
  Real noise_value = getNoise(noise_std_dev);
  Real uncertainty_std_dev = getUncertaintyStdDev();
  Real uncertainty_value = getUncertainty(uncertainty_std_dev);
  Real delay_value = getDelay();
  Real _input_signal_delayed = getDelayedInputSignal(delay_value, _time_values, _input_signal_values);

  //computing integral term
  Real term_for_integration = _input_signal; //+ signalToNoise_value * noise_value;  // can change for different tests
  _integrand.push_back(term_for_integration);
  _integration_value = getIntegral(_integrand, _time_values, delay_value);

  // output
  Real proportional_value = _input_signal_delayed + signalToNoise_value * noise_value;
  _sensor_value = drift_value + efficiency_value * (_proportional_weight * proportional_value + _integral_weight * _integration_value) + uncertainty_value;
}

PostprocessorValue
GeneralSensorPostprocessor::getValue() const
{  
  return _sensor_value;
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

Real GeneralSensorPostprocessor::getNoiseStdDev()
{
  // Noise standard deviation as a function of time
  Real noise_std_dev;
  noise_std_dev = _noise_std_dev_function.value(_t);
  return noise_std_dev;
}

Real GeneralSensorPostprocessor::getNoise(Real noise_std_dev)
{
  // Sample a normal distribution centered around the noise mean, with a width of the noise standard deviation
  Real noise_value = 0.0;
  noise_value = _rng.randNormal(0, noise_std_dev);
  return noise_value;
}

Real GeneralSensorPostprocessor::getUncertaintyStdDev()
{
  // Uncertainty standard deviation as a function of time
  Real uncertainty_std_dev;
  uncertainty_std_dev = _uncertainty_std_dev_function.value(_t);
  return uncertainty_std_dev;
}

Real GeneralSensorPostprocessor::getUncertainty(Real uncertainty_std_dev)
{
  // Uncertainty term
  Real uncertainty_value;
  uncertainty_value = _rng.randNormal(0, uncertainty_std_dev);
  return uncertainty_value;
}

Real GeneralSensorPostprocessor::getDelay()
{
  // delay (tau) as a function of time
  Real delay_value = 0.0;
  delay_value = _delay_function.value(_t);
  return delay_value;
}

Real GeneralSensorPostprocessor::getDelayedInputSignal(Real delay_value, std::vector<Real> time_values, std::vector<Real> input_signal_values)
{ 
  // delayed input signal
  Real t_desired = _t - delay_value;
  Real input_signal_delayed;

  if (t_desired < time_values[0])
    input_signal_delayed = 0;

  else if (t_desired == time_values[0])
    input_signal_delayed = _input_signal;

  // linear interpolation
  else if (t_desired > time_values[0] && t_desired <= _t && t_desired >= _t - _dt)
    input_signal_delayed = _input_signal + (t_desired - _t) * (_pp_old - _input_signal) / (-_dt);

  else if (t_desired > time_values[0] && t_desired < _t - _dt) //&& floor(t_desired/_dt)*_dt == time_values[0]
  {
    LinearInterpolation time_and_input_signal(time_values, input_signal_values);
    input_signal_delayed = time_and_input_signal.sample(t_desired);
  }

  else
    cout << "Some error in interpolation values." <<endl;

  return input_signal_delayed;
}

Real GeneralSensorPostprocessor::getIntegral(std::vector<Real> integrand, std::vector<Real> time_values, Real delay_value)
{
  Real integration_value;
  std::vector<Real> R_function_values;
  // computing vector of R function values
  for (size_t i = 0; i < time_values.size(); ++i) 
      R_function_values.push_back(_R_function.value(_t)); 

  // calculate the total integrand vector including previous integrand and exponential terms
  integrand = elementwiseMultiply(integrand, R_function_values);

  // if there are more than two datapoints, do simpson's 3/8 rule for integration
  // else do trapezoidal rule
  if (time_values.size() > 2)
  {
    SplineInterpolation spline(time_values, integrand);
    // number of intervals
    Real n; 

    // if number of datapoints is less than 30, use spline interpolation to get 30 intervals
    if (time_values.size() < 30)
    {
      n = 30;
    }
    else 
    {
      n = time_values.size();
      // if interval is not a multiple of 3, make it
      while (static_cast<int>(n) % 3 != 0)
      {
        n = n+1;
      }
    }
  
    Real h = (time_values.back() - time_values[0]) / n; // distance between time values
    cout << "h is : " << h << endl;
    // time vector for simpson integration
    vector<Real> time_vec_simp;
    // integrand vector for simpson integration
    vector<Real> integrand_vec_simp;

    for (Real i = 0; i < n+1; i++)
    {
      Real new_time = time_values[0] + i*h;
      time_vec_simp.push_back(new_time);
      Real new_integrand = spline.SplineInterpolation::sample(new_time);
      integrand_vec_simp.push_back(new_integrand);
    }

    // initialize integral with endpoints
    integration_value = integrand_vec_simp[0] + integrand_vec_simp[n];

    // Sum for points not at endpoints
    for (int i = 1; i < n; ++i) 
    {
        if (i % 3 == 0) 
        { // Every third point
            integration_value += 2 * integrand_vec_simp[i];
        } 
        else 
        {
            integration_value += 3 * integrand_vec_simp[i];
        }
    }

    // Multiply by 3h/8
    integration_value *= (3 * h) / 8.0;
  }

  else
  {
    // performing integration by trapezoidal rule, not accurate
    //LinearInterpolation object with two vectors
    LinearInterpolation integral(time_values, integrand);
    //integrate the vectors
    integration_value = integral.integrate(); 
  }

  return integration_value;
}