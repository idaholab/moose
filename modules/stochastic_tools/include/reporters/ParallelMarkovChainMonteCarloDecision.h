//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "ParallelMarkovChainMonteCarloBase.h"
#include "Likelihood.h"
#include "LikelihoodInterface.h"

/**
 * ParallelMarkovChainMonteCarloDecision will help make sample accept/reject decisions in MCMC
 * schemes (for e.g., when performing Bayesian inference).
 */
class ParallelMarkovChainMonteCarloDecision : public GeneralReporter, public LikelihoodInterface
{
public:
  static InputParameters validParams();
  ParallelMarkovChainMonteCarloDecision(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  /**
   * Compute the evidence (aka, betterness of the proposed sample vs the previous)
   */
  virtual void computeEvidence(std::vector<Real> & evidence, DenseMatrix<Real> & inputs_matrix);

  /**
   * Compute the transition probability vector (after the computation of evidence)
   */
  virtual void computeTransitionVector(std::vector<Real> & tv, std::vector<Real> & evidence);

  /**
   * Resample inputs given the transition vector (after transition vector computed)
   */
  virtual void nextSamples(std::vector<Real> & req_inputs,
                           DenseMatrix<Real> & inputs_matrix,
                           const std::vector<Real> & tv,
                           const unsigned int & parallel_index);

  /**
   * Compute the next set of seeds to facilitate proposals
   * Set to empty in base to permit flexibility for MCMC samplers
   */
  virtual void nextSeeds() {}

  /// Model output value from SubApp
  const std::vector<Real> & _output_value;

  /// Transfer the right outputs to the file
  std::vector<Real> & _outputs_required;

  /// Model input data that is uncertain
  std::vector<std::vector<Real>> & _inputs;

  /// Transition probability matrix
  std::vector<Real> & _tpm;

  /// Storage for the likelihood objects to be utilized
  std::vector<const Likelihood *> _likelihoods;

  /// Storage for prior distribution objects to be utilized
  std::vector<const Distribution *> _priors;

  /// The MCMC sampler
  Sampler & _sampler;

  /// MCMC sampler base
  const ParallelMarkovChainMonteCarloBase * const _pmcmc;

  /// Storage for the previous likelihood
  // Real _likelihood_prev;

  /// Storage for the number of parallel proposals
  dof_id_type _props;

  /// Storage for the random numbers for decision making
  const std::vector<Real> & _rnd_vec;

  /// Storage for the number of experimental configurations
  dof_id_type _num_confg;

  /// Storage for previous inputs
  DenseMatrix<Real> _data_prev;

  /// Storage for previous outputs
  std::vector<Real> _outputs_prev;

private:
  /// Track the current step of the main App
  const int & _step;

  /// Communicator that was split based on samples that have rows
  libMesh::Parallel::Communicator _local_comm;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;
};
