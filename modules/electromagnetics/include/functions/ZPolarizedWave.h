#pragma once

#include "Function.h"

class ZPolarizedWave : public Function
{
public:
  static InputParameters validParams();

  ZPolarizedWave(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) const override;

private:
  Real _theta;

  Real _k;

  Real _sign;

  MooseEnum _component;

  bool _sign_flip;
};
