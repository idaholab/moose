//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectAction.h"

/**
 * Helper for performing common tasks for stochastic simulations
 *
 * 1. Setup Mesh, Problem, Executioner for runner only problems
 */
class StochasticToolsAction : public Action
{
public:
  static InputParameters validParams();
  StochasticToolsAction(const InputParameters & params);
  virtual void act() override;
};
