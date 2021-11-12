//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"
#include "InputParameters.h"
#include "Function.h"

// Forward Declarations
class IdealGasFluidProperties;

/**
 * NSFunctionInitialCondition sets intial constant values for all variables
 * given the:
 * .) Initial pressure
 * .) Initial temperature
 * .) Initial velocity
 * and a FluidProperties UserObject.
 */
class NSFunctionInitialCondition : public InitialCondition
{
public:
  static InputParameters validParams();

  NSFunctionInitialCondition(const InputParameters & parameters);

  /**
   * The value of the variable at a point.
   */
  virtual Real value(const Point & p);

protected:
  /// Used to map the variable to one of the expected types
  const std::string _variable_type;

  /// Initial constant value of the pressure
  const Function & _initial_pressure;

  /// Initial constant value of the fluid temperature
  const Function & _initial_temperature;

  /// Initial constant value of the velocity
  std::vector<const Function *> _initial_velocity;

  /// Fluid properties
  const IdealGasFluidProperties & _fp;

  /// pressure variable name
  const std::string _pressure_variable_name;
};
