//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Maxwellian.h"
#include "NormalBase.h"
registerMooseObject("StochasticToolsApp", Maxwellian);

/// boltzman constant with units of J/K
constexpr Real k_B = 1.380649e-23;

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
    NormalBase(0.0,
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
  return NormalBase::pdf(x, 0, standardDeviation(mass, temperature));
}

Real
Maxwellian::cdf(const Real & x, const Real & mass, const Real & temperature)
{
  return NormalBase::cdf(x, 0, standardDeviation(mass, temperature));
}

Real
Maxwellian::quantile(const Real & p, const Real & mass, const Real & temperature)
{
  return NormalBase::quantile(p, 0, standardDeviation(mass, temperature));
}

Real
Maxwellian::pdf(const Real & x) const
{
  return NormalBase::pdf(x);
}

Real
Maxwellian::cdf(const Real & x) const
{
  return NormalBase::cdf(x);
}

Real
Maxwellian::quantile(const Real & p) const
{
  return NormalBase::quantile(p);
}
