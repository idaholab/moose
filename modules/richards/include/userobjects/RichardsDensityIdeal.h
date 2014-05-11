/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSDENSITYIDEAL_H
#define RICHARDSDENSITYIDEAL_H

#include "RichardsDensity.h"

class RichardsDensityIdeal;


template<>
InputParameters validParams<RichardsDensityIdeal>();

/**
 * Fluid density of an ideal gas
 */
class RichardsDensityIdeal : public RichardsDensity
{
 public:
  RichardsDensityIdeal(const std::string & name, InputParameters parameters);

  /**
   * fluid density as a function of porepressure
   * @param p porepressure
   */
  Real density(Real p) const;

  /**
   * derivative of fluid density wrt porepressure
   */
  Real ddensity(Real /*p*/) const;

  /**
   * second derivative of fluid density wrt porepressure
   */
  Real d2density(Real /*p*/) const;

 protected:

  /// density = _slope*(p - _p0)
  Real _slope;

  /// density = _slope*(p - _p0)
  Real _p0;
};

#endif // RICHARDSDENSITYIDEAL_H
