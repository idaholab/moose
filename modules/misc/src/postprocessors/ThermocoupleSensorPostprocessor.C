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
  params.addClassDescription("This is a ThermocoupleSensorPostprocessor for various classes of "
                             "thermocouples, described by the 'thermocouple_type' parameter");
  return params;
}

ThermocoupleSensorPostprocessor::ThermocoupleSensorPostprocessor(const InputParameters & parameters)
  : GeneralSensorPostprocessor(parameters),
  _thermocouple_type(getParam<string>("thermocouple_type"))
{
}

void ThermocoupleSensorPostprocessor::initialize() 
{ 
  // conversion of T to emf
  // Evaluating the emf
  Real e;
  
  
  
  
  
  
  time_values.push_back(_t);
  _input_signal_values.push_back(_input_signal);

  // Check if the size is greater than 500
  if (time_values.size() > 500 && _input_signal_values.size() > _vector_size) 
  {
    // Remove the first 10 elements
    time_values.erase(time_values.begin(), time_values.begin() + 10);
    _input_signal_values.erase(_input_signal_values.begin(), _input_signal_values.begin() + 10);
  }

  Real drift_value;
  Real efficiency_value;
  Real signalToNoise_value;
  Real noise_value;
  Real uncertainty_value;
  Real delay_value;

  if (_thermocouple_type == "B")
  {
    // B type, temperature in degree celcius
    if (_input_signal >= 0 && _input_signal < 630.615)
    {
      e = evaluateEMF(_input_signal, coeff_C_thermo_type_B_0_to_630);
    }
    else if (_input_signal >= 630.615 && _input_signal <= 1820)
    {
      e = evaluateEMF(_input_signal, coeff_C_thermo_type_B_630_to_1820);
    }
    else
    {
      mooseWarning("Temperature is outside this thermocouple type's operation range.");
    }
    
    drift_value = 1;
    efficiency_value = 1;
    signalToNoise_value = 1;
    noise_value = 1;
    uncertainty_value = 1;
    delay_value = 1;

    // add noise, drift, delay etc to e

    // convert e back to T
  }

  else if (_thermocouple_type == "E")
  {
    // E type, temperature in degree celcius
    if (_input_signal >= -270 && _input_signal < 0)
    {
      e = evaluateEMF(_input_signal, coeff_C_thermo_type_E_minus_270_to_0);
    }
    else if (_input_signal >= 0 && _input_signal <= 1000)
    {
      e = evaluateEMF(_input_signal, coeff_C_thermo_type_E_0_to_1000);
    }
    else
    {
      mooseWarning("Temperature is outside this thermocouple type's operation range.");
    }

    drift_value = 1;
    efficiency_value = 1;
    signalToNoise_value = 1;
    noise_value = 1;
    uncertainty_value = 1;
    delay_value = 1;

    // add noise, drift, delay etc to e

    // convert e back to T
  }

  else if (_thermocouple_type == "J")
  {
    // J type, temperature in degree celcius
    if (_input_signal >= -210 && _input_signal < 760)
    {
      e = evaluateEMF(_input_signal, coeff_C_thermo_type_J_minus_210_to_760);
    }
    else if (_input_signal >= 760 && _input_signal <= 1200)
    {
      e = evaluateEMF(_input_signal, coeff_C_thermo_type_J_760_to_1200);
    }
    else
    {
      mooseWarning("Temperature is outside this thermocouple type's operation range.");
    }
    drift_value = 1;
    efficiency_value = 1;
    signalToNoise_value = 1;
    noise_value = 1;
    uncertainty_value = 1;
    delay_value = 1;

    // add noise, drift, delay etc to e

    // convert e back to T
  }

  else if (_thermocouple_type == "K")
  {
    // K type, temperature in degree celcius
    if (_input_signal >= -210 && _input_signal < 0)
    {
      e = evaluateEMF(_input_signal, coeff_C_thermo_type_K_minus_270_to_0);
    }
    else if (_input_signal >= 0 && _input_signal <= 1372)
    {
      e = evaluateEMFTypeK(_input_signal, coeff_C_thermo_type_K_0_to_1372, coeff_A_thermo_type_K_0_to_1372);
    }
    else
    {
      mooseWarning("Temperature is outside this thermocouple type's operation range.");
    }

    drift_value = 1;
    efficiency_value = 1;
    signalToNoise_value = 1;
    noise_value = 1;
    uncertainty_value = 1;
    delay_value = 1;

    // add noise, drift, delay etc to e

    // convert e back to T
  }

  else
    mooseError("The thermocouple you want is invalid or not available.");

  //-------------Misc-------------
  Real _input_signal_initial;

  if (_t == 0)
    _input_signal_initial = _input_signal;

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
  //sensor_value = drift_value + efficiency_value * (_proportional_weight * proportional_value + _integral_weight * integration_value);
  sensor_value = sensor_value_old + (_input_signal - sensor_value_old) * (1 - exp(-_t/delay_value));
  sensor_value_old = sensor_value;
  cout << "sensor_value_old: " << sensor_value_old << endl;
}

PostprocessorValue
ThermocoupleSensorPostprocessor::getValue() const
{
  return sensor_value;
  // switch (_thermocouple)
  // {
  //   // Switch cases based on thermocouple temperature ranges
  //   case ThermocoupleType::B:
  //   {
  //     if (pp_value < 273.15 || pp_value > 2093.15)
  //       mooseWarning("Temperature is outside this thermocouple type's operation range.");
  //   }
  //   case ThermocoupleType::E:
  //   {
  //     if (pp_value < -543.15 || pp_value > 1273.15)
  //       mooseWarning("Temperature is outside this thermocouple type's operation range.");
  //   }
  //   case ThermocoupleType::J:
  //   {
  //     if (pp_value < -483.15 || pp_value > 1473.15)
  //       mooseWarning("Temperature is outside this thermocouple type's operation range.");
  //   }
  //   case ThermocoupleType::K:
  //   {
  //     if (pp_value < -543.15 || pp_value > 1645.15)
  //       mooseWarning("Temperature is outside this thermocouple type's operation range.");
  //   }
  //   default:
  //     mooseError("Error, invalid thermocouple type.");
  // }
  // If the timestep value is less than or equal to the sensor delay, use the previous value of
  // the postprocessor
  // if (_t <= _delay)
  //   return _pp_old;
  // else
  //  return pp_value;
}

Real ThermocoupleSensorPostprocessor::evaluateEMF(Real t_90, const std::vector<Real>& coefficients) // formula different for K type thermocouple
{
    Real emf = 0.0;
    Real t_power = 1.0;

    for (Real coefficient : coefficients) 
    {
        emf += coefficient * t_power;
        t_power *= t_90;
    }

    return emf;
}

Real ThermocoupleSensorPostprocessor::evaluateEMFTypeK(Real t90, const std::vector<Real>& coefficients, const std::vector<Real>& additionalCoefficients) 
{
    Real result = 0.0;

    // Evaluate the polynomial part
    for (size_t i = 0; i < coefficients.size(); ++i) 
    {
        result += coefficients[i] * std::pow(t90, static_cast<Real>(i));
    }

    // Evaluate the exponential part
    result += additionalCoefficients[0] * std::exp(additionalCoefficients[1] * std::pow(t90 - additionalCoefficients[2], 2.0));

    return result;
}