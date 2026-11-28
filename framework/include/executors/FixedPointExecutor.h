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
#include "SolveObject.h"

#include <vector>
#include <set>

class Convergence;
class SystemBase;

/**
 * Runs a multi-system fixed point iteration. Each iteration sweeps over the inner executors. A
 * Convergence object decides when the iteration stops. Without one, the executor does one sweep.
 */
class FixedPointExecutor : public Executor
{
public:
  static InputParameters validParams();

  FixedPointExecutor(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual Result run() override;

  /// The inner executors in the order of one sweep
  std::vector<Executor *> _sweep;
  /// The Convergence object of the iteration, or nullptr for one sweep
  Convergence * _convergence = nullptr;
  /// The systems whose solution of the previous iteration is stored before each iteration
  std::set<SystemBase *, SystemNumberLess> _systems_to_copy_back;
};
