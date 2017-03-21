/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  Fluid density ideal gas
//
#include "RichardsDensityIdeal.h"

template <>
InputParameters
validParams<RichardsDensityIdeal>()
{
  InputParameters params = validParams<RichardsDensity>();
  params.addRequiredParam<Real>("slope", "Density = slope*(p - p0)");
  params.addRequiredParam<Real>("p0", "Density = slope*(p - p0)");
  params.addClassDescription("Fluid density of ideal gas.  Density = slope*(p - p0)");
  return params;
}

RichardsDensityIdeal::RichardsDensityIdeal(const InputParameters & parameters)
  : RichardsDensity(parameters), _slope(getParam<Real>("slope")), _p0(getParam<Real>("p0"))
{
}

Real
RichardsDensityIdeal::density(Real p) const
{
  return _slope * (p - _p0);
}

Real RichardsDensityIdeal::ddensity(Real /*p*/) const { return _slope; }

Real RichardsDensityIdeal::d2density(Real /*p*/) const { return 0.0; }
