/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  Methane density - a quadratic fit to expressions in:
// "Results of (pressure, density, temperature) measurements on methane and on nitrogen in the temperature range from 273.15K to 323.15K at pressures up to 12MPa using new apparatus for accurate gas-density"
//
#ifndef RICHARDSDENSITYMETHANE20DEGC_H
#define RICHARDSDENSITYMETHANE20DEGC_H

#include "RichardsDensity.h"

class RichardsDensityMethane20degC;


template<>
InputParameters validParams<RichardsDensityMethane20degC>();

class RichardsDensityMethane20degC : public RichardsDensity
{
 public:
  RichardsDensityMethane20degC(const std::string & name, InputParameters parameters);

  Real density(Real p) const;
  Real ddensity(Real p) const;
  Real d2density(Real p) const;
};

#endif // RICHARDSDENSITYMETHANE20DEGC_H
