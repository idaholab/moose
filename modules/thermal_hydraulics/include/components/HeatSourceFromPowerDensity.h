//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatSourceBase.h"

/**
 * Heat source from power density
 */
class HeatSourceFromPowerDensity : public HeatSourceBase
{
public:
  HeatSourceFromPowerDensity(const InputParameters & parameters);

  virtual void addMooseObjects() override;

protected:
  /// The name of the power density variable (typically an aux variable)
  const VariableName _power_density_name;

public:
  static InputParameters validParams();
};
