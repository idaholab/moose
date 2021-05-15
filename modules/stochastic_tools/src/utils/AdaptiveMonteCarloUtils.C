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

namespace StochasticTools
{
// Compute standard deviation of a data vector by ignoring some values in the vector at the
// beginning
Real
AdaptiveMonteCarloUtils::computeSTD(const std::vector<Real> & data,
                                    const unsigned int & start_index)
{
  Real sum1 = 0.0, sq_diff1 = 0.0;
  for (unsigned int i = start_index; i < data.size(); ++i)
  {
    sum1 += data[i];
  }
  for (unsigned int i = start_index; i < data.size(); ++i)
  {
    sq_diff1 += std::pow((data[i] - sum1 / (data.size() - start_index)), 2);
  }
  return std::pow(sq_diff1 / (data.size() - start_index), 0.5);
}

// Compute mean of a data vector by ignoring some values in the vector at the beginning
Real
AdaptiveMonteCarloUtils::computeMEAN(const std::vector<Real> & data,
                                     const unsigned int & start_index)
{
  Real sum1 = 0.0;
  for (unsigned int i = start_index; i < data.size(); ++i)
  {
    sum1 += (data[i]);
  }
  return (sum1 / (data.size() - start_index));
}
} // StochasticTools namespace
