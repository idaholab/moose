//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/**
 * A thermocouple Postprocessor
  */
class ThermocoupleSensorPostprocessor : public GeneralSensorPostprocessor
{
public:
  static InputParameters validParams();

  ThermocoupleSensorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  using Postprocessor::getValue;
  virtual PostprocessorValue getValue() const override; 

protected:
  string _thermocouple_type;
  string _method;
  Real _sensor_value_old;
  virtual Real getIntegral(std::vector<Real> integrand, std::vector<Real> time_values, Real delay_value) override;
};