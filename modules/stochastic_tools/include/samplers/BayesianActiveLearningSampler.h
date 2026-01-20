//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PMCMCBase.h"
#include "ReporterInterface.h"

/**
 * Fast Bayesian inference with the parallel active learning (partly inspired from El Gammal et al.
 * 2023)
 */
class BayesianActiveLearningSampler : public PMCMCBase
{
public:
  static InputParameters validParams();

  BayesianActiveLearningSampler(const InputParameters & parameters);

  /**
   * Return the random samples for the GP to try in the reporter class
   */
  const std::vector<std::vector<Real>> & getSampleTries() const;

  /**
   * Return the random variance samples for the GP to try in the reporter class
   */
  const std::vector<Real> & getVarSampleTries() const;

protected:
  /**
   * Propose new samples and trial points using stateless RNG draws.
   * @param seed_value The seed for the random number generator
   * @param rn_ind The stateless RNG index to advance
   */
  virtual void proposeSamples(const unsigned int seed_value, std::size_t & rn_ind) override;

  /**
   * Fill in the provided vector with random samples given the distributions
   * @param vector The vector to be filled
   * @param seed_value The seed value to generate random numbers
   * @param rn_ind The stateless RNG index to advance
   */
  virtual void
  fillVector(std::vector<Real> & vector, const unsigned int & seed_value, std::size_t & rn_ind);

  /// The selected sample indices to evaluate the subApp
  const std::vector<unsigned int> & _sorted_indices;

private:
  /// Number of samples to propose in each iteration (not all are sent for subApp evals)
  const unsigned int & _num_tries;

  /// Storage for all the proposed samples
  std::vector<std::vector<Real>> _inputs_test;

  /// Storage for all the proposed variances
  std::vector<Real> _var_test;
};
