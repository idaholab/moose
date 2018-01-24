//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NSINITIALCONDITION_H
#define NSINITIALCONDITION_H

#include "InitialCondition.h"
#include "InputParameters.h"

// Forward Declarations
class NSInitialCondition;
class IdealGasFluidProperties;

template <>
InputParameters validParams<NSInitialCondition>();

/**
 * NSInitialCondition sets intial constant values for all variables
 * given the:
 * .) Initial pressure
 * .) Initial temperature
 * .) Initial velocity
 * and a FluidProperties UserObject.
 */
class NSInitialCondition : public InitialCondition
{
public:
  NSInitialCondition(const InputParameters & parameters);

  /**
   * The value of the variable at a point.
   */
  virtual Real value(const Point & p);

protected:
  Real _initial_pressure;
  Real _initial_temperature;
  RealVectorValue _initial_velocity;

  // Fluid properties
  const IdealGasFluidProperties & _fp;
};

#endif
