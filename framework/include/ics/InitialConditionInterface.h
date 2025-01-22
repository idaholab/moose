//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"

/**
 * InitialConditionInterface serves as the abstract class for InitialConditions,
 * FVInitialConditions, and ScalarInitialConditions
 */
class InitialConditionInterface
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  InitialConditionInterface(const InputParameters & parameters);

  virtual ~InitialConditionInterface();

  static InputParameters validParams();

  /**
   * Retrieves the state of this initial condition.
   * @return The state of this initial condition.
   */
  unsigned short getState() const;

protected:
  // variable used when applying initial conditions to previous states
  unsigned short _my_state;
};
