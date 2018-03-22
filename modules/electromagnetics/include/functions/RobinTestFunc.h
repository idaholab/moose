#ifndef ROBINTESTFUNC_H
#define ROBINTESTFUNC_H

#include "Function.h"
#include "FunctionInterface.h"

class RobinTestFunc;

template <>
InputParameters validParams<RobinTestFunc>();

/**
 *  Function of analytical solution for use in convergence testing with coupled_helmholtz test file
 */
class RobinTestFunc : public Function, public FunctionInterface
{
public:
  RobinTestFunc(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;

protected:
  Real _L;

  Real _a;
  Real _b;

  Real _d;
  Real _h;

  Real _g0_real;
  Real _g0_imag;

  Real _gL_real;
  Real _gL_imag;

  Function & _func;

  MooseEnum _component;
};

#endif // ROBINTESTFUNC_H
