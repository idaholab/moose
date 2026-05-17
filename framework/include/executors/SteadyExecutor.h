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

class FEProblemBase;

class SteadyExecutor : public Executor
{
public:
  static InputParameters validParams();
  SteadyExecutor(const InputParameters & params);
  virtual Result run() override;

protected:
  /// The finite element problem providing setup and output routines
  FEProblemBase & _fe_problem;

  /// The inner executors to execute between setup and output
  std::vector<Executor *> _inner_executors;
};
