#ifndef GAUSSCONTFORCING_H
#define GAUSSCONTFORCING_H

#include "Kernel.h"

class GaussContForcing;

template<>
InputParameters validParams<GaussContForcing>();


class GaussContForcing : public Kernel
{
public:

  GaussContForcing(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  const Real _amplitude;
  const Real _x_center;
  const Real _y_center;
  const Real _z_center;
  
  const Real _x_spread;
  const Real _y_spread;
  const Real _z_spread;
  
  const Real _x_min;
  const Real _x_max;
  const Real _y_min;
  const Real _y_max;
  const Real _z_min;
  const Real _z_max;
};

#endif //GAUSSCONTFORCING_H
