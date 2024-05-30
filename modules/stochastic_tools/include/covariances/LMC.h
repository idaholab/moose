//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CovarianceFunctionBase.h"

/**
 * Covariance function for multi-output Gaussian Processes based on
 * the linear model of coregionalization (LMC)
 */
class LMC : public CovarianceFunctionBase
{
public:
  static InputParameters validParams();
  LMC(const InputParameters & parameters);

  void computeCovarianceMatrix(RealEigenMatrix & K,
                               const RealEigenMatrix & x,
                               const RealEigenMatrix & xp,
                               const bool is_self_covariance) const override;

  bool computedKdhyper(RealEigenMatrix & dKdhp,
                       const RealEigenMatrix & x,
                       const std::string & hyper_param_name,
                       unsigned int ind) const override;

protected:
  /**
   * Computes the covariance matrix for the outputs (using the latent coefficients)
   * We use a $B = \sum_i a_i a_i^T + diag(lambda_i)$ expansion here where
   * $a_i$ and $lambda_i$ are vectors
   * @param Bmat The matrix which should be populated by the covariance values
   * @param exp_i The expansion index in the latent space
   */
  void computeBMatrix(RealEigenMatrix & Bmat, const unsigned int exp_i) const;

  /**
   * Computes the gradient of $B$ with respect to the entries in $a_i$ in the
   * following expression: $B = \sum_i a_i a_i^T + diag(lambda_i)$
   * @param grad The gradient matrix that should be populated
   * @param exp_i The index of the expansion of B
   * @param index The index within the vector $a_i$
   */
  void computeAGradient(RealEigenMatrix & grad,
                        const unsigned int exp_i,
                        const unsigned int index) const;

  /**
   * Computes the gradient of $B$ with respect to the entries in $lambda_i$ in the
   * following expression: $B = \sum_i a_i a_i^T + diag(lambda_i)$
   * @param grad The gradient matrix that should be populated
   * @param exp_i The index of the expansion of B
   * @param index The index within the vector $lambda_i$
   */
  void computeLambdaGradient(RealEigenMatrix & grad,
                             const unsigned int exp_i,
                             const unsigned int index) const;

  /// The number of expansion terms in the output ovariance matrix
  const unsigned int _num_expansion_terms;

private:
  /// The vectors in the $B = \sum_i a_i a_i^T + diag(lambda_i)$
  /// expansion
  std::vector<const std::vector<Real> *> _a_coeffs;
  std::vector<const std::vector<Real> *> _lambdas;
};
