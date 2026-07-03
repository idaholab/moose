//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Maxwellian.h"
#include "Normal.h"
registerMooseObject("StochasticToolsApp", Maxwellian);

InputParameters
Maxwellian::validParams()
{
  InputParameters params = Distribution::validParams();
  params.addClassDescription("Maxwellian distribution");
  params.addRequiredRangeCheckedParam<Real>(
      "temperature", "temperature > 0", "The temperature of the distribution in K.");
  params.addRequiredRangeCheckedParam<Real>(
      "mass", "mass > 0", "The mass of the species that are the given temperature in kg.");
  return params;
}

Maxwellian::Maxwellian(const InputParameters & parameters)
  : Distribution(parameters),
    _standard_deviation(
        Maxwellian::standardDeviation(getParam<Real>("mass"), getParam<Real>("temperature")))

{
}

Real
Maxwellian::standardDeviation(const Real mass, const Real temperature)
{
  return std::sqrt(k_B * temperature / mass);
}

Real
Maxwellian::pdf(const Real & x, const Real & mass, const Real & temperature)
{
  return Normal::pdf(x, 0, standardDeviation(mass, temperature));
}

Real
Maxwellian::cdf(const Real & x, const Real & mass, const Real & temperature)
{
  return Normal::cdf(x, 0, standardDeviation(mass, temperature));
}

Real
Maxwellian::quantile(const Real & p, const Real & mass, const Real & temperature)
{
  return Normal::quantile(p, 0, standardDeviation(mass, temperature));
}

Real
Maxwellian::pdf(const Real & x) const
{
  return Normal::pdf(x, 0, _standard_deviation);
}

Real
Maxwellian::cdf(const Real & x) const
{
  return Normal::cdf(x, 0, _standard_deviation);
}

Real
Maxwellian::quantile(const Real & p) const
{
  return Normal::quantile(p, 0, _standard_deviation);
}
