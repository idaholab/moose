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

InputParameters
HistogramVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
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
HistogramVectorPostprocessor::initialSetup()
{
  const VectorPostprocessor & vpp = getUserObjectByName<VectorPostprocessor>(_vpp_name);
  for (const auto & vec_name : vpp.getVectorNames())
    _histogram_data[vec_name] = {&declareVector(vec_name + "_lower"),
                                 &declareVector(vec_name + "_upper"),
                                 &declareVector(vec_name)};
  if (_histogram_data.empty())
    paramError("vpp", "The specified VectorPostprocessor does not have any declared vectors");
}

void
HistogramVectorPostprocessor::initialize()
{
  // no need to reset, execute() writes in place
}

void
HistogramVectorPostprocessor::execute()
{
  if (processor_id() == 0) // Only compute on processor 0
  {
    const VectorPostprocessor & vpp = getUserObjectByName<VectorPostprocessor>(_vpp_name);
    for (const auto & vec_name : vpp.getVectorNames())
    {
      const auto & values = _fe_problem.getVectorPostprocessorValueByName(_vpp_name, vec_name);

      mooseAssert(_histogram_data.count(vec_name), "Error retrieving VPP vector");
      auto & histo_data = _histogram_data.at(vec_name);
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
