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
#include "AffineInvariantDESGPry.h"
#include "SurrogateModelInterface.h"
#include "Distribution.h"

/**
 * AIDESGPryTestTransform will help making sample accept/reject decisions in MCMC
 * schemes (for e.g., when performing Bayesian inference).
 */
class AIDESGPryTestTransform : public GeneralReporter, public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  AIDESGPryTestTransform(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  /**
   * Compute the evidence (aka, betterness of the proposed sample vs the previous)
   * @param evidence The evidence vector to be filled
   * @param input_matrix The matrix of proposed inputs that are provided
   */
  virtual void computeEvidence(std::vector<Real> & evidence,
                               const DenseMatrix<Real> & input_matrix);

  /**
   * Compute the transition probability vector (after the computation of evidence)
   * @param tv The transition probability vector to be filled
   * @param evidence The vector of evidences provided
   */
  virtual void computeTransitionVector(std::vector<Real> & tv, const std::vector<Real> & evidence);

  Real correctGP(const Real & GPoutput, const Real & trueVariance);

  /**
   * Resample inputs given the transition vector (after transition vector computed)
   * @param req_inputs The vector of accepted samples to be filled
   * @param input_matrix The matrix of proposed inputs provided
   * @param tv The vector of transition probabilities provided
   * @param parallel_index The current parallel proposal index provided
   */
  virtual void nextSamples(std::vector<Real> & req_inputs,
                           DenseMatrix<Real> & input_matrix,
                           const std::vector<Real> & tv,
                           const unsigned int & parallel_index);

  /**
   * Compute the next set of seeds to facilitate proposals.
   * Set to empty in base to permit flexibility for MCMC samplers
   */
  virtual void nextSeeds() {}

  /// Transfer the right outputs to the file
  std::vector<Real> & _outputs_required;

  /// Model input data that is uncertain
  std::vector<std::vector<Real>> & _inputs;

  /// Transition probability matrix
  std::vector<Real> & _tpm;

  /// Model variance term
  std::vector<Real> & _variance;

  /// The MCMC sampler
  Sampler & _sampler;

  /// MCMC sampler base
  const AffineInvariantDESGPry * const _pmcmc;

  /// Storage for the number of parallel proposals
  dof_id_type _props;

  /// Storage for the random numbers for decision making
  const std::vector<Real> & _rnd_vec;

  /// Storage for new proposed variance samples
  const std::vector<Real> & _new_var_samples;

  /// Storage for the priors
  const std::vector<const Distribution *> _priors;

  /// Storage for the prior over the variance
  const Distribution * _var_prior;

  /// Storage for previous inputs
  DenseMatrix<Real> _data_prev;

  /// Storage for previous variances
  std::vector<Real> _var_prev;

  /// Storage for previous outputs
  std::vector<Real> _outputs_prev;

  /// The GP evaluator object
  const SurrogateModel & _gp_eval;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;

  /// Communicator that was split based on samples that have rows
  libMesh::Parallel::Communicator & _local_comm;

  /// Transition probability matrix
  std::vector<Real> & _estimated_loglikelihood;
};
