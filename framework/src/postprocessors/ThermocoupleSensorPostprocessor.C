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
  MooseEnum thermocoupleType("B E J K", "B");
  params.addParam<MooseEnum>("thermocouple_type", thermocoupleType, "Thermocouple type");
  params.addClassDescription("This is a ThermocoupleSensorPostprocessor for various classes of "
                             "thermocouples, described by the 'thermocouple_type' parameter");
  return params;
}

ThermocoupleSensorPostprocessor::ThermocoupleSensorPostprocessor(const InputParameters & parameters)
  : GeneralSensorPostprocessor(parameters),
    _thermocouple(getParam<MooseEnum>("thermocouple_type").getEnum<ThermocoupleType>())
{
}

PostprocessorValue
ThermocoupleSensorPostprocessor::getValue() const
{
  //-------------DRIFT-------------
  // Evaluate drift function at the current time
  Real drift_value = 0.0;
  drift_value = _drift_function.value(_t);

  //-------------EFFICIENCY-------------
  // Efficiency function at the current time
  Real efficiency_value = 0.8;    // default efficiency is taken as 80%
  efficiency_value = _efficiency_function.value(_t); 

  //-------------SIGNAL TO NOISE FACTOR-------------
  // Signal to noise factor as a function of time
  Real signalToNoise_value = 0.2; // default value in MOOSE
  signalToNoise_value = _signalToNoise_function.value(_t);

  //-------------NOISE-------------
  // Sample a normal distribution centered around the noise mean, with a width of the noise standard deviation
  Real noise_value = 0.0;
  // noise std dev as a function of time
  Real noise_std_dev = 0.5; // default value
  noise_std_dev = _noise_std_dev_function.value(_t);
  //Real random_number = Sampler::getRand1(1,0,1); // NO NEED
  //MooseRandom::seed(10); 
  noise_value = MooseRandom::randNormal(0, noise_std_dev);

  //-------------UNCERTAINTY-------------
  // Uncertainty term
  Real uncertainty_value = 0.0;
  Real uncertainty_std_dev = 0.1; // default value
  uncertainty_std_dev = _uncertainty_std_dev_function.value(_t);
  //MooseRandom::seed(100); 
  uncertainty_value = MooseRandom::randNormal(0, uncertainty_std_dev);

  Real pp_value =  drift_value + efficiency_value * (_input_signal + signalToNoise_value * noise_value);
  switch (_thermocouple)
  {
    // Switch cases based on thermocouple temperature ranges
    case ThermocoupleType::B:
    {
      if (pp_value < 273.15 || pp_value > 2093.15)
        mooseWarning("Temperature is outside this thermocouple type's operation range.");
    }
    case ThermocoupleType::E:
    {
      if (pp_value < -543.15 || pp_value > 1273.15)
        mooseWarning("Temperature is outside this thermocouple type's operation range.");
    }
    case ThermocoupleType::J:
    {
      if (pp_value < -483.15 || pp_value > 1473.15)
        mooseWarning("Temperature is outside this thermocouple type's operation range.");
    }
    case ThermocoupleType::K:
    {
      if (pp_value < -543.15 || pp_value > 1645.15)
        mooseWarning("Temperature is outside this thermocouple type's operation range.");
    }
    default:
      mooseError("Error, invalid thermocouple type.");
  }
  // If the timestep value is less than or equal to the sensor delay, use the previous value of
  // the postprocessor
  // if (_t <= _delay)
  //   return _pp_old;
  // else
    return pp_value;
}