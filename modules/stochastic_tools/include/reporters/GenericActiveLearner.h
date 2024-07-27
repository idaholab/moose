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
#include "GenericActiveLearningSampler.h"
#include "ActiveLearningGaussianProcess.h"
#include "GaussianProcess.h"
#include "SurrogateModel.h"
#include "SurrogateModelInterface.h"
#include "GaussianProcessSurrogate.h"
#include "ParallelAcquisitionFunctionBase.h"
#include "ParallelAcquisitionInterface.h"

/**
 * A generic reporter to support parallel active learning: re-trains GP and picks the next best batch
 */
class GenericActiveLearner : public GeneralReporter,
                             public ParallelAcquisitionInterface,
                             public SurrogateModelInterface

{
public:
  static InputParameters validParams();
  GenericActiveLearner(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  /**
   * Sets up the training data for the GP model
   * @param data_out The data vector containing the outputs to train the GP
   * @param data_in The data matrix containing the inputs to train the GP
   */
  virtual void setupGPData(const std::vector<Real> & data_out, const DenseMatrix<Real> & data_in);

  /**
   * Computes the outputs of the trained GP model
   * @param eval_outputs The outputs predicted by the GP model
   */
  virtual void computeGPOutput(std::vector<Real> & eval_outputs);

  /**
   * Computes the convergence value during active learning
   */
  virtual void computeConvergenceValue();

  /**
   * Evaluate the GP on all the test samples sent by the Sampler
   */
  virtual void evaluateGPTest();

  /**
    Setup the generic variable for acquisition computation (depends on the objective:
    optimization, UQ, etc.)
  */
  virtual void setupGeneric();

  /**
   * Include additional inputs before evaluating the acquisition function.
   * Has trivial function in base, but can be modified in derived if necessary depending 
   * upon the objective of active learning
   */
  virtual void includeAdditionalInputs();

  /**
   * Output the acquisition function values and ordering of the indices
   * @param acq_new The computed values of the acquisition function
   * @param indices The indices ordered according to the acqusition values to be sent to Sampler
   */
  virtual void getAcquisition(std::vector<Real> & acq_new, std::vector<unsigned int> & indices);

  /// Model output value from SubApp
  const std::vector<Real> & _output_value;

  /// Modified value of model output by this reporter class
  std::vector<Real> & _output_comm;

  /// The adaptive Monte Carlo sampler
  Sampler & _sampler;

  /// Adaptive Importance Sampler
  const GenericActiveLearningSampler * const _al_sampler;

  /// The selected sample indices to evaluate the subApp
  std::vector<unsigned int> & _sorted_indices;

  /// Storage for all the proposed samples to test the GP model
  const std::vector<std::vector<Real>> & _inputs_test;

  /// The active learning GP trainer that permits re-training
  const ActiveLearningGaussianProcess & _al_gp;

  /// The GP evaluator object that permits re-evaluations
  const SurrogateModel & _gp_eval;

  /// Storage for the parallel acquisition object to be utilized
  ParallelAcquisitionFunctionBase * _acquisition_obj;

  /// The acquistion function values in the current iteration
  std::vector<Real> & _acquisition_value;

  /// For monitoring convergence of active learning
  Real & _convergence_value;

  /// Penalize acquisition to prevent clustering when operating in parallel
  const bool & _penalize_acquisition;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;

  /// Communicator that was split based on samples that have rows
  libMesh::Parallel::Communicator & _local_comm;

  /// Storage for the GP re-training inputs
  std::vector<std::vector<Real>> _gp_inputs;

  /// Storage for all the modified proposed samples to test the GP model
  std::vector<std::vector<Real>> _inputs_test_modified;

  /// Storage for the GP re-training outputs
  std::vector<Real> _gp_outputs;

  /// The input dimension for GP, equal to Sampler columns
  unsigned int _n_dim;

  /// Outputs of GP model for the test samples
  std::vector<Real> _gp_outputs_test;

  /// Outputs of GP model standard deviation for the test samples
  std::vector<Real> _gp_std_test;

  /// Storage for the number of parallel proposals
  dof_id_type _props;

  /// Storage for the length scales after the GP training
  std::vector<Real> _length_scales;

  /// A generic parameter to be passed to the acquisition function
  std::vector<Real> _generic;

  /// The GP outputs from the current iteration before re-training (to evaluate convergence)
  std::vector<Real> _eval_outputs_current;
};
