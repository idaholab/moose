//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NormalDistribution.h"

registerMooseObjectReplaced("StochasticToolsApp", NormalDistribution, "07/01/2020 00:00", Normal);

InputParameters
NormalDistribution::validParams()
{
  return Normal::validParams();
}

NormalDistribution::NormalDistribution(const InputParameters & parameters) : Normal(parameters) {}
