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
#include "TransientInterface.h"
#include "Distribution.h"

/**
 * A generic sampler to support parallel active learning
 */
class GenericActiveLearningSampler : public Sampler, public TransientInterface
{
public:
  static InputParameters validParams();

  GenericActiveLearningSampler(const InputParameters & parameters);

  /**
   * Return random samples for the GP to try in the reporter class
   */
  const std::vector<std::vector<Real>> & getSampleTries() const;

  /**
   * Return the number of parallel proposals.
   */
  dof_id_type getNumParallelProposals() const { return _num_parallel_proposals; }

protected:
  /**
   * Return the sample for the given row and column.
   */
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /**
   * Fill in the provided vector with random samples given the distributions
   * @param vector The vector to be filled
   * @param seed_value The seed value to generate random numbers
   * @param rn_ind The stateless RNG index to advance
   */
  virtual void
  fillVector(std::vector<Real> & vector, const unsigned int & seed_value, std::size_t & rn_ind);

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

  /// Number of parallel proposals to be made and subApps to be executed
  const unsigned int _num_parallel_proposals;

  /// The selected sample indices to evaluate the subApp
  const std::vector<unsigned int> & _sorted_indices;

  /// Ensure that the algorithm proceeds in a sequential fashion
  int _check_step;

  /// Initial values of the input params to get the MCMC scheme started
  const std::vector<Real> & _initial_values;

  /// Vectors of new proposed samples
  std::vector<std::vector<Real>> _new_samples;

private:
  /// Refresh stored samples for the current step.
  void updateSamples();

  /// Number of samples to propose in each iteration (not all are sent for subApp evals)
  const unsigned int _num_tries;

  /// Storage for all the proposed samples
  std::vector<std::vector<Real>> _inputs_all;
};
