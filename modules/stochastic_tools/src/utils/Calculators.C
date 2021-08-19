//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Calculators.h"

namespace StochasticTools
{

MultiMooseEnum
makeCalculatorEnum()
{
  return MultiMooseEnum("min=0 max=1 sum=2 mean=3 stddev=4 norm2=5 ratio=6 stderr=7 median=8");
}

createCalculators(std::vector<Real>, Real);
createCalculators(std::vector<int>, Real);

} // StocasticTools namespace
