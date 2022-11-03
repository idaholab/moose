//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActiveLearningReporterBase.h"
#include "ActiveLearningGaussianProcess.h"
#include "GaussianProcess.h"
#include "SurrogateModelInterface.h"

class ActiveLearningGPDecision : public ActiveLearningReporterTempl<Real>,
                                 public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  ActiveLearningGPDecision(const InputParameters & parameters);

protected:
  /**
   * This evaluates the inputted function to determine whether a multiapp solve is
   * necessary/allowed, otherwise it replaces the "transferred" quantity with a
   * default value.
   */
  virtual bool needSample(const std::vector<Real> & row,
                          dof_id_type local_ind,
                          dof_id_type global_ind,
                          Real & val) override;

private:
  /**
   * This evaluates the active learning acquisition function and returns bool
   * that indicates whether a full model evaluation is required or not.
   */
  bool learningFunction(const Real & gp_mean,
                        const Real & gp_std,
                        const MooseEnum & function_name,
                        const Real & parameter,
                        const Real & threshold);

  /**
   * This sets up data for re-training the GP.
   */
  void setupData(const std::vector<Real> & output_parallel,
                 const std::vector<std::vector<Real>> & inputs_prev);

  /**
   * This makes decisions whether to call the full model or not based on
   * GP prediction and uncertainty.
   */
  void facilitateDecision(const std::vector<Real> & row,
                          dof_id_type local_ind,
                          Real & val,
                          const bool & retrain);

  /**
   * This transmits GP outputs to file.
   */
  void transferOutput(const DenseMatrix<Real> & inputs_parallel,
                      const std::vector<Real> & gp_mean_parallel,
                      const std::vector<Real> & gp_std_parallel);

  /// Track the current step of the main App
  const int & _step;

  /// The learning function for active learning
  const MooseEnum & _learning_function;

  /// The learning function threshold
  const Real & _learning_function_threshold;

  /// The learning function parameter
  const Real * _learning_function_parameter;

  /// The active learning GP trainer that permits re-training
  const ActiveLearningGaussianProcess & _al_gp;

  /// The GP evaluator object that permits re-evaluations
  const SurrogateModel & _gp_eval;

  /// The Monte Carlo sampler
  Sampler & _sampler;

  /// Flag samples when the GP fails
  std::vector<bool> & _flag_sample;

  /// Number of initial training points for GP
  const int _n_train;

  /// Storage for the input vectors to be transferred to the output file
  std::vector<std::vector<Real>> & _inputs;

  /// Broadcast the GP mean prediciton to JSON
  std::vector<Real> & _gp_mean;

  /// Broadcast the GP standard deviation to JSON
  std::vector<Real> & _gp_std;

  /// Store all the input vectors in the batch
  std::vector<std::vector<Real>> _inputs_batch;

  /// Store all the outputs in the batch
  std::vector<Real> _outputs_batch;

  /// Store all the input vectors in the batch from the previous step
  std::vector<std::vector<Real>> _inputs_batch_prev;

  /// GP pass/fail decision
  std::vector<bool> _decision;

  /// Track GP fails
  unsigned int _track_gp_fails;

  /// Store the user-specified number of allowed GP fails
  unsigned int _allowed_gp_fails;

  /// For parallelization
  libMesh::Parallel::Communicator _local_comm;

  /// Store GP outputs
  std::vector<Real> _gp_sto;

  /// Facilitate allGather of outputs
  std::vector<Real> _output_parallel;

  /// Facilitate allGather of GP mean predictions
  std::vector<Real> _gp_mean_parallel;

  /// Facilitate allGather of GP prediction standard deviations
  std::vector<Real> _gp_std_parallel;
};
