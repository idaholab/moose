#pragma once

#include "Function.h"

/**
 *    Function for field coefficient in slab reflection benchmark case.
 */
class JinSlabCoeffFunc : public Function
{
public:
  static InputParameters validParams();

  JinSlabCoeffFunc(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

protected:
  Real _k;

  Real _theta;

  Real _length;

  MooseEnum _component;
};
