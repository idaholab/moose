#pragma once

#include "Function.h"

/**
 *  Function for cosine(theta) (where theta is in degrees) for use in slab
 *  reflection benchmark.
 */
class CosTheta : public Function
{
public:
  static InputParameters validParams();

  CosTheta(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) const override;

private:
  Real _theta;
};
