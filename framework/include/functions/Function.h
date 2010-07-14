#ifndef FUNCTION_H
#define FUNCTION_H

#include "MooseObject.h"

class Function;

template<>
InputParameters validParams<Function>();

/**
 * Base class for function objects.  Functions override operator() to supply a
 * value at a point.
 */
class Function : public MooseObject
{
public:
  Function(std::string name, MooseSystem & moose_system, InputParameters parameters);
  virtual ~Function();

  /**
   * Evaluate this function at point (t,x,y,z)
   */
  virtual Real operator()(Real t, Real x, Real y = 0, Real z = 0) = 0;
};

#endif //FUNCTION_H
