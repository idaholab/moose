#pragma once

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

  virtual Real value(Real t, const Point & p) const override;

protected:
  Real _length;

  const Function & _a;
  const Function & _b;

  Real _d;
  Real _h;

  Real _g_0_real;
  Real _g_0_imag;

  Real _g_l_real;
  Real _g_l_imag;

  MooseEnum _component;

  Real _C1_real;
  Real _C1_imag;

  Real _C2_real;
  Real _C2_imag;
};
