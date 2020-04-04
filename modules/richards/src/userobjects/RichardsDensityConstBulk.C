//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  Fluid density assuming constant bulk modulus
//
#include "RichardsDensityConstBulk.h"

registerMooseObject("RichardsApp", RichardsDensityConstBulk);

InputParameters
RichardsDensityConstBulk::validParams()
{
  InputParameters params = RichardsDensity::validParams();
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
