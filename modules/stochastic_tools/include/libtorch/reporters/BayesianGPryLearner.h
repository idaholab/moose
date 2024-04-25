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

#include "GeneralReporter.h"
#include "Sampler.h"
#include "BayesianGPrySampler.h"
#include "ActiveLearningLibtorchNN.h"

/**
 * Fast Bayesian inference with the GPry algorithm by El Gammal et al. 2023: NN and GP training step
 */
class BayesianGPryLearner : public GeneralReporter
{
public:
  static InputParameters validParams();
  BayesianGPryLearner(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  /// Model output value from SubApp
  const std::vector<Real> & _output_value;

  /// Modified value of model output by this reporter class
  std::vector<Real> & _output_comm;

private:
  /**
   * Sets up the training data for the neural network classifier
   * @param data_in The data matrix containing the inputs to the NNs
   */
  virtual void setupNNData(const DenseMatrix<Real> & data_in);

  /// The adaptive Monte Carlo sampler
  Sampler & _sampler;

  /// Adaptive Importance Sampler
  const BayesianGPrySampler * const _gpry_sampler;

  /// The active learning NN trainer that permits re-training
  const ActiveLearningLibtorchNN & _al_nn;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;

  /// Communicator that was split based on samples that have rows
  libMesh::Parallel::Communicator & _local_comm;

  /// SubApp inputs for neural network model
  std::vector<std::vector<Real>> _nn_inputs;

  /// SubApp outputs for neural network model
  std::vector<Real> _nn_outputs;
};

#endif
