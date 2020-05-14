//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoostLognormalDistribution.h"

registerMooseObjectReplaced("StochasticToolsApp",
                            BoostLognormalDistribution,
                            "07/01/2020 00:00",
                            Lognormal);

InputParameters
BoostLognormalDistribution::validParams()
{
  return BoostLognormal::validParams();
}

BoostLognormalDistribution::BoostLognormalDistribution(const InputParameters & parameters)
  : BoostLognormal(parameters)
{
}
