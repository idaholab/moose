//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AddGeochemistrySolverAction.h"

/**
 * Action that sets up a spatial reaction solver
 */
class AddSpatialReactionSolverAction : public AddGeochemistrySolverAction
{
public:
  static InputParameters validParams();

  AddSpatialReactionSolverAction(const InputParameters & parameters);

  virtual void act() override;
};
