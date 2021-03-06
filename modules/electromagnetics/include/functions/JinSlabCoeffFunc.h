#pragma once

#include "Function.h"
#include "FunctionInterface.h"

/**
 *    Function for coefficient in JinSlab1D case.
 */
class JinSlabCoeffFunc : public Function, public FunctionInterface
{
public:
  static InputParameters validParams();

  JinSlabCoeffFunc(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) const override;

protected:

  Real _k;

  Real _theta;

  MooseEnum _component;
};
