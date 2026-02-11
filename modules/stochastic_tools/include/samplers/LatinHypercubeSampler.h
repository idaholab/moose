//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"

/**
 * A class used to perform Monte Carlo Sampling
 *
 * WARNING! This Sampler manages the generators advancement for parallel operation manually.
 */
class LatinHypercubeSampler : public Sampler
{
public:
  static InputParameters validParams();

  LatinHypercubeSampler(const InputParameters & parameters);

protected:
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;
  virtual void computeSampleMatrix(DenseMatrix<Real> & matrix) override;
  virtual void computeLocalSampleMatrix(DenseMatrix<Real> & matrix) override;
  virtual void computeSampleRow(dof_id_type i, std::vector<Real> & data) override;

  virtual std::size_t getStatelessAdvanceCount(unsigned int seed_index) const override;
  virtual void finalizeStatelessAdvance() override;

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

private:
  void buildProbabilities(const Sampler::SampleMode mode);

  /**
   * Shuffle helper using stateless RNGs for reproducible, index-based permutations.
   */
  void
  shuffleStateless(std::vector<Real> & data, const std::size_t seed_index, const CommMethod method);

  // Probability values for each distribution
  std::vector<std::vector<Real>> _probabilities;

  // toggle for local/global data access in computeSample
  bool _is_local;

  // Guard to avoid rebuilding probabilities during matrix construction.
  bool _building_matrix = false;

  // Track whether probabilities are ready for row iteration.
  bool _probabilities_ready = false;

  // Track whether stateless RNGs should be advanced at execute tear down.
  bool _stateless_advance_pending = false;
};
