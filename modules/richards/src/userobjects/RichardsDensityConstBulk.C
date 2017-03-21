/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  Fluid density assuming constant bulk modulus
//
#include "RichardsDensityConstBulk.h"

template <>
InputParameters
validParams<RichardsDensityConstBulk>()
{
  InputParameters params = validParams<RichardsDensity>();
  params.addRequiredParam<Real>("dens0", "Reference density of fluid.  Eg 1000");
  params.addRequiredParam<Real>("bulk_mod", "Bulk modulus of fluid.  Eg 2E9");
  params.addClassDescription(
      "Fluid density assuming constant bulk modulus.  dens0 * Exp(pressure/bulk)");
  return params;
}

RichardsDensityConstBulk::RichardsDensityConstBulk(const InputParameters & parameters)
  : RichardsDensity(parameters), _dens0(getParam<Real>("dens0")), _bulk(getParam<Real>("bulk_mod"))
{
}

Real
RichardsDensityConstBulk::density(Real p) const
{
  return _dens0 * std::exp(p / _bulk);
}

Real
RichardsDensityConstBulk::ddensity(Real p) const
{
  return density(p) / _bulk;
}

Real
RichardsDensityConstBulk::d2density(Real p) const
{
  return density(p) / _bulk / _bulk;
}
