//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#pragma once

#include <torch/torch.h>
#include "GeneralReporter.h"
#include "Sampler.h"
#include "BayesianGPrySampler.h"
#include "ActiveLearningLibtorchNN.h"
#include "LibtorchANNSurrogate.h"
#include "ActiveLearningGaussianProcess.h"
#include "GaussianProcess.h"
#include "SurrogateModel.h"
#include "SurrogateModelInterface.h"
#include "Standardizer.h"
#include "GaussianProcess.h"
#include "LikelihoodFunctionBase.h"
#include "LikelihoodInterface.h"
// #include "CovarianceInterface.h"
// #include "GaussianProcessHandler.h"

/**
 * Fast Bayesian inference with the GPry algorithm by El Gammal et al. 2023: NN and GP training step
 */
class BayesianGPryLearner : public GeneralReporter,
                            public LikelihoodInterface,
                            // public CovarianceInterface,
                            public SurrogateModelInterface

{
public:
  static InputParameters validParams();
  BayesianGPryLearner(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

  /**
   * This function is called by LoadCovarianceDataAction when the surrogate is
   * loading training data from a file. The action must recreate the covariance
   * object before this surrogate can set the correct pointer.
   */
  // virtual void setupCovariance(UserObjectName _covar_name);

  // StochasticTools::GaussianProcessHandler & gpHandler() { return _gp_handler; }
  // const StochasticTools::GaussianProcessHandler & getGPHandler() const { return _gp_handler; }

protected:
  /// Model output value from SubApp
  const std::vector<Real> & _output_value;

  /// Modified value of model output by this reporter class
  std::vector<Real> & _output_comm;

private:
  // StochasticTools::GaussianProcessHandler & _gp_handler;

  /**
   * Sets up the training data for the neural network classifier and GP model
   * @param data_in The data matrix containing the inputs to the NNs
   */
  void setupNNGPData(const std::vector<Real> & log_posterior, const DenseMatrix<Real> & data_in);

  /**
   * Compute the log of the un-normalized posterior (aka likelihood times the prior)
   * @param log_posterior The evidence vector to be filled
   * @param input_matrix The matrix of proposed inputs that are provided
   */
  void computeLogPosterior(std::vector<Real> & log_posterior,
                           const DenseMatrix<Real> & input_matrix);

  /// The adaptive Monte Carlo sampler
  Sampler & _sampler;

  /// Adaptive Importance Sampler
  const BayesianGPrySampler * const _gpry_sampler;

  /// The selected sample indices to evaluate the subApp
  std::vector<unsigned int> & _sorted_indices;

  /// Storage for the likelihood objects to be utilized
  std::vector<const LikelihoodFunctionBase *> _likelihoods;

  /// Storage for all the proposed samples
  const std::vector<std::vector<Real>> & _inputs_all;

  /// Storage for all the proposed variance samples
  const std::vector<Real> & _var_all;

  /// The active learning NN trainer that permits re-training
  const ActiveLearningLibtorchNN & _al_nn;
  /// Get the libtorch classifer neural network
  const LibtorchANNSurrogate & _nn_eval;

  /// The active learning GP trainer that permits re-training
  const ActiveLearningGaussianProcess & _al_gp;
  /// The GP evaluator object that permits re-evaluations
  const SurrogateModel & _gp_eval;

  /// Storage for new proposed variance samples
  const std::vector<Real> & _new_var_samples;

  /// Storage for the priors
  const std::vector<const Distribution *> _priors;

  /// Storage for the prior over the variance
  const Distribution * _var_prior;

  /// Model noise term to pass to Likelihoods object
  Real & _noise;

  /// The maximum value of the acquistion function in the current iteration
  std::vector<Real> & _acquisition_function;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;

  /// Communicator that was split based on samples that have rows
  libMesh::Parallel::Communicator & _local_comm;

  /// SubApp inputs for neural network model
  std::vector<std::vector<Real>> _nn_inputs;

  /// SubApp outputs for neural network model
  std::vector<Real> _nn_outputs;

  /// SubApp inputs for GP model
  std::vector<std::vector<Real>> _gp_inputs;

  /// SubApp outputs for GP model
  std::vector<Real> _gp_outputs;

  /// Maximum number of subApp calls in each iteration
  unsigned int _num_samples;

  /// Outputs for neural network model for the try samples
  std::vector<Real> _nn_outputs_try;

  /// Outputs for GP model for the try samples
  std::vector<Real> _gp_outputs_try;

  /// Outputs for GP model standard deviation for the try samples
  std::vector<Real> _gp_std_try;

  /// Storage for the number of parallel proposals
  dof_id_type _props;

  /// Storage for the number of experimental configuration values
  dof_id_type _num_confg_values;

  /// Storage for the number of experimental configuration parameters
  dof_id_type _num_confg_params;
};

#endif
