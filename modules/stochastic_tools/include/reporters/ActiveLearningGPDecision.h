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
   * This is where most of the computations happen:
   *   - Data is accumulated for training
   *   - GP models are trained
   *   - Decision is made whether more data is needed for GP training
   */
  virtual void preNeedSample() override;

  /**
   * Based on the computations in preNeedSample, the decision to get more data is passed and results
   * from the GP fills @param val
   */
  virtual bool needSample(const std::vector<Real> & row,
                          dof_id_type local_ind,
                          dof_id_type global_ind,
                          Real & val) override;

private:
  /**
   * This evaluates the active learning acquisition function and returns bool
   * that indicates whether the GP model failed.
   *
   * @param gp_mean Mean of the gaussian process model
   * @param gp_mean Standard deviation of the gaussian process model
   * @return bool If the GP model failed
   */
  bool learningFunction(const Real & gp_mean, const Real & gp_std) const;

  /**
   * This sets up data for re-training the GP.
   *
   * @param inputs Matrix of inputs for the current step
   * @param outputs Vector of outputs for the current step
   */
  void setupData(const std::vector<std::vector<Real>> & inputs, const std::vector<Real> & outputs);

  /**
   * This makes decisions whether to call the full model or not based on
   * GP prediction and uncertainty.
   *
   * @return bool Whether a full order model evaluation is required
   */
  bool facilitateDecision();

  /// Track the current step of the main App
  const int & _step;

  /// The learning function for active learning
  const MooseEnum & _learning_function;
  /// The learning function threshold
  const Real & _learning_function_threshold;
  /// The learning function parameter
  const Real & _learning_function_parameter;

  /// The active learning GP trainer that permits re-training
  const ActiveLearningGaussianProcess & _al_gp;
  /// The GP evaluator object that permits re-evaluations
  const SurrogateModel & _gp_eval;

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

  /// GP pass/fail decision
  bool _decision;

  /// Reference to global input data requested from base class
  const std::vector<std::vector<Real>> & _inputs_global;
  /// Reference to global output data requested from base class
  const std::vector<Real> & _outputs_global;

  /// Store all the input vectors used for training
  std::vector<std::vector<Real>> _inputs_batch;
  /// Store all the outputs used for training
  std::vector<Real> _outputs_batch;
};
