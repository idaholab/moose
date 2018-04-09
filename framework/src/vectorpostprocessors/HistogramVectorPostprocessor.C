//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HistogramVectorPostprocessor.h"

#include <algorithm>

registerMooseObject("MooseApp", HistogramVectorPostprocessor);

template <>
InputParameters
validParams<HistogramVectorPostprocessor>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();
  params.addClassDescription("Compute a histogram for each column of a VectorPostprocessor");

  params.addRequiredParam<VectorPostprocessorName>(
      "vpp", "The VectorPostprocessor to compute histogram of");

  params.addRequiredParam<unsigned int>("num_bins", "The number of bins for the histograms");

  return params;
}

HistogramVectorPostprocessor::HistogramVectorPostprocessor(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _vpp_name(getParam<VectorPostprocessorName>("vpp")),
    _num_bins(getParam<unsigned int>("num_bins"))
{
}

void
HistogramVectorPostprocessor::initialize()
{
  const auto & vpp_vectors = _fe_problem.getVectorPostprocessorVectors(_vpp_name);

  // Add vectors for each column, the reason for the extra logic here is that the columns a VPP
  // produces can change
  for (const auto & the_pair : vpp_vectors)
  {
    const auto & name = the_pair.first;

    if (_histogram_data.find(name) == _histogram_data.end())
      _histogram_data[name] = {
          &declareVector(name + "_lower"), &declareVector(name + "_upper"), &declareVector(name)};
  }
}

void
HistogramVectorPostprocessor::execute()
{
  if (processor_id() == 0) // Only compute on processor 0
  {
    const auto & vpp_vectors = _fe_problem.getVectorPostprocessorVectors(_vpp_name);

    // For each value vector compute the histogram
    for (auto & the_pair : vpp_vectors)
    {
      const auto & name = the_pair.first;
      const auto & values = *the_pair.second.current;

      mooseAssert(_histogram_data.count(name), "Error retrieving VPP vector");
      auto & histo_data = _histogram_data.at(name);

      computeHistogram(values, histo_data);
    }
  }
}

void
HistogramVectorPostprocessor::finalize()
{
}

void
HistogramVectorPostprocessor::computeHistogram(const std::vector<Real> & values,
                                               HistoData & histo_data)
{
  if (values.empty())
    mooseError("Cannot compute histogram without data!");

  // Grab the vectors to fill
  auto & lower_vector = *histo_data._lower;
  auto & upper_vector = *histo_data._upper;
  auto & histogram = *histo_data._histogram;

  // Resize everything
  // Note: no need to zero anything out
  // that will automatically be done if the bin should be zero by the algorithm below
  lower_vector.resize(_num_bins);
  upper_vector.resize(_num_bins);
  histogram.resize(_num_bins);

  // Create a sorted copy of the values
  std::vector<Real> sorted_values(values.size());
  std::partial_sort_copy(values.begin(), values.end(), sorted_values.begin(), sorted_values.end());

  // Get the min and max values
  auto min = sorted_values.front();
  auto max = sorted_values.back();

  // The bin stride/length
  auto bin_stride = (max - min) / static_cast<Real>(_num_bins);

  auto current_value_iter = sorted_values.begin();
  auto sorted_values_end = sorted_values.end();

  // Fill the bins
  for (unsigned int bin = 0; bin < _num_bins; bin++)
  {
    // Compute bin edges
    // These are computed individually on purpose so that the exact same values will match the
    // previous and next bins
    auto lower = (bin * bin_stride) + min;
    auto upper = ((bin + 1) * bin_stride) + min;

    lower_vector[bin] = lower;
    upper_vector[bin] = upper;

    // Find the number of values that fall in this bin
    unsigned long int num_values = 0;
    while (current_value_iter != sorted_values_end && *current_value_iter <= upper)
    {
      num_values++;
      current_value_iter++;
    }

    histogram[bin] = static_cast<Real>(num_values);
  }
}
