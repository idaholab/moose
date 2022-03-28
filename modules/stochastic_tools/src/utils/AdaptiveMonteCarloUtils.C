//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdaptiveMonteCarloUtils.h"
#include "IndirectSort.h"
#include "libmesh/int_range.h"

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
        std::accumulate(data.begin() + start_index,
                        data.end(),
                        0.0,
                        [&mean](Real x, Real y) { return x + (y - mean) * (y - mean); });
    return std::sqrt(sq_diff / (data.size() - start_index));
  }
}

Real
computeMean(const std::vector<Real> & data, const unsigned int & start_index)
{
  if (data.size() < start_index)
    return 0.0;
  else
    return std::accumulate(data.begin() + start_index, data.end(), 0.0) /
           (data.size() - start_index);
}

std::vector<std::vector<Real>>
sortInput(const std::vector<std::vector<Real>> & inputs,
          const std::vector<Real> & outputs,
          const unsigned int samplessub,
          const Real subset_prob)
{
  std::vector<size_t> ind;
  Moose::indirectSort(outputs.begin(), outputs.end(), ind);

  std::vector<std::vector<Real>> out(inputs.size(), std::vector<Real>(samplessub * subset_prob));
  const size_t offset = std::ceil(samplessub * (1 - subset_prob));
  for (const auto & j : index_range(out))
    for (const auto & i : index_range(out[j]))
      out[j][i] = inputs[j][ind[i + offset]];

  return out;
}

std::vector<Real>
sortOutput(const std::vector<Real> & outputs, const unsigned int samplessub, const Real subset_prob)
{
  std::vector<size_t> ind;
  Moose::indirectSort(outputs.begin(), outputs.end(), ind);

  std::vector<Real> out(samplessub * subset_prob);
  const size_t offset = std::round(samplessub * (1 - subset_prob));
  for (const auto & i : index_range(out))
    out[i] = outputs[ind[i + offset]];

  return out;
}

Real
computeMin(const std::vector<Real> & data)
{
  return *std::min_element(data.begin(), data.end());
}

std::vector<Real>
computeVectorABS(const std::vector<Real> & data)
{
  std::vector<Real> data_abs(data.size());
  for (unsigned int i = 0; i < data.size(); ++i)
    data_abs[i] = std::abs(data[i]);
  return data_abs;
}

} // namespace AdaptiveMonteCarloUtils
