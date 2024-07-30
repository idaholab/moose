//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermocoupleSensorPostprocessor.h"
#include "GeneralSensorPostprocessor.h"
#include "Function.h"
#include "MooseRandom.h"
#include "LinearInterpolation.h"
#include "SplineInterpolation.h"

registerMooseObject("MiscApp", ThermocoupleSensorPostprocessor);

InputParameters
ThermocoupleSensorPostprocessor::validParams()
{
  InputParameters params = GeneralSensorPostprocessor::validParams();
  params.addClassDescription("This is a ThermocoupleSensorPostprocessor for various classes of "
                             "thermocouples, described by the 'thermocouple_type' parameter");
  params.addParam<Real>("proportional_weight", 0, "The weight assigned to the proportional term");
  params.addParam<Real>("integral_weight", 1, "The weight assigned to the integral term");
  return params;
}

ThermocoupleSensorPostprocessor::ThermocoupleSensorPostprocessor(const InputParameters & parameters)
  : GeneralSensorPostprocessor(parameters)
{
  if (isParamSetByUser("R_function"))
    mooseError("In thermocouple postprocessor R function is fixed. If you want to change it, use "
               "GeneralSensorPostprocessor.");
}

void
ThermocoupleSensorPostprocessor::initialize()
{
  // setting seed for random number generator
  _rng.seed(_seed);
  Real drift_value = _drift_function.value(_t);
  Real efficiency_value = _efficiency_function.value(_t);
  Real signalToNoise_value = _signalToNoise_function.value(_t);
  Real noise_std_dev = _noise_std_dev_function.value(_t);
  Real noise_value = _rng.randNormal(0, noise_std_dev);
  Real uncertainty_std_dev = _uncertainty_std_dev_function.value(_t);
  Real uncertainty_value = _rng.randNormal(0, uncertainty_std_dev);

  // if the problem is steady-state
  if (!_fe_problem.isTransient())
    _sensor_value = drift_value +
                    efficiency_value * (_input_signal + signalToNoise_value * noise_value) +
                    uncertainty_value;

  // if the problem is transient
  else
  {
    _delay_value = _delay_function.value(_t);
    _time_values.push_back(_t);
    _input_signal_values.push_back(_input_signal);

    // Check if the size is greater than 500
    if (_time_values.size() > 500 && _input_signal_values.size() > _vector_size)
    {
      // Remove the first 10 elements
      _time_values.erase(_time_values.begin(), _time_values.begin() + 10);
      _input_signal_values.erase(_input_signal_values.begin(), _input_signal_values.begin() + 10);
    }
    Real _input_signal_delayed = getDelayedInputSignal();

    // computing integral term
    Real term_for_integration = _input_signal + signalToNoise_value * noise_value;
    _integrand.push_back(term_for_integration);
    _integration_value = getIntegral(_integrand);

    // output
    Real proportional_value = _input_signal_delayed + signalToNoise_value * noise_value;
    _sensor_value = drift_value +
                    efficiency_value * (_proportional_weight * proportional_value +
                                        _integral_weight * _integration_value) +
                    uncertainty_value;
  }
}

PostprocessorValue
ThermocoupleSensorPostprocessor::getValue() const
{
  return _sensor_value;
}

vector<Real>
ThermocoupleSensorPostprocessor::getRVector()
{
  _R_function_values.clear();
  // computing vector of exponential term
  for (const auto i : index_range(_time_values))
    _R_function_values.push_back(exp(-(_t - _time_values[i]) / _delay_value) / _delay_value);
  return _R_function_values;
}
