#ifndef HELMHOLTZTESTFUNC_H
#define HELMHOLTZTESTFUNC_H

#include "Function.h"
#include "FunctionInterface.h"

class HelmholtzTestFunc;

template <>
InputParameters validParams<HelmholtzTestFunc>();

/**
 *  Function of analytical solution for use in convergence testing with coupled_helmholtz test file
 */
class HelmholtzTestFunc : public Function, public FunctionInterface
{
public:
  HelmholtzTestFunc(const InputParameters & parameters);

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

  MooseEnum _component;

  Real _C1_real;
  Real _C1_imag;

  Real _C2_real;
  Real _C2_imag;
};

#endif // HELMHOLTZTESTFUNC_H
