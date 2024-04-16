//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermocoupleSensorPostprocessor.h"

registerMooseObject("MooseApp", ThermocoupleSensorPostprocessor);

InputParameters
ThermocoupleSensorPostprocessor::validParams()
{
  InputParameters params = GeneralSensorPostprocessor::validParams();
  params.addParam<string>("thermocouple_type", "B", "The type of thermocouple (B, E, J or K)");
  params.addParam<string>("method", "numerical", "The equation to be used (lumped or numerical).");
  params.addClassDescription("This is a ThermocoupleSensorPostprocessor for various classes of "
                             "thermocouples, described by the 'thermocouple_type' parameter");
  params.addParam<Real>("proportional_weight", 0, "The weight assigned to the proportional term");
  params.addParam<Real>("integral_weight", 1, "The weight assigned to the integral term");
  return params;
}

ThermocoupleSensorPostprocessor::ThermocoupleSensorPostprocessor(const InputParameters & parameters)
  : GeneralSensorPostprocessor(parameters),
  _thermocouple_type(getParam<string>("thermocouple_type")),
  _method(getParam<string>("method"))
{
}

void ThermocoupleSensorPostprocessor::initialize() 
{ 
  Real drift_value;
  Real efficiency_value;
  Real signalToNoise_value;
  Real noise_value;
  Real noise_std_dev;
  Real uncertainty_value;
  Real uncertainty_std_dev;
  Real delay_value;

  // setting values for thermocouple types B, J, E, K
  // current values are placeholders
  if (_thermocouple_type == "B")
  {
    // B type, temperature in degree celcius
    if (_input_signal >= 0 && _input_signal < 630.615)
    {
      drift_value = 1;
      efficiency_value = 1;
      signalToNoise_value = 1;
      noise_std_dev = 1;
      noise_value = getNoise(noise_std_dev);
      uncertainty_std_dev = 1;
      uncertainty_value = getUncertainty(uncertainty_std_dev);
      delay_value = 1;
    }
    else
    {
      mooseWarning("Temperature is outside this thermocouple type's operation range.");
    }
  }

  else if (_thermocouple_type == "E")
  {
    // E type, temperature in degree celcius
    if (_input_signal >= -270 && _input_signal < 0)
    {
      drift_value = 1;
      efficiency_value = 1;
      signalToNoise_value = 1;
      noise_std_dev = 1;
      noise_value = getNoise(noise_std_dev);
      uncertainty_std_dev = 1;
      uncertainty_value = getUncertainty(uncertainty_std_dev);
      delay_value = 1;
    }
    else if (_input_signal >= 0 && _input_signal <= 1000)
    {
      drift_value = 1;
      efficiency_value = 1;
      signalToNoise_value = 1;
      noise_std_dev = 1;
      noise_value = getNoise(noise_std_dev);
      uncertainty_std_dev = 1;
      uncertainty_value = getUncertainty(uncertainty_std_dev);
      delay_value = 1;
    }
    else
    {
      mooseWarning("Temperature is outside this thermocouple type's operation range.");
    }
  }

  else if (_thermocouple_type == "J")
  {
    // J type, temperature in degree celcius
    if (_input_signal >= -210 && _input_signal < 760)
    {
      drift_value = 1;
      efficiency_value = 1;
      signalToNoise_value = 1;
      noise_std_dev = 1;
      noise_value = getNoise(noise_std_dev);
      uncertainty_std_dev = 1;
      uncertainty_value = getUncertainty(uncertainty_std_dev);
      delay_value = 1;
    }
    else if (_input_signal >= 760 && _input_signal <= 1200)
    {
      drift_value = 1;
      efficiency_value = 1;
      signalToNoise_value = 1;
      noise_std_dev = 1;
      noise_value = getNoise(noise_std_dev);
      uncertainty_std_dev = 1;
      uncertainty_value = getUncertainty(uncertainty_std_dev);
      delay_value = 1;
    }
    else
    {
      mooseWarning("Temperature is outside this thermocouple type's operation range.");
    }
  }

  else if (_thermocouple_type == "K")
  {
    // K type, temperature in degree celcius
    if (_input_signal >= -210 && _input_signal < 0)
    {
      drift_value = 2;
      efficiency_value = 1;
      signalToNoise_value = 5;
      noise_std_dev = 1;
      noise_value = getNoise(noise_std_dev);
      uncertainty_std_dev = 1;
      uncertainty_value = getUncertainty(uncertainty_std_dev);
      delay_value = 0.3;
    }
    else if (_input_signal >= 0 && _input_signal <= 1372)
    {
      drift_value = 2;
      efficiency_value = 1;
      signalToNoise_value = 5;
      noise_std_dev = 1;
      noise_value = getNoise(noise_std_dev);
      uncertainty_std_dev = 1;
      uncertainty_value = getUncertainty(uncertainty_std_dev);
      delay_value = 1;
    }
    else
    {
      mooseWarning("Temperature is outside this thermocouple type's operation range.");
    }
  }

  else
    mooseError("The thermocouple you want is invalid or not available.");

  // if the problem is steady-state
  if (!_fe_problem.isTransient())
  {
    _sensor_value = drift_value + efficiency_value * (_input_signal + signalToNoise_value * noise_value) + uncertainty_value;
  }

  // if the problem is transient
  else
  {
    _time_values.push_back(_t);
    _input_signal_values.push_back(_input_signal);

    // Check if the size is greater than 500
    if (_time_values.size() > 500 && _input_signal_values.size() > _vector_size) 
    {
      // Remove the first 10 elements
      _time_values.erase(_time_values.begin(), _time_values.begin() + 10);
      _input_signal_values.erase(_input_signal_values.begin(), _input_signal_values.begin() + 10);
    }
    Real _input_signal_delayed = getDelayedInputSignal(delay_value, _time_values, _input_signal_values);

    if (_method == "numerical")
    {
      cout << "doing something" << endl;
      //computing integral term
      Real term_for_integration = _input_signal; //+ signalToNoise_value * noise_value;  // can change for different tests
      _integrand.push_back(term_for_integration);
      _integration_value = getIntegral(_integrand, _time_values, delay_value);

      // output
      Real proportional_value = _input_signal_delayed + signalToNoise_value * noise_value;
      _sensor_value = drift_value + efficiency_value * (_proportional_weight * proportional_value + _integral_weight * _integration_value) + uncertainty_value;
    }

    else if (_method == "lumped")
    {
      cout << "doing something" << endl;
      if (_t == _time_values[0])
      {
        _sensor_value = uncertainty_value + 0;
      }
      else
      {
        _sensor_value = _sensor_value_old + ((_input_signal /*+ signalToNoise_value * noise_value*/) - _sensor_value_old) * (1 - exp(-_t/delay_value)) + uncertainty_value;
      }
      _sensor_value_old = _sensor_value - uncertainty_value;
    }

    else
      mooseError("The entered method is invalid.");
  }
}

PostprocessorValue
ThermocoupleSensorPostprocessor::getValue() const
{
  return _sensor_value;
}

Real ThermocoupleSensorPostprocessor::getIntegral(std::vector<Real> integrand, std::vector<Real> time_values, Real delay_value)
{
  Real integration_value;
  std::vector<Real> exponential_term;
  // computing vector of exponential term
  for (size_t i = 0; i < time_values.size(); ++i) 
      exponential_term.push_back(exp(-(_t - time_values[i])/delay_value)/delay_value); 

  // calculate the total integrand vector including previous integrand and exponential terms
  integrand = elementwiseMultiply(integrand, exponential_term);

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
  
    // distance between time values
    Real h = (time_values.back() - time_values[0]) / n; 
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
    // but we do not have enough no. of points either
    //LinearInterpolation object with two vectors
    LinearInterpolation integral(time_values, integrand);
    //integrate the vectors
    integration_value = integral.integrate(); 
  }

  return integration_value;
}