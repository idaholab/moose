//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralSensorPostprocessor.h"
#include "Function.h"
#include "MooseRandom.h"

registerMooseObject("MooseApp", GeneralSensorPostprocessor);

InputParameters
GeneralSensorPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<PostprocessorName>("input_signal", "The input signal postprocessor");
  params.addParam<Real>("noise_mean", 0.5, "The mean value of sensor noise");
  params.addParam<Real>(
      "noise_std_dev", 0.1, "Standard deviation of noise (for a Gaussian)");
  params.addParam<Real>("delay", 0.1, "The delay value (s)");
  params.addParam<FunctionName>("drift_function",
                                "Drift function describing signal drift over time");
  params.addParam<Real>("scaling_factor", 1.0, "The scaling factor");
  params.addClassDescription("This is a GeneralSensorPostprocessor, and functions as a base class "
                             "for other sensor postprocessors");
  return params;
}

GeneralSensorPostprocessor::GeneralSensorPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _input_signal(getPostprocessorValue("input_signal")),
    _noise_mean(getParam<Real>("noise_mean")),
    _noise_std_dev(getParam<Real>("noise_std_dev")),
    _delay(getParam<Real>("delay")),
    _drift_function(getFunction("drift_function")),
    _scaling_factor(getParam<Real>("scaling_factor")),
    _pp_old(getPostprocessorValueOld("input_signal"))
{
}

PostprocessorValue
GeneralSensorPostprocessor::getValue() const
{
  // Evaluate drift function at the current time
  Real drift_value = 0.0;
  drift_value = _drift_function.value(_t);

  // Sample a normal distribution centered around the noise mean, with a width of the noise standard
  // deviation
  Real normal_value = 0.0;
  normal_value = MooseRandom::randNormal(_noise_mean, _noise_std_dev);

  // If the timestep value is less than or equal to the sensor delay, use the previous value of the
  // postprocessor
  if (_dt <= _delay)
    return _pp_old;
  else
    return _scaling_factor * (_input_signal + normal_value + drift_value);
}
