//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Action.h"

/**
 * An action for creating the necessary objects to perform a thermal mechanics problem using
 * Darcy's equation.
 * */
class SetupDarcySimulation : public Action
{
public:
  static InputParameters validParams();

  SetupDarcySimulation(const InputParameters & params);
  virtual void act() override;

protected:
  const bool _compute_velocity;
  const bool _compute_pressure;
  const bool _compute_temperature;
};
