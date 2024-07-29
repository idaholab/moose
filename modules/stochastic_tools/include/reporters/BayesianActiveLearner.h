//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericActiveLearner.h"
#include "BayesianActiveLearningSampler.h"
#include "LikelihoodFunctionBase.h"
#include "LikelihoodInterface.h"

/**
 * A reporter to support parallel active learning for Bayesian UQ tasks
 */
class BayesianActiveLearner : public GenericActiveLearner, public LikelihoodInterface

{
public:
  static InputParameters validParams();

  BayesianActiveLearner(const InputParameters & parameters);

protected:
  virtual void setupGPData(const std::vector<Real> & data_out,
                           const DenseMatrix<Real> & data_in) override;

  // virtual void computeGPOutput(std::vector<Real> & eval_outputs,
  //                              const DenseMatrix<Real> & eval_inputs) override;

  virtual void computeConvergenceValue() override;

  virtual void evaluateGPTest() override;

  virtual void includeAdditionalInputs() override;

private:
  /**
   * Sets up the training data for the GP model for Bayesian UQ tasks
   * @param log_likelihood The log-likelihood to be computed
   * @param data_out The data vector containing the outputs from subApp evaluations
   */
  void computeLogLikelihood(const std::vector<Real> & data_out);

  /// Bayesian Active Learning Sampler
  const BayesianActiveLearningSampler * const _bayes_al_sampler;

  /// Storage for new proposed variance samples
  const std::vector<Real> & _new_var_samples;

  /// Storage for the prior over the variance
  const Distribution * _var_prior;

  /// Storage for all the proposed variance samples to test the GP model
  const std::vector<Real> & _var_test;

  /// Model noise term to pass to Likelihoods object
  Real & _noise;

  /// Storage for the likelihood objects to be utilized
  std::vector<const LikelihoodFunctionBase *> _likelihoods;

  /// Storage for the number of experimental configuration values
  dof_id_type _num_confg_values;

  /// Storage for the number of experimental configuration parameters
  dof_id_type _num_confg_params;

  /// Storage for the computed log-likelihood values in each iteration of active learning
  std::vector<Real> _log_likelihood;
};
