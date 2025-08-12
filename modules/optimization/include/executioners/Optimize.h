//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Steady.h"
#include "OptimizeSolve.h"

// System includes
#include <string>

// Forward declarations
class InputParameters;
class Optimize;
class FEProblemBase;

class Optimize : public Steady
{
public:
  static InputParameters validParams();

  Optimize(const InputParameters & parameters);

  virtual void execute() override;

  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

  OptimizeSolve & getOptimizeSolve() { return _optim_solve; }

protected:
  OptimizeSolve _optim_solve;

private:
  bool _last_solve_converged = true;
};
