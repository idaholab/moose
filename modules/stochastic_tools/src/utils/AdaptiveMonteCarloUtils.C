//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseUtils.h"
#include "AdaptiveMonteCarloUtils.h"

std::vector<Real>
AdaptiveMonteCarloUtils::sortINPUT(const std::vector<Real> & inputs, const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob)
{
  std::vector<Real> tmp;
  std::vector<Real> tmp1;
  tmp.resize(samplessub);
  tmp1.resize(samplessub);
  for (unsigned int i = ((subset-1) * samplessub); i < (subset * samplessub); ++i)
  {
    tmp[i - ((subset-1) * samplessub)] = outputs[i];
    tmp1[i - ((subset-1) * samplessub)] = inputs[i];
  }
  std::vector<int> tmp2(tmp.size());
  std::iota(tmp2.begin(), tmp2.end(), 0);
  auto comparator = [&tmp](int a, int b){ return tmp[a] < tmp[b]; };
  std::sort(tmp2.begin(), tmp2.end(), comparator);
  std::vector<Real> out;
  out.resize(std::floor(samplessub * subset_prob));
  for (unsigned int i = 0; i < out.size(); ++i)
  {
    out[i] = tmp1[tmp2[i + std::ceil(samplessub * (1-subset_prob))]];
  }
  return out;
}

std::vector<Real>
AdaptiveMonteCarloUtils::sortOUTPUT(const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob)
{
  std::vector<Real> tmp;
  tmp.resize(samplessub);
  for (unsigned int i = ((subset-1) * samplessub); i < (subset * samplessub); ++i)
  {
    tmp[i - ((subset-1) * samplessub)] = (outputs[i]);
  }
  std::sort(tmp.begin(), tmp.end());
  std::vector<Real> out;
  out.resize(std::floor(samplessub * subset_prob));
  for (unsigned int i = 0; i < (out.size()); ++i)
  {
    out[i] = tmp[i + std::round(samplessub * (1-subset_prob))];
  }
  return out;
}

Real
AdaptiveMonteCarloUtils::computeSTD(const std::vector<Real> & data)
{
  Real sum1 = 0.0, sq_diff1 = 0.0;
  for (unsigned int i = 2; i < data.size(); ++i)
  {
    sum1 += (data[i]);
  }
  //auto sum1 = std::accumulate(data.begin(), data.end(), 0);
  // Real sq_diff1 = 0.0;
  for (unsigned int i = 2; i < data.size(); ++i)
  {
    sq_diff1 += std::pow(((data[i])-sum1/(data.size()-2)), 2);
  }
  return std::pow(sq_diff1 / (data.size()-2), 0.5);
}

Real
AdaptiveMonteCarloUtils::computeMEAN(const std::vector<Real> & data)
{
  Real sum1 = 0.0;
  for (unsigned int i = 2; i < data.size(); ++i)
  {
    sum1 += (data[i]);
  }
  return (sum1 / (data.size()-2));
  // auto local_sum = std::accumulate(data.begin(), data.end(), 0.);
  // auto local_count = data.size();
  // return data.empty() ? 0. : local_sum / local_count;
}

Real
AdaptiveMonteCarloUtils::computeMIN(const std::vector<Real> & data)
{
  // Real tmp = data[0];
  // for (unsigned int i = 0; i < data.size(); ++i)
  // {
  //   if(tmp>(data[i]))
  //   {
  //      tmp=(data[i]);
  //   }
  // }
  // return tmp;
  auto local_min = *std::min_element(data.begin(), data.end());
  return local_min;
}
