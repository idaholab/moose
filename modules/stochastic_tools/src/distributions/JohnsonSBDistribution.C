//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JohnsonSBDistribution.h"

registerMooseObjectReplaced("StochasticToolsApp",
                            JohnsonSBDistribution,
                            "07/01/2020 00:00",
                            JohnsonSB);

InputParameters
JohnsonSBDistribution::validParams()
{
  return JohnsonSB::validParams();
}

JohnsonSBDistribution::JohnsonSBDistribution(const InputParameters & parameters)
  : JohnsonSB(parameters)
{
}
