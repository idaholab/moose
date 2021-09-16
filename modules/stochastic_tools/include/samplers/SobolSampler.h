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

  /**
   * Sobol sampling should have a slightly different partitioning in order to keep
   * the sample and resample samplers distributed and make computing indices more
   * efficient.
   */
  virtual LocalRankConfig constructRankConfig(bool batch_mode) const override;

  ///@{
  /// Sobol Monte Carlo rows
  std::vector<Real> _row_a;
  std::vector<Real> _row_b;
  ///@}

  /// Sampler matrix
  Sampler & _sampler_a;

  /// Re-sample matrix
  Sampler & _sampler_b;

  /// Flag for building the re-sampling matrix for computing second order sensitivity indices
  const bool & _resample;

private:
  /// Number of matrices
  const dof_id_type _num_matrices;
};
