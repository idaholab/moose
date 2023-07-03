// //* This file is part of the MOOSE framework
// //* https://www.mooseframework.org
// //*
// //* All rights reserved, see COPYRIGHT for full restrictions
// //* https://github.com/idaholab/moose/blob/master/COPYRIGHT
// //*
// //* Licensed under LGPL 2.1, please see LICENSE for details
// //* https://www.gnu.org/licenses/lgpl-2.1.html

// #pragma once

// #include "GeneralReporter.h"
// #include "ParallelMarkovChainMonteCarloBase.h"
// #include "LikelihoodInterface.h"
// // #include "DistributionInterface.h"

// /**
//  * ParallelMarkovChainMonteCarloDecision will help make sample accept/reject decisions in MCMC
//  * schemes (for e.g., when performing Bayesian inference).
//  */
// class ParallelMarkovChainMonteCarloDecision : public GeneralReporter, public LikelihoodInterface
// // , public DistributionInterface
// {
// public:
//   static InputParameters validParams();
//   ParallelMarkovChainMonteCarloDecision(const InputParameters & parameters);
//   virtual void initialize() override {}
//   virtual void finalize() override {}
//   virtual void execute() override;

//   /**
//    * Compute the transition probability vector
//    */
//   void computeTransitionVector(std::vector<Real> & tv, std::vector<const Distribution *> priors,
//   std::vector<const Likelihood *> likelihoods, const DenseMatrix<Real> & inputs, const
//   std::vector<Real> & outputs, const dof_id_type & num_confg); //  const = 0
//   // void computeTransitionVector(std::vector<Real> & tv, std::vector<const Distribution *> priors, std::vector<const Likelihood *> likelihoods, const DenseMatrix<Real> & inputs, const std::vector<Real> & outputs, const dof_id_type & num_confg, const DenseMatrix<Real> & prev_inputs, const std::vector<Real> & prev_outputs); //  const = 0
//   // virtual

//   /**
//    * Resample inputs given weights
//    */
//   // void resample(const DenseMatrix<Real> & given_inputs, const std::vector<Real> & weights, std::vector<Real> & req_inputs, const dof_id_type & num_confg); //  const = 0
//   void resample(const DenseMatrix<Real> & given_inputs, const std::vector<Real> & weights,
//   std::vector<Real> & req_inputs, const dof_id_type & num_confg); //  const = 0
//   // virtual

//   /**
//    * Resample inputs given weights
//    */
//   void proposeSTD(const DenseMatrix<Real> & given_inputs, const std::vector<Real> & weights,
//   std::vector<Real> & req_inputs, const dof_id_type & num_confg); //  const = 0
//   // virtual

// protected:
//   /// Reporter value of the seed input values for proposing the next set of samples
//   std::vector<Real> & _seed_inputs;

//   /// Model output value from SubApp
//   const std::vector<Real> & _output_value;

//   /// Model input data that is uncertain
//   std::vector<std::vector<Real>> & _inputs;

//   /// Model input data that is uncertain
//   std::vector<Real> & _outputs;

//   /// TPM
//   std::vector<Real> & _tpm;

//   /// Proposal STD
//   std::vector<Real> & _proposal_std;

// private:
//   /// Track the current step of the main App
//   const int & _step;

//   /// The adaptive Monte Carlo sampler
//   Sampler & _sampler;

//   /// Adaptive Importance Sampler
//   const ParallelMarkovChainMonteCarloBase * const _pmcmc;

//   /// Ensure that the MCMC algorithm proceeds in a sequential fashion
//   int _check_step;

//   /// Storage for the likelihood objects to be utilized
//   std::vector<const Likelihood *> _likelihoods;

//   /// Storage for prior distribution objects to be utilized
//   std::vector<const Distribution *> _priors;

//   /// Communicator that was split based on samples that have rows
//   libMesh::Parallel::Communicator _local_comm;

//   /// Facilitate allGather of outputs
//   std::vector<Real> _output_comm;

//   /// Storage for previous inputs
//   DenseMatrix<Real> _data_prev;

//   std::vector<Real> _output_prev;

//   std::vector<std::vector<Real>> _inputs_sto;

// };

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
   * Compute the transition probability vector
   */
  virtual void computeTransitionVector(std::vector<Real> & tv);

  /**
   * Resample inputs given the transition vector
   */
  virtual void nextSamples(std::vector<Real> & req_inputs,
                           DenseMatrix<Real> & inputs_matrix,
                           const std::vector<Real> & tv,
                           const unsigned int & parallel_index);

  /**
   * Compute the next set of seeds to facilitate proposals
   */
  virtual void nextSeeds() {}

  /// Model output value from SubApp
  const std::vector<Real> & _output_value;

  /// Transfer the right outputs to the file
  std::vector<Real> & _outputs;

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

  /// Adaptive Importance Sampler
  const ParallelMarkovChainMonteCarloBase * const _pmcmc;

  /// Storage for the previous likelihood
  Real _likelihood_prev;

  /// Storage for the number of parallel proposals
  dof_id_type _props;

  /// Storage for the random numbers for decision making
  std::vector<Real> _rnd_vec;

  /// Storage for previous inputs
  DenseMatrix<Real> _data_prev;

  /// Storage for previous outputs
  std::vector<Real> _outputs_prev;

private:
  /// Track the current step of the main App
  const int & _step;

  /// Communicator that was split based on samples that have rows
  libMesh::Parallel::Communicator _local_comm;
};
