//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LatinHypercubeSampler.h"
#include "Distribution.h"

registerMooseObjectAliased("StochasticToolsApp", LatinHypercubeSampler, "LatinHypercube");

InputParameters
LatinHypercubeSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Latin Hypercube Sampler.");
  params.addRequiredParam<dof_id_type>("num_rows",
                                       "The number of rows per matrix to generate; a single value "
                                       "or one value per distribution may be supplied.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addRequiredParam<std::vector<unsigned int>>(
      "num_bins", "The number of intervals to consider within the sampling.");
  params.addParam<std::vector<Real>>(
      "lower_limits",
      std::vector<Real>(1, 0),
      "The lower limit of probability for each of the associated distributions.");
  params.addParam<std::vector<Real>>(
      "upper_limits",
      std::vector<Real>(1, 1),
      "The upper limit of probability for each of the associated distributions.");
  return params;
}

LatinHypercubeSampler::LatinHypercubeSampler(const InputParameters & parameters)
  : Sampler(parameters),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _num_bins_input(getParam<std::vector<unsigned int>>("num_bins")),
    _upper_limits_input(getParam<std::vector<Real>>("upper_limits")),
    _lower_limits_input(getParam<std::vector<Real>>("lower_limits"))
{
  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  if (_upper_limits_input.size() != 1 && _upper_limits_input.size() != _distributions.size())
    paramError("upper_limits",
               "The length of 'upper_limits' must be one or match the length of 'distributions'");

  if (_lower_limits_input.size() != 1 && _lower_limits_input.size() != _distributions.size())
    paramError("lower_limits",
               "The length of 'lower_limits' must be one or match the length of 'distributions'");

  if (_num_bins_input.size() != 1 && _num_bins_input.size() != _distributions.size())
    paramError("num_bins",
               "The length of 'num_bins' must be one or match the length of 'distributions'");

  for (const auto & lim : _upper_limits_input)
    if (lim < 0 || lim > 1)
      paramError("upper_limits", "The items in 'upper_limits' must be in the range [0,1]");

  for (const auto & lim : _lower_limits_input)
    if (lim < 0 || lim > 1)
      paramError("lower_limits", "The items in 'lower_limits' must be in the range [0,1]");

  for (std::size_t i = 0; i < _lower_limits_input.size(); ++i)
    if (_lower_limits_input[i] >= _upper_limits_input[i])
      paramError(
          "lower_limits",
          "The items in 'lower_limits' must be less than the corresponding 'upper_limits' value");

  setNumberOfRandomSeeds(2); // 0 = Bin seed; 1 = distribution sample seed
  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(_distributions.size());
}

void
LatinHypercubeSampler::sampleSetUp()
{
  // Setup bin and limit information
  const std::size_t num_dists = _distributions.size();
  _size_bins.resize(num_dists);
  _num_bins = _num_bins_input.size() == 1 ? std::vector<unsigned int>(num_dists, _num_bins_input[0])
                                          : _num_bins_input;

  _lower_limits = _lower_limits_input.size() == 1
                      ? std::vector<Real>(num_dists, _lower_limits_input[0])
                      : _lower_limits_input;
  _upper_limits = _upper_limits_input.size() == 1
                      ? std::vector<Real>(num_dists, _upper_limits_input[0])
                      : _upper_limits_input;

  for (std::size_t dist_idx = 0; dist_idx < num_dists; ++dist_idx)
  {
    const Real lower = _lower_limits[dist_idx];
    const Real upper = _upper_limits[dist_idx];
    _size_bins[dist_idx] = (upper - lower) / _num_bins[dist_idx];
  }
}

Real
LatinHypercubeSampler::computeSample(dof_id_type /*row_index*/, dof_id_type col_index)
{
  // Determine the bin
  const uint32_t bin_num = getRandl(0, 0, _num_bins[col_index]);

  // Compute probability in the range within the bin
  Real lower = bin_num * _size_bins[col_index] + _lower_limits[col_index];
  Real prob = getRand(1) * _size_bins[col_index] + lower;
  mooseAssert(prob >= _lower_limits[col_index] && prob <= _upper_limits[col_index],
              "Computed probability out of range.");

  // Sample the distribution
  return _distributions[col_index]->quantile(prob);
}

void
LatinHypercubeSampler::sampleTearDown()
{
  _num_bins.clear();
  _size_bins.clear();
}
