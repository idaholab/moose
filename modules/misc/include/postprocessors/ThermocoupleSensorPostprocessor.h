//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralSensorPostprocessor.h"

/**
 * A Thermocouple Sensor Postprocessor that allows the user to characterize a thermocouple's
 * response by imposing noise, delay, drift, efficiency and uncertainty.
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
  /// Function to calculate R vector
  virtual vector<Real> getRVector() override;
};
