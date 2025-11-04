//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TruncatedNormalDistribution.h"

registerMooseObjectReplaced("StochasticToolsApp",
                            TruncatedNormalDistribution,
                            "07/01/2020 00:00",
                            TruncatedNormal);

InputParameters
TruncatedNormalDistribution::validParams()
{
  return TruncatedNormal::validParams();
}

TruncatedNormalDistribution::TruncatedNormalDistribution(const InputParameters & parameters)
  : TruncatedNormal(parameters)
{
}
