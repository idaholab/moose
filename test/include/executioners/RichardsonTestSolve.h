//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NonlinearSolveObject.h"

/**
 * Solves the first nonlinear system R(u) = 0 with the Richardson iteration
 * u <- u - step_size * R(u). The iteration converges for a linear problem with a symmetric
 * positive definite Jacobian if step_size is less than 2 / (largest eigenvalue of the Jacobian).
 */
class RichardsonTestSolve : public NonlinearSolveObject
{
public:
  static InputParameters validParams();

  RichardsonTestSolve(Executioner & ex);

  virtual bool solve() override;

protected:
  const Real _step_size;
  const unsigned int _max_its;
  const Real _abs_tol;
  const Real _rel_tol;
};
