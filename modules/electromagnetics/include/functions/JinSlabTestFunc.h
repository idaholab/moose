#ifndef JINSLABTESTFUNC_H
#define JINSLABTESTFUNC_H

#include "Function.h"
#include "FunctionInterface.h"

class JinSlabTestFunc;

template <>
InputParameters validParams<JinSlabTestFunc>();

/**
 *  Function of analytical solution for use in convergence testing with coupled_helmholtz test file
 */
class JinSlabTestFunc : public Function, public FunctionInterface
{
public:
  JinSlabTestFunc(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;

protected:
  Real _L;

  Real _theta;

  Function & _epsR_real;

  Function & _epsR_imag;

  Real _muR_real;

  Real _muR_imag;

  Real _k;

  MooseEnum _component;
};

#endif // JINSLABTESTFUNC_H
