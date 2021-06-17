//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"

/**
 * A class used to perform Monte Carlo sampling for performing Sobol sensitivity analysis.
 *
 * The created matrices are stacked in the following order, following the nomenclature from
 * Saltelli (2002), "Making best use of model evaluations to compute sensitivity indices"
 *
 * with re-sampling: [M2, N_1, ..., N_n, N_-1, ..., N-n, M1]
 * without re-sampling: [M2, N_1, ..., N_n, M1]
 */
class SobolSampler : public Sampler
{
public:
  static InputParameters validParams();

  SobolSampler(const InputParameters & parameters);

  /// Resampling flag, see SobolStatistics
  bool resample() const { return _resample; }

protected:
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;
  virtual void sampleSetUp(const Sampler::SampleMode mode) override;
  virtual void sampleTearDown(const Sampler::SampleMode mode) override;

  ///@{
  /// Sobol Monte Carlo matrices, these are sized and cleared to avoid keeping large matrices around
  DenseMatrix<Real> _m1_matrix;
  DenseMatrix<Real> _m2_matrix;
  ///@}

  /// Sampler matrix
  Sampler & _sampler_a;

  /// Re-sample matrix
  Sampler & _sampler_b;

  /// Flag for building the re-sampling matrix for computing second order sensitivity indices
  const bool & _resample;

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

private:
  /// The number of rows per matrix
  dof_id_type _num_rows_per_matrix;

  /// Number of matrices
  dof_id_type _num_matrices;

  /// PerfGraph timer
  const PerfID _perf_sample_setup;
  const PerfID _perf_sample_teardown;
  const PerfID _perf_compute_sample;
};
