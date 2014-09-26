#ifndef TENSORMECHANICSPLASTICJ2GAUSSIAN_H
#define TENSORMECHANICSPLASTICJ2GAUSSIAN_H

#include "TensorMechanicsPlasticJ2.h"


class TensorMechanicsPlasticJ2Gaussian;


template<>
InputParameters validParams<TensorMechanicsPlasticJ2Gaussian>();

/**
 * J2 plasticity, associative, with no hardning.
 * Yield_function = sqrt(3*J2) - yieldStrength
 * In this class yieldStrength = yinf + (y0 - yinf)*exp(-0.5*(intnl*rate)^2)
 */
class TensorMechanicsPlasticJ2Gaussian : public TensorMechanicsPlasticJ2
{
 public:
  TensorMechanicsPlasticJ2Gaussian(const std::string & name, InputParameters parameters);

 protected:

  /// y0
  Real _y0;

  /// yinf
  Real _yinf;

  /// rate
  Real _rate;

  /**
   * YieldStrength.  The yield function is sqrt(3*J2) - yieldStrength.
   * In this class yieldStrength = yinf + (y0 - yinf)*exp(-0.5*(intnl*rate)^2)
   */
  virtual Real yieldStrength(const Real & intnl) const;

  /// d(yieldStrength)/d(intnl)
  virtual Real dyieldStrength(const Real & intnl) const;

 

};

#endif // TENSORMECHANICSPLASTICJ2GAUSSIAN_H
