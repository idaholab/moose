/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  Methane density - a quadratic fit to expressions in:
// "Results of (pressure, density, temperature) measurements on methane and on nitrogen in the temperature range from 273.15K to 323.15K at pressures up to 12MPa using new apparatus for accurate gas-density"
//
#include "RichardsDensityMethane20degC.h"

template<>
InputParameters validParams<RichardsDensityMethane20degC>()
{
  InputParameters params = validParams<RichardsDensity>();
  params.addClassDescription("Methane density (kg/m^3) at 20degC.  Pressure is assumed to be measured in Pascals.");
  return params;
}

RichardsDensityMethane20degC::RichardsDensityMethane20degC(const std::string & name, InputParameters parameters) :
  RichardsDensity(name, parameters)
{}


Real
RichardsDensityMethane20degC::density(Real p) const
{
  return 0.00654576947608E-3*p + 1.04357716547E-13*std::pow(p, 2);
}

Real
RichardsDensityMethane20degC::ddensity(Real p) const
{
  return 0.00654576947608E-3 + 2.08715433094E-13*p;
}

Real
RichardsDensityMethane20degC::d2density(Real p) const
{
  return 2.08715433094E-13;
}

