//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TransientBase.h"

class FEProbleSolve;

/**
 * Transient executioners usually loop through a number of timesteps... calling solve()
 * for each timestep.
 */
class Transient : public TransientBase
{
public:
  static InputParameters validParams();

  Transient(const InputParameters & parameters);

  /**
   * The relative L2 norm of the difference between solution and old solution vector.
   */
  virtual Real relativeSolutionDifferenceNorm() override;

protected:
  virtual std::set<TimeIntegrator *> getTimeIntegrators() const override;

  /// inner-most solve object to perform Newton solve with PETSc on every time step
  FEProblemSolve _feproblem_solve;
};
