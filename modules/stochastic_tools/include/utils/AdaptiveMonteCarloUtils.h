//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/utility.h"
#include "DataIO.h"

namespace StochasticTools
{
/* AdaptiveMonteCarloUtils contains functions that are used across the Adaptive Monte
 Carlo set of algorithms.*/

class AdaptiveMonteCarloUtils
{
public:
  AdaptiveMonteCarloUtils(){};

  ~AdaptiveMonteCarloUtils() = default;

  static Real computeSTD(const std::vector<Real> & data, const unsigned int & start_index);
  static Real computeMEAN(const std::vector<Real> & data, const unsigned int & start_index);

  // private:
};
} // namespace
