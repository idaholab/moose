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
#include "GaussianProcess.h"
#include "SurrogateModelInterface.h"
#include "LibtorchANNSurrogate.h"
#include "ActiveLearningGaussianProcess.h"
#include "SurrogateModel.h"

/**
 * A class for performing Affine Invariant Ensemble MCMC with differential sampler
 */
class AIDESGPryLearner : public PMCMCDecision,
                         public SurrogateModelInterface
{
public:
  static InputParameters validParams();

  AIDESGPryLearner(const InputParameters & parameters);

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

  void computeGPOutput(std::vector<Real> & eval_outputs, const DenseMatrix<Real> & eval_inputs);

  /// The active learning GP trainer that permits re-training
  const ActiveLearningGaussianProcess & _al_gp;

  /// The GP evaluator object
  const SurrogateModel & _gp_eval;

  /// Storage for new proposed samples
  const std::vector<std::vector<Real>> & _new_samples;

//   std::vector<Real> & _acquisition_function;

  Real & _convergence_value;

private:
  void setupNNGPData(const DenseMatrix<Real> & data_in);

  /// SubApp inputs for GP model
  std::vector<std::vector<Real>> _gp_inputs;

  /// SubApp outputs for GP model
  std::vector<Real> _gp_outputs;

  std::vector<Real> _eval_outputs_current;
};
