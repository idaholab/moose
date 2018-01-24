/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  Methane density - a quadratic fit to expressions in:
// "Results of (pressure, density, temperature) measurements on methane and on nitrogen in the
// temperature range from 273.15K to 323.15K at pressures up to 12MPa using new apparatus for
// accurate gas-density"
//
// This is only valid for p>=0, which is the physical region.  I extend to the p>0 domain with an
// exponential, which will probably be sampled as the newton interative process converges towards
// the solution.
#include "RichardsDensityMethane20degC.h"

template <>
InputParameters
validParams<RichardsDensityMethane20degC>()
{
  InputParameters params = validParams<RichardsDensity>();
  params.addParam<Real>(
      "p_unit",
      1,
      "Set to 1 for pressure measured in Pascals.  Set to 1E6 for pressure measured in MPa.  Etc.");
  params.addClassDescription("Methane density (kg/m^3) at 20degC.  Pressure is assumed to be "
                             "measured in Pascals.  NOTE: this expression is only valid to about "
                             "P=20MPa.  Use van der Waals (RichardsDensityVDW) for higher "
                             "pressures.");
  return params;
}

RichardsDensityMethane20degC::RichardsDensityMethane20degC(const InputParameters & parameters)
  : RichardsDensity(parameters), _p_unit(getParam<Real>("p_unit"))
{
}

Real
RichardsDensityMethane20degC::density(Real p) const
{
  if (p >= 0)
    return 0.00654576947608E-3 * p * _p_unit + 1.04357716547E-13 * std::pow(p * _p_unit, 2);
  else
    return 0.1 * (std::exp(6.54576947608E-5 * p * _p_unit) - 1);
}

Real
RichardsDensityMethane20degC::ddensity(Real p) const
{
  if (p >= 0)
    return (0.00654576947608E-3 + 2.08715433094E-13 * p * _p_unit) * _p_unit;
  else
    return 0.1 * 6.54576947608E-5 * _p_unit * std::exp(6.54576947608E-5 * p * _p_unit);
}

Real
RichardsDensityMethane20degC::d2density(Real p) const
{
  if (p >= 0)
    return 2.08715433094E-13 * std::pow(_p_unit, 2);
  else
    return 0.1 * std::pow(6.54576947608E-5 * _p_unit, 2) * std::exp(6.54576947608E-5 * p * _p_unit);
}
