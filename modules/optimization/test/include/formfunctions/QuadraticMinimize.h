#pragma once

#include "FormFunction.h"

/**
 * This form function simply represents a quadratic objective function:
 *    f(x) = val + \sum_{i=1}^N (x_i - a_i)^2
 * where val is the inputted objective value and a_i is the inputted solution.
 */
class QuadraticMinimize : public FormFunction
{
public:
  static InputParameters validParams();
  QuadraticMinimize(const InputParameters & parameters);

  virtual Real computeObjective() override;
  virtual void computeGradient() override;
  virtual void computeHessian() override;

protected:
  /// Inputted objective function value
  const Real & _result;
};
