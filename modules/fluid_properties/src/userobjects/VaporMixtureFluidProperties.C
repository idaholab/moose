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

#pragma GCC diagnostic pop
