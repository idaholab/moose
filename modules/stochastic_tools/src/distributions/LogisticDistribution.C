//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LogisticDistribution.h"

registerMooseObjectReplaced("StochasticToolsApp",
                            LogisticDistribution,
                            "07/01/2020 00:00",
                            Logistic);

InputParameters
LogisticDistribution::validParams()
{
  return Logistic::validParams();
}

LogisticDistribution::LogisticDistribution(const InputParameters & parameters)
  : Logistic(parameters)
{
}
