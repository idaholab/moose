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

  /**
   * This call is basically a copy from Steady without the AMR loop and with a call to the
   * adjoint solver after the fixed point solver
   */
  virtual void execute() override;

  /**
   * Copy of the functionality from Steady to keep track of whether the latest solve converged
   */
  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

protected:
  /// The solver which computes the adjoint system. This is where the real magic happens,
  /// so it is recommended to look into this object to understand the algorithm.
  AdjointSolve _adjoint_solve;

private:
  bool _last_solve_converged = true;
};
