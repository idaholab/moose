//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ReporterInterface.h"

/**
 * Fast Bayesian inference with the GPry algorithm by El Gammal et al. 2023: sampler step
 */
class GPryTest : public Sampler, public TransientInterface
{
public:
  static InputParameters validParams();

  GPryTest(const InputParameters & parameters);

  /**
   * Return the random samples for the GP and NN combo to try in the reporter class
   */
  const std::vector<std::vector<Real>> & getSampleTries() const;

  /**
   * Return the number of parallel proposals.
   */
  dof_id_type getNumParallelProposals() const { return _num_parallel_proposals; }

protected:
  /// Gather all the samples
  virtual void sampleSetUp(const Sampler::SampleMode mode) override;
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /**
   * Fill in the provided vector with random samples given the distributions
   * @param vector The vector to be filled
   * @param seed_value The seed value to generate random numbers
   */
  virtual void fillVector(std::vector<Real> & vector, const unsigned int & seed_value);

  /// The selected sample indices to evaluate the subApp
  const std::vector<unsigned int> & _sorted_indices;

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

private:
  /// Number of samples to propose in each iteration (not all are sent for subApp evals)
  const unsigned int & _num_parallel_proposals;

  /// Number of samples to propose in each iteration (not all are sent for subApp evals)
  const unsigned int & _num_tries;

  /// Storage for all the proposed samples
  std::vector<std::vector<Real>> _inputs_all;

  /// A temporary vector to facilitate the sampling
  std::vector<Real> _sample_vector;

  /// Storage for previously accepted samples by the decision reporter system
  std::vector<std::vector<Real>> _inputs_sto;

  /// Ensure that the sampler proceeds in a sequential fashion
  int _check_step;
};
