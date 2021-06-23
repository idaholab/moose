//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdaptiveMonteCarloUtils.h"

/* AdaptiveMonteCarloUtils contains functions that are used across the Adaptive Monte
 Carlo set of algorithms.*/

namespace AdaptiveMonteCarloUtils
{

Real
computeSTD(const std::vector<Real> & data, const unsigned int & start_index)
{
  if (data.size() < start_index)
    return 0.0;
  else
  {
    const Real mean = computeMean(data, start_index);
    const Real sq_diff =
        std::accumulate(data.begin() + start_index, data.end(), 0.0, [&mean](Real x, Real y) {
          return x + (y - mean) * (y - mean);
        });
    return std::sqrt(sq_diff / (data.size() - start_index));
  }
}

// Compute mean of a data vector by ignoring some values in the vector at the beginning
Real
computeMean(const std::vector<Real> & data, const unsigned int & start_index)
{
  if (data.size() < start_index)
    return 0.0;
  else
    return std::accumulate(data.begin() + start_index, data.end(), 0.0) /
           (data.size() - start_index);
}

} // namespace AdaptiveMonteCarloUtils
