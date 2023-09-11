//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "QuadraticMinimize.h"

/**
 * This form function represents a constrained quadratic objective function:
 *    minimize f(x) = val + \sum_{i=1}^N (x_i - a_i)^2
 * Subject to the equality constraint:
 *    c_e(x) = \sum_{i=1}^N x_i - c_{total} = 0
 *    where val is the input objective value, a_i is the input solution, and
 *    c_total is the equality sum constant.
 */
class QuadraticMinimizeConstrained : public QuadraticMinimize
{
public:
  static InputParameters validParams();
  QuadraticMinimizeConstrained(const InputParameters & parameters);

  virtual Real computeObjective() override;
  virtual void computeGradient(libMesh::PetscVector<Number> & gradient) const override;
  virtual void
  computeEqualityConstraints(libMesh::PetscVector<Number> & eqs_constraints) const override;
  virtual void computeEqualityGradient(libMesh::PetscMatrix<Number> & gradient) const override;

private:
  /// Input objective function value
  const Real & _result;

  /// Desired solution to optimize to
  const std::vector<Real> & _solution;

  const Real _eq_constraint;
};
