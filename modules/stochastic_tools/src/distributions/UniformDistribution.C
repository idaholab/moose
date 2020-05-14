//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UniformDistribution.h"

registerMooseObjectReplaced("StochasticToolsApp", UniformDistribution, "07/01/2020 00:00", Uniform);

InputParameters
UniformDistribution::validParams()
{
  return Uniform::validParams();
}

UniformDistribution::UniformDistribution(const InputParameters & parameters) : Uniform(parameters)
{
}
