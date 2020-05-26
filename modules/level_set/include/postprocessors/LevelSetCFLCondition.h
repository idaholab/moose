//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementPostprocessor.h"

/**
 * Computes the maximum timestep based on the CFL condition.
 */
class LevelSetCFLCondition : public ElementPostprocessor
{
public:
  static InputParameters validParams();

  LevelSetCFLCondition(const InputParameters & parameters);
  void initialize() override {}
  void execute() override;
  void finalize() override;
  void threadJoin(const UserObject & user_object) override;
  virtual PostprocessorValue getValue() override;

private:
  /// The max velocity on an element, this is done simply to avoid creating temporary calls to execute.
  Real _max_velocity;

  /// The minimum timestep computed using CFL condition.
  Real _cfl_timestep;

  /// Velocity vector variable
  const ADVectorVariableValue & _velocity;
};
