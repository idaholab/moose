#pragma once

#include "Function.h"
#include "FunctionInterface.h"

/**
 *  Function of RHS for manufactured solution in spatial_constant_helmholtz test
 */
class MMSTestFunc : public Function, public FunctionInterface
{
public:
  static InputParameters validParams();
  MMSTestFunc(const InputParameters & parameters);

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

};
