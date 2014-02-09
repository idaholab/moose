/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  Fluid density ideal gas
//
#ifndef RICHARDSDENSITYIDEAL_H
#define RICHARDSDENSITYIDEAL_H

#include "RichardsDensity.h"

class RichardsDensityIdeal;


template<>
InputParameters validParams<RichardsDensityIdeal>();

class RichardsDensityIdeal : public RichardsDensity
{
 public:
  RichardsDensityIdeal(const std::string & name, InputParameters parameters);

  Real density(Real p) const;
  Real ddensity(Real /*p*/) const;
  Real d2density(Real /*p*/) const;

 protected:
  Real _slope;
  Real _p0;
};

#endif // RICHARDSDENSITYIDEAL_H
