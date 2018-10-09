//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VaporMixtureFluidProperties.h"

template <>
InputParameters
validParams<VaporMixtureFluidProperties>()
{
  InputParameters params = validParams<FluidProperties>();

  return params;
}

VaporMixtureFluidProperties::VaporMixtureFluidProperties(const InputParameters & parameters)
  : FluidProperties(parameters)
{
}

VaporMixtureFluidProperties::~VaporMixtureFluidProperties() {}

Real
VaporMixtureFluidProperties::primaryMassFraction(const std::vector<Real> & x) const
{
  return 1 - std::accumulate(x.begin(), x.end(), 0.0);
}
