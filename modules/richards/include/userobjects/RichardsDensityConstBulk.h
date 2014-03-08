/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  Fluid density assuming constant bulk modulus
//
#ifndef RICHARDSDENSITYCONSTBULK_H
#define RICHARDSDENSITYCONSTBULK_H

#include "RichardsDensity.h"

class RichardsDensityConstBulk;


template<>
InputParameters validParams<RichardsDensityConstBulk>();

class RichardsDensityConstBulk : public RichardsDensity
{
 public:
  RichardsDensityConstBulk(const std::string & name, InputParameters parameters);

  Real density(Real p) const;
  Real ddensity(Real p) const;
  Real d2density(Real p) const;

 protected:
  Real _dens0;
  Real _bulk;
};

#endif // RICHARDSDENSITYCONSTBULK_H
