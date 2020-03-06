//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LatinHypercube.h"
#include "Distribution.h"

registerMooseObject("StochasticToolsApp", LatinHypercube);

defineLegacyParams(LatinHypercube);

InputParameters
LatinHypercube::validParams()
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
  params.addRequiredParam<std::vector<double>>(
      "upper_limits", "The lower limit of probability for each of the associated distributions.");
  params.addRequiredParam<std::vector<double>>(
      "lower_limits", "The upper limit of probability for each of the associated distributions.");
  return params;
}

LatinHypercube::LatinHypercube(const InputParameters & parameters)
  : Sampler(parameters),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _num_bins_input(getParam<std::vector<unsigned int>>("num_bins")),
    _upper_limits(getParam<std::vector<double>>("upper_limits")),
    _lower_limits(getParam<std::vector<double>>("lower_limits"))
{
  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  if (_upper_limits.size() != _distributions.size())
    paramError("upper_limits",
               "The length of 'upper_limits' must match the length of 'distributions'");

  if (_lower_limits.size() != _distributions.size())
    paramError("lower_limits",
               "The length of 'lower_limits' must match the length of 'distributions'");

  if (_num_bins_input.size() != 1 && _num_bins_input.size() != _distributions.size())
    paramError("num_bins",
               "The length of 'num_bins' must be one or match the length of 'distributions'");

  for (const auto & lim : _upper_limits)
    if (lim < 0 || lim > 1)
      paramError("upper_limits", "The items in 'upper_limits' must be in the range [0,1]");

  for (const auto & lim : _lower_limits)
    if (lim < 0 || lim > 1)
      paramError("lower_limits", "The items in 'lower_limits' must be in the range [0,1]");

  setNumberOfRandomSeeds(2); // 0 = Bin seed; 1 = distribution sample seed
  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(_distributions.size());
}

void
LatinHypercube::sampleSetUp()
{
  // Setup bin information
  const std::size_t num_dists = _distributions.size();
  _size_bins.resize(num_dists);
  _num_bins = _num_bins_input.size() == 1 ? std::vector<unsigned int>(_num_bins_input[0], num_dists)
                                          : _num_bins_input;

  for (std::size_t dist_idx = 0; dist_idx < num_dists; ++dist_idx)
  {
    const double lower = _lower_limits[dist_idx];
    const double upper = _upper_limits[dist_idx];
    _size_bins[dist_idx] = (upper - lower) / _num_bins[dist_idx];
  }
}

Real
LatinHypercube::computeSample(dof_id_type /*row_index*/, dof_id_type col_index)
{
  // Determine the bin
  const uint32_t bin_num = getRandl(0, 0, _num_bins[col_index]);

  // Compute probability in the range within the bin
  double lower = bin_num * _size_bins[col_index];
  double prob = getRand(1) * _size_bins[col_index] + lower;

  // Sample the distribution
  return _distributions[col_index]->quantile(prob);
}

void
LatinHypercube::sampleTearDown()
{
  _num_bins.clear();
  _size_bins.clear();
}
