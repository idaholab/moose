#ifndef FUNCTION_H_
#define FUNCTION_H_

#include "Object.h"
// libMesh
#include "vector_value.h"
#include "point.h"

class Function;

template<>
InputParameters validParams<Function>();

/**
 * Base class for function objects.  Functions override value to supply a
 * value at a point.
 */
class Function : public Object
{
public:
  Function(const std::string & name, InputParameters parameters);
  virtual ~Function();

  /**
   * Override this to evaluate the function at point (t,x,y,z)
   */
  virtual Real value(Real t, const Point & p) = 0;

  /**
   * Function objects can optionally provide a gradient at a point. By default
   * this returns 0, you must override it.
   */
  virtual RealGradient gradient(Real t, const Point & p);
};

#endif //FUNCTION_H_
