//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CovarianceFunctionBase.h"

/**
 * Non-stationary linear covariance function for Gaussian Processes.
 *
 * Implements: K(x, x') = sigma_b^2 + sigma_v^2 * sum_k (x_k - c_k)(x'_k - c_k)
 *
 * where:
 *   sigma_v^2  -- signal variance, scales the inner product term
 *   sigma_b^2  -- bias variance, constant offset added to every entry
 *   sigma_n^2  -- noise variance, added to the diagonal (self-covariance only)
 *   c          -- per-dimension offset vector (pivot point of the kernel)
 */
class LinearCovariance : public CovarianceFunctionBase
{
public:
  static InputParameters validParams();
  LinearCovariance(const InputParameters & parameters);

  void computeCovarianceMatrix(RealEigenMatrix & K,
                               const RealEigenMatrix & x,
                               const RealEigenMatrix & xp,
                               const bool is_self_covariance) const override;

  /// Evaluates K(ii,jj) = sigma_b^2 + sigma_v^2 * sum_k (x_ik - c_k)(x'_jk - c_k)
  /// and adds sigma_n^2 to the diagonal when is_self_covariance is true.
  static void LinearFunction(RealEigenMatrix & K,
                             const RealEigenMatrix & x,
                             const RealEigenMatrix & xp,
                             const std::vector<Real> & c,
                             const Real sigma_v_squared,
                             const Real sigma_b_squared,
                             const Real sigma_n_squared,
                             const bool is_self_covariance);

  /// Redirect dK/dhp for hyperparameter "hp".
  /// Returns false if the parameter has not been found in this covariance object.
  bool computedKdhyper(RealEigenMatrix & dKdhp,
                       const RealEigenMatrix & x,
                       const std::string & hyper_param_name,
                       unsigned int ind) const override;

  /// Computes dK/dc_ind -- derivative with respect to offset component 'ind'.
  /// dK(x_i, x_j)/dc_ind = -sigma_v^2 * [(x_i,ind - c_ind) + (x_j,ind - c_ind)]
  static void computedKdc(RealEigenMatrix & K,
                          const RealEigenMatrix & x,
                          const std::vector<Real> & c,
                          const Real sigma_v_squared,
                          const int ind);

private:
  /// Offset (c) per input dimension
  const std::vector<Real> & _c;

  /// Signal variance (sigma_v^2): scales the inner product (x-c)^T(x'-c)
  const Real & _sigma_v_squared;

  /// Bias variance (sigma_b^2): constant offset added to all covariance entries
  const Real & _sigma_b_squared;

  /// Noise variance (sigma_n^2): added to the diagonal for self-covariance
  const Real & _sigma_n_squared;
};
