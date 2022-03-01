//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

/**
 * Computes a histogram for each column in a given VectorPostprocessor
 *
 * Creates three columns for each original column:
 * column_name: The histogram
 * column_name_lower: The lower bound for each bin
 * column_name_upper: The upper bound for each bin
 *
 *
 */
class HistogramVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  HistogramVectorPostprocessor(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /**
   * Helper class to hold the vectors for the histogram
   */
  struct HistoData
  {
    /// The lower edges for each bin
    VectorPostprocessorValue * _lower;

    /// The upper edges for each bin
    VectorPostprocessorValue * _upper;

    /// Where the data will actually be stored
    VectorPostprocessorValue * _histogram;
  };

  /**
   * Compute the histogram of a vector and fill in the lower/upper edges
   *
   * @param stat_vector The vector to compute a Histogram of
   * @histo_data The HistoData object that will be filled in
   */
  void computeHistogram(const std::vector<Real> & values, HistoData & histo_data);

  /// The name of the VPP to work on
  const VectorPostprocessorName & _vpp_name;

  /// The number of bins
  const unsigned int & _num_bins;

  /// The VPP vectors that will hold the Histogram for each column
  std::map<std::string, HistoData> _histogram_data;
};
