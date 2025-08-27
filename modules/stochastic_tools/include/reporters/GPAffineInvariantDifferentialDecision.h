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
#include "AffineInvariantDES.h"
#include "GaussianProcess.h"
#include "SurrogateModel.h"
#include "SurrogateModelInterface.h"
#include "GaussianProcessSurrogate.h"

/**
 * A class to perform decision making for Affine Invariant differential MCMC using a GP
 */
class GPAffineInvariantDifferentialDecision : public PMCMCDecision, public SurrogateModelInterface
{
public:
  static InputParameters validParams();

  GPAffineInvariantDifferentialDecision(const InputParameters & parameters);
  virtual void initialize() override;

protected:
  virtual void computeEvidence(std::vector<Real> & evidence,
                               const DenseMatrix<Real> & input_matrix) override;

  virtual void computeTransitionVector(std::vector<Real> & tv,
                                       const std::vector<Real> & evidence) override;

  virtual void nextSamples(std::vector<Real> & req_inputs,
                           DenseMatrix<Real> & input_matrix,
                           const std::vector<Real> & tv,
                           const unsigned int & parallel_index) override;

private:
  /**
   * Return the corrected GP output after using the right variance value
   * @param GPoutput The current value of the GP output using incorrect variance
   * @param input_matrix The required value of the variance
   */
  Real correctGP(const Real & GPoutput, const Real & trueVariance);

  /// Bool to correct the GP predicted output to adjust for the right variance
  const bool & _correct_GP_output;

  /// The old incorrect variance used during active learning
  const Real & _incorrect_variance;

  /// Affine differential sampler
  const AffineInvariantDES * const _aides;

  /// The GP evaluator object
  const SurrogateModel * _gp_eval;

  /// The GP estimated value of the log-likelihood
  std::vector<Real> & _estimated_loglikelihood;
};
