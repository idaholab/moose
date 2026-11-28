//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Executor.h"

#include <vector>

/**
 * Runs a steady-state simulation: sets up the problem, runs the inner executors in order, and
 * performs the output and final execution
 */
class SteadyExecutor : public Executor
{
public:
  static InputParameters validParams();

  SteadyExecutor(const InputParameters & parameters);

  virtual void init() override;

protected:
  virtual Result run() override;

  /// The inner executors, run in the order of the inner_executors parameter
  std::vector<Executor *> _inner_executors;
};
