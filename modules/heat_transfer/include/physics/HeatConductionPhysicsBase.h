//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsBase.h"

/**
 * Base class to host common parameters and attributes to all Physics solving the heat conduction
 * equation
 */
class HeatConductionPhysicsBase : public PhysicsBase
{
public:
  static InputParameters validParams();

  HeatConductionPhysicsBase(const InputParameters & parameters);

protected:
  /// Name of the temperature variable
  const VariableName & _temperature_name;

private:
  virtual void addPreconditioning() override;
  virtual void addInitialConditions() override;
};
