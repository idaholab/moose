//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VaporMixtureFluidProperties.h"

InputParameters
VaporMixtureFluidProperties::validParams()
{
  InputParameters params = FluidProperties::validParams();

  return params;
}

VaporMixtureFluidProperties::VaporMixtureFluidProperties(const InputParameters & parameters)
  : FluidProperties(parameters)
{
}

VaporMixtureFluidProperties::~VaporMixtureFluidProperties() {}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

Real
VaporMixtureFluidProperties::primaryMassFraction(const std::vector<Real> & x) const
{
  return 1 - std::accumulate(x.begin(), x.end(), 0.0);
}

Real
VaporMixtureFluidProperties::mixtureMolarMass(const std::vector<Real> & molar_fractions,
                                              const std::vector<Real> & molar_masses) const
{
  mooseAssert(molar_fractions.size() == molar_masses.size(), "Must have same size");

  Real molar_mass_mix = 0;
  for (const auto i : index_range(molar_fractions))
    molar_mass_mix += molar_fractions[i] * molar_masses[i];
  return molar_mass_mix;
}

std::vector<Real>
VaporMixtureFluidProperties::massFractionsFromMolarFractions(
    const std::vector<Real> & molar_fractions, const std::vector<Real> & molar_masses) const
{
  mooseAssert(molar_fractions.size() == molar_masses.size(), "Must have same size");

  const auto molar_mass_mix = mixtureMolarMass(molar_fractions, molar_masses);

  std::vector<Real> mass_fractions(molar_fractions.size(), 0.0);
  for (const auto i : index_range(molar_fractions))
    mass_fractions[i] = molar_masses[i] / molar_mass_mix * molar_fractions[i];
  return mass_fractions;
}

#pragma GCC diagnostic pop
