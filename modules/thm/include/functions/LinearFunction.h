#pragma once

#include "Function.h"
#include "FunctionInterface.h"

class LinearFunction;

template <>
InputParameters validParams<LinearFunction>();

/**
 * The LinearFunction returns:
 *
 * a + b * x_function
 *
 */
class LinearFunction : public Function, public FunctionInterface
{
public:
  LinearFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) const;
  virtual RealVectorValue gradient(Real t, const Point & p) const;

protected:
  /// A function representing the x function
  const Function & _x_func;
  const Real & _a;
  const Real & _b;
};
