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

#include "PMCMCDecision.h"
#include "GaussianProcess.h"
#include "SurrogateModelInterface.h"
#include "LibtorchANNSurrogate.h"

/**
 * A class for performing Affine Invariant Ensemble MCMC with differential sampler
 */
class AffineInvariantDifferentialDecisionwithGPry : public PMCMCDecision,
                                                    public SurrogateModelInterface
{
public:
  static InputParameters validParams();

  AffineInvariantDifferentialDecisionwithGPry(const InputParameters & parameters);

protected:
  virtual void execute() override;

  virtual void computeEvidence(std::vector<Real> & evidence,
                               const DenseMatrix<Real> & input_matrix) override;

  virtual void computeTransitionVector(std::vector<Real> & tv,
                                       const std::vector<Real> & evidence) override;

  virtual void nextSamples(std::vector<Real> & req_inputs,
                           DenseMatrix<Real> & input_matrix,
                           const std::vector<Real> & tv,
                           const unsigned int & parallel_index) override;

  /// The GP evaluator object
  const SurrogateModel & _gp_eval;

  /// The libtorch classifer neural network
  const LibtorchANNSurrogate & _nn_eval;

  /// Storage for new proposed samples
  const std::vector<std::vector<Real>> & _new_samples;

private:
};

#endif
