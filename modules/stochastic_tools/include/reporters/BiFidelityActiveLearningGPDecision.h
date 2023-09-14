//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActiveLearningGPDecision.h"

/**
 * A class for performing active learning decision making in bi-fidelity modeling
 */
class BiFidelityActiveLearningGPDecision : public ActiveLearningGPDecision
{
public:
  static InputParameters validParams();

  BiFidelityActiveLearningGPDecision(const InputParameters & parameters);

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
   *
   * @param row Input parameters to the model
   * @param local_ind Current processor row index
   * @param global_ind All processors row index
   * @param val Output predicted by either the LF model + GP correction or the HF model
   * @return bool Whether a full order model evaluation is required
   */
  virtual bool needSample(const std::vector<Real> & row,
                          dof_id_type local_ind,
                          dof_id_type global_ind,
                          Real & val) override;

  /**
   * This makes decisions whether to call the full model or not based on
   * GP prediction and uncertainty.
   *
   * @return bool Whether a full order model evaluation is required
   */
  virtual bool facilitateDecision() override;

private:
  /// The sampler
  Sampler & _sampler;

  /// Store all the outputs used for training from the LF model
  const std::vector<Real> & _outputs_lf;

  /// Store all the outputs used for training from the LF model
  std::vector<Real> _outputs_lf_batch;

  /// Broadcast the GP-corrected LF prediciton to JSON
  std::vector<Real> & _lf_corrected;

  /// Communicator that was split based on samples that have rows
  libMesh::Parallel::Communicator & _local_comm;
};
