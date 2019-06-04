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

  virtual Real value(Real t, const Point & p) const override;

protected:
  Real _length;

  Real _a;
  Real _b;

  Real _d;
  Real _h;

  Real _g_0_real;
  Real _g_0_imag;

  const Function & _func;

  MooseEnum _component;
};

#endif // ROBINTESTFUNC_H
