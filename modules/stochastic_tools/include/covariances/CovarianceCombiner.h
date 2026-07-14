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
 * Composite covariance function that combines exactly two child kernels via
 * Sum or element-wise (Hadamard) Product.
 *
 *   Sum:     K(x, x') = K1(x, x') + K2(x, x')
 *   Product: K(x, x') = K1(x, x') * K2(x, x')   [element-wise]
 *
 * ── Hyperparameter management ─────────────────────────────────────────────
 * No new tunable hyperparameters are introduced by this class. All parameters
 * of K1 and K2 are surfaced through the base-class infrastructure:
 *   • buildHyperParamMap / loadHyperParamMap  – recursively collect/restore both
 *     sub-kernel parameter maps, so the GP optimizer sees one flat set.
 *   • isTunable / getTuningData              – delegate to sub-kernels first.
 *
 * Parameters are stored with their owning object's name as a prefix
 * ("kernel_name:param_name"), guaranteeing uniqueness even when both
 * sub-kernels share a parameter name (e.g. both have "signal_variance").
 *
 * ── Gradient derivations ──────────────────────────────────────────────────
 * Sum:
 *   dK/dhp  = dK1/dhp + dK2/dhp
 *   Because K1 and K2 own disjoint prefix-namespaced sets, only one term
 *   is non-zero for any given hyperparameter name.
 *
 * Product:
 *   dK/dhp1 = (dK1/dhp1) .* K2_base        (product rule, no noise in K2)
 *   dK/dhp2 = K1_base .* (dK2/dhp2)        (product rule, no noise in K1)
 *
 * ── Note on noise in Product kernels ──────────────────────────────────────
 * K = K1 .* K2 does not yield a clean additive diagonal noise structure.
 * Sub-kernels used with 'Product' should therefore have noise_variance = 0.
 * If observation noise is required, add a Sum of this Product kernel with a
 * LinearCovariance whose signal_variance and bias_variance are both zero
 * (functioning as a pure noise kernel).
 *
 * ── Recursive composition ─────────────────────────────────────────────────
 * Either child kernel may itself be a CovarianceCombiner, enabling arbitrary
 * kernel trees. For example:
 *
 *   top  : Sum   (rbf_x_matern,  linear)
 *   rbf_x_matern : Product (rbf, matern)
 *
 * Each gradient call propagates down the tree through virtual dispatch.
 */
class CovarianceCombiner : public CovarianceFunctionBase
{
public:
  static InputParameters validParams();
  CovarianceCombiner(const InputParameters & parameters);

  /// Computes the combined covariance matrix from the two child kernels
  void computeCovarianceMatrix(RealEigenMatrix & K,
                               const RealEigenMatrix & x,
                               const RealEigenMatrix & xp,
                               const bool is_self_covariance) const override;

  /// Routes dK/dhp to the owning sub-kernel and applies the chain rule for
  /// Product. Returns false if no sub-kernel owns the hyperparameter.
  bool computedKdhyper(RealEigenMatrix & dKdhp,
                       const RealEigenMatrix & x,
                       const std::string & hyper_param_name,
                       unsigned int ind) const override;

private:
  /// The supported combination modes
  enum class Operation
  {
    Sum,    ///< K = K1 + K2
    Product ///< K = K1 .* K2   (element-wise / Hadamard)
  };

  /// Resolved operation from the 'operation' input parameter
  const Operation _operation;
};
