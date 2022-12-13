//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OptimizationReporter.h"

/**
 * This form function simply represents a quadratic objective function:
 *    f(x) = val + \sum_{i=1}^N (x_i - a_i)^2
 * where val is the input objective value and a_i is the input solution.
 */
class QuadraticMinimize : public OptimizationReporter
{
public:
  static InputParameters validParams();
  QuadraticMinimize(const InputParameters & parameters);

  virtual Real computeObjective() override;
  virtual void computeGradient(libMesh::PetscVector<Number> & gradient) const override;

private:
  /// Input objective function value
  const Real & _result;

  /// Desired solution to optimization
  const std::vector<Real> & _solution;
};
