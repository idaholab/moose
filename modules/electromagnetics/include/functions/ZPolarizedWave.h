#ifndef ZPOLARIZEDWAVE_H
#define ZPOLARIZEDWAVE_H

#include "Function.h"

class ZPolarizedWave;

template <>
InputParameters validParams<ZPolarizedWave>();

class ZPolarizedWave : public Function
{
public:
  ZPolarizedWave(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;

private:
  Real _theta;

  Real _k;

  Real _sign;

  MooseEnum _component;

  bool _sign_flip;
};

#endif // ZPOLARIZEDWAVE_H
