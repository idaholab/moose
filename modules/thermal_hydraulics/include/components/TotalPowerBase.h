//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component.h"

/**
 * Base class for components that provide total power
 */
class TotalPowerBase : public Component
{
public:
  TotalPowerBase(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual const VariableName & getPowerVariableName() const { return _power_var_name; }

protected:
  /// The scalar variable holding the value of power
  const VariableName _power_var_name;

public:
  static InputParameters validParams();
};
