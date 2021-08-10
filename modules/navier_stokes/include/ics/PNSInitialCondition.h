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

// Forward Declarations
class IdealGasFluidProperties;

/**
 * PNSInitialCondition sets intial constant values for all variables
 * given the:
 * .) Initial pressure
 * .) Initial temperature
 * .) Initial velocity
 * .) the porosity variable (or material property)
 * and a FluidProperties UserObject.
 */
class PNSInitialCondition : public InitialCondition
{
public:
  static InputParameters validParams();

  PNSInitialCondition(const InputParameters & parameters);

  /**
   * The value of the variable at a point.
   */
  virtual Real value(const Point & p);

protected:
  /// Used to map the variable to one of the expected types
  const std::string _variable_type;

  /// Initial constant value of the pressure
  const Real _initial_pressure;

  /// Initial constant value of the fluid temperature
  const Real _initial_temperature;

  /// Whether initial velocities were specified as superficial or interstitial
  const bool _superficial_velocities_set;

  /// Initial constant value of the superficial velocity
  RealVectorValue _initial_superficial_velocity;

  /// Initial constant value of the interstitial velocity
  RealVectorValue _initial_interstitial_velocity;

  /// The porosity variable
  const VariableValue & _eps;

  /// Fluid properties
  const IdealGasFluidProperties & _fp;
};
