#ifndef LINEARFUNCTION_H
#define LINEARFUNCTION_H

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

  virtual Real value(Real t, const Point & p);
  virtual RealVectorValue gradient(Real t, const Point & p);

protected:
  /// A function representing the x function
  Function & _x_func;
  const Real & _a;
  const Real & _b;
};

#endif // LINEARFUNCTION_H
