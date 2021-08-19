//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
// Bootstrap CI

#include "BootstrapCalculators.h"

namespace StochasticTools
{

MooseEnum
makeBootstrapCalculatorEnum()
{
  return MooseEnum("percentile=0 bca=1");
}

createBootstrapCalculators(std::vector<Real>, Real);
createBootstrapCalculators(std::vector<int>, Real);

} // namespace
