//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PMCMCDecision.h"
#include "IndependentGaussianMH.h"
#include "AdaptiveMonteCarloUtils.h"

/**
 * A class for performing independent Metropolis-Hastings MCMC decision making
 */
class IndependentMHDecision : public PMCMCDecision
{
public:
  static InputParameters validParams();

  IndependentMHDecision(const InputParameters & parameters);

protected:
  virtual void computeEvidence(std::vector<Real> & evidence,
                               const DenseMatrix<Real> & input_matrix) override;

  virtual void computeTransitionVector(std::vector<Real> & tv,
                                       const std::vector<Real> & evidence) override;

  virtual void nextSamples(std::vector<Real> & req_inputs,
                           DenseMatrix<Real> & input_matrix,
                           const std::vector<Real> & tv,
                           const unsigned int & parallel_index) override;

  virtual void nextSeeds() override;

private:
  /// IndependentGaussianMH sampler
  const IndependentGaussianMH * const _igmh;

  /// Seed vector input for proposing new samples
  std::vector<Real> & _seed_input;

  /// Outputs corresponding to the seed input vector
  std::vector<Real> _seed_outputs;

  /// Modified transition vector considering the seed input
  std::vector<Real> _tpm_modified;

  /// Store the gathered outputs
  std::vector<Real> _outputs_sto;
};
