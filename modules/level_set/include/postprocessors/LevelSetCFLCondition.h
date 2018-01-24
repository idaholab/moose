//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LEVELSETCFLCONDITION_H
#define LEVELSETCFLCONDITION_H

// MOOSE includes
#include "ElementPostprocessor.h"
#include "LevelSetVelocityInterface.h"

// Forward declarations
class LevelSetCFLCondition;

template <>
InputParameters validParams<LevelSetCFLCondition>();

/**
 * Computes the maximum timestep based on the CFL condition.
 */
class LevelSetCFLCondition : public LevelSetVelocityInterface<ElementPostprocessor>
{
public:
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
};

#endif // LEVELSETCFLCONDITION_H
