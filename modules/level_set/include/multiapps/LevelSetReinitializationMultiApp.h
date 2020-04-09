//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiApp.h"

// Forward declarations
class LevelSetReinitializationProblem;
/**
 * MultiApp that performs a time reset prior to solving, this enables the level set reinitialization
 * to
 * solve repeatedly.
 */
class LevelSetReinitializationMultiApp : public MultiApp
{
public:
  static InputParameters validParams();

  LevelSetReinitializationMultiApp(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual bool solveStep(Real dt, Real target_time, bool auto_advance = true) override;

protected:
  /// Access to the level set specific problem to allow for the resetTime() method to be called.
  LevelSetReinitializationProblem * _level_set_problem;

  /// Access to the Executioner object to call execute()
  Executioner * _executioner;

  /// The solve interval for reinitialization.
  const unsigned int & _interval;
};
