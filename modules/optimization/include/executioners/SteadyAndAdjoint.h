//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Steady.h"
#include "AdjointSolve.h"

// Forward declarations
class InputParameters;

class SteadyAndAdjoint : public Steady
{
public:
  static InputParameters validParams();

  SteadyAndAdjoint(const InputParameters & parameters);

  virtual void execute() override;

  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

protected:
  AdjointSolve _adjoint_solve;

private:
  bool _last_solve_converged = true;
};
