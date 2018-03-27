#ifndef COSTHETA_H
#define COSTHETA_H

#include "Function.h"

class CosTheta;

template <>
InputParameters validParams<CosTheta>();

/**
 *  Function for cosine(theta) (where theta is in degrees) for use in reflection problems.
 */
class CosTheta : public Function
{
public:
  CosTheta(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;

private:
  Real _theta;
};

#endif // COSTHETA_H
