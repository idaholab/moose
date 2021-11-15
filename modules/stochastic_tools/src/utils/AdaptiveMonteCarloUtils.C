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

std::vector<Real>
sortINPUT(const std::vector<Real> & inputs,
          const std::vector<Real> & outputs,
          const int & samplessub,
          const unsigned int & subset,
          const Real & subset_prob)
{
  std::vector<Real> tmp;
  std::vector<Real> tmp1;
  tmp.resize(samplessub);
  tmp1.resize(samplessub);
  for (unsigned int i = ((subset - 1) * samplessub); i < (subset * samplessub); ++i)
  {
    tmp[i - ((subset - 1) * samplessub)] = outputs[i];
    tmp1[i - ((subset - 1) * samplessub)] = inputs[i];
  }
  std::vector<int> tmp2(tmp.size());
  std::iota(tmp2.begin(), tmp2.end(), 0);
  auto comparator = [&tmp](int a, int b) { return tmp[a] < tmp[b]; };
  std::sort(tmp2.begin(), tmp2.end(), comparator);
  std::vector<Real> out;
  out.resize(std::floor(samplessub * subset_prob));
  for (unsigned int i = 0; i < out.size(); ++i)
  {
    out[i] = tmp1[tmp2[i + std::ceil(samplessub * (1 - subset_prob))]];
  }
  return out;
}

std::vector<Real>
sortOUTPUT(const std::vector<Real> & outputs,
           const int & samplessub,
           const unsigned int & subset,
           const Real & subset_prob)
{
  std::vector<Real> tmp;
  tmp.resize(samplessub);
  for (unsigned int i = ((subset - 1) * samplessub); i < (subset * samplessub); ++i)
  {
    tmp[i - ((subset - 1) * samplessub)] = (outputs[i]);
  }
  std::sort(tmp.begin(), tmp.end());
  std::vector<Real> out;
  out.resize(std::floor(samplessub * subset_prob));
  for (unsigned int i = 0; i < (out.size()); ++i)
  {
    out[i] = tmp[i + std::round(samplessub * (1 - subset_prob))];
  }
  return out;
}

Real
computeMIN(const std::vector<Real> & data)
{
  auto local_min = *std::min_element(data.begin(), data.end());
  return local_min;
}

std::vector<Real>
computeVectorABS(const std::vector<Real> & data)
{
  std::vector<Real> data_abs;
  data_abs.resize(data.size());
  for (unsigned int i = 0; i < data.size(); ++i)
    data_abs[i] = std::abs(data[i]);
  return data_abs;
}

} // namespace AdaptiveMonteCarloUtils
