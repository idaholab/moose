#pragma once

#include "OptimizationReporter.h"

/**
 * This form function simply represents a quadratic objective function:
 *    f(x) = val + \sum_{i=1}^N (x_i - a_i)^2
 * where val is the inputted objective value and a_i is the inputted solution.
 */
class QuadraticMinimize : public OptimizationReporter
{
public:
  static InputParameters validParams();
  QuadraticMinimize(const InputParameters & parameters);

  virtual Real computeAndCheckObjective(bool /*multiapp_passed*/) override;
  virtual void computeGradient(libMesh::PetscVector<Number> & gradient) override;

private:
  /// Inputted objective function value
  const Real & _result;

  /// Desired solution to optimization
  const std::vector<Real> & _solution;
};
