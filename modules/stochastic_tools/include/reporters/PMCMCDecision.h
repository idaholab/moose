//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PMCMCDecisionBase.h"
#include "LikelihoodFunctionBase.h"
#include "LikelihoodInterface.h"

/**
 * PMCMCDecision will help making sample accept/reject decisions in MCMC
 * schemes (for e.g., when performing Bayesian inference).
 */
class PMCMCDecision : public PMCMCDecisionBase, public LikelihoodInterface
{
public:
  static InputParameters validParams();
  PMCMCDecision(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  virtual void computeEvidence(std::vector<Real> & evidence,
                               const DenseMatrix<Real> & input_matrix) override;

  virtual void computeTransitionVector(std::vector<Real> & tv,
                                       const std::vector<Real> & evidence) override;

  virtual void nextSamples(std::vector<Real> & req_inputs,
                           DenseMatrix<Real> & input_matrix,
                           const std::vector<Real> & tv,
                           const unsigned int & parallel_index) override;

  /// Model output value from SubApp
  const std::vector<Real> & _output_value;

  /// Transfer the right outputs to the file
  std::vector<Real> & _outputs_required;

  /// Storage for the likelihood objects to be utilized
  std::vector<const LikelihoodFunctionBase *> _likelihoods;

  /// Storage for previous outputs
  std::vector<Real> _outputs_prev;
};
