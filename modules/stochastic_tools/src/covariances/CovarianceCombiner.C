//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CovarianceCombiner.h"

registerMooseObject("StochasticToolsApp", CovarianceCombiner);

// ---------------------------------------------------------------------------
// validParams
// ---------------------------------------------------------------------------

InputParameters
CovarianceCombiner::validParams()
{
  InputParameters params = CovarianceFunctionBase::validParams();

  params.addClassDescription(
      "Composite covariance function that combines exactly two child kernels. "
      "'Sum' computes K = K1 + K2; 'Product' computes K = K1 .* K2 (Hadamard). "
      "All hyperparameters of both sub-kernels are exposed and tuned simultaneously. "
      "For Product, set noise_variance = 0 in both sub-kernels (see class docs).");

  params.setDocString("covariance_functions",
                      "Exactly two covariance function names to combine, given as "
                      "[kernel_1 kernel_2]. Either child may itself be a CovarianceCombiner "
                      "to build arbitrarily deep kernel trees.");

  MooseEnum operation_options("Sum Product");
  params.addRequiredParam<MooseEnum>(
      "operation",
      operation_options,
      "The combination rule applied to the two child kernels:\n"
      "  'Sum'     -- K = K1 + K2 (additive; each sub-kernel retains its own noise)\n"
      "  'Product' -- K = K1 .* K2 (element-wise; sub-kernel noise_variance should be 0)");

  return params;
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

CovarianceCombiner::CovarianceCombiner(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters),
    _operation(getParam<MooseEnum>("operation") == "Sum" ? Operation::Sum : Operation::Product)
{
  // The base class populates _covariance_functions from the 'covariance_functions' param.
  if (_covariance_functions.size() != 2)
    mooseError("CovarianceCombiner requires exactly two entries in 'covariance_functions', "
               "but ",
               _covariance_functions.size(),
               " were provided.");
}

// ---------------------------------------------------------------------------
// computeCovarianceMatrix
// ---------------------------------------------------------------------------

void
CovarianceCombiner::computeCovarianceMatrix(RealEigenMatrix & K,
                                             const RealEigenMatrix & x,
                                             const RealEigenMatrix & xp,
                                             const bool is_self_covariance) const
{
  switch (_operation)
  {
    // ── Sum ────────────────────────────────────────────────────────────────
    // K = K1 + K2
    //
    // is_self_covariance is forwarded to each sub-kernel so that each kernel's
    // own noise term (sigma_n_i^2 * I) is added to its diagonal, yielding a
    // combined diagonal noise of (sigma_n1^2 + sigma_n2^2) on K.  This is the
    // correct behaviour for an additive GP kernel mixture.
    case Operation::Sum:
    {
      _covariance_functions[0]->computeCovarianceMatrix(K, x, xp, is_self_covariance);

      RealEigenMatrix K2(x.rows(), xp.rows());
      _covariance_functions[1]->computeCovarianceMatrix(K2, x, xp, is_self_covariance);

      K += K2;
      break;
    }

    // ── Product ────────────────────────────────────────────────────────────
    // K = K1 .* K2   (Hadamard / element-wise product)
    //
    // Sub-kernels are always evaluated with is_self_covariance = false because
    // (K1_base + sigma_n1^2 * I) .* (K2_base + sigma_n2^2 * I) does not reduce
    // to a clean additive diagonal noise; see class-level documentation for how
    // to add noise to a product kernel via a surrounding Sum.
    case Operation::Product:
    {
      _covariance_functions[0]->computeCovarianceMatrix(K, x, xp, false);

      RealEigenMatrix K2(x.rows(), xp.rows());
      _covariance_functions[1]->computeCovarianceMatrix(K2, x, xp, false);

      K = K.cwiseProduct(K2);
      break;
    }
  }
}

// ---------------------------------------------------------------------------
// computedKdhyper
// ---------------------------------------------------------------------------

bool
CovarianceCombiner::computedKdhyper(RealEigenMatrix & dKdhp,
                                     const RealEigenMatrix & x,
                                     const std::string & hyper_param_name,
                                     unsigned int ind) const
{
  // ── Routing logic ─────────────────────────────────────────────────────────
  // Every hyperparameter is stored as "object_name:param_name".  Each sub-kernel's
  // computedKdhyper() first checks whether hyper_param_name begins with its own
  // name, so calling both sub-kernels in sequence is safe: the wrong sub-kernel
  // will always return false without modifying dKdhp.  This also supports nested
  // CovarianceCombiner trees transparently via virtual dispatch.

  switch (_operation)
  {
    // ── Sum ────────────────────────────────────────────────────────────────
    // K = K1 + K2
    //
    // Derivative:
    //   dK/dhp = dK1/dhp + dK2/dhp
    //
    // Because K1 and K2 own disjoint prefix-namespaced hyperparameter sets,
    // at most one sub-kernel returns true for any given hyper_param_name.
    // No further manipulation of dKdhp is needed.
    case Operation::Sum:
    {
      if (_covariance_functions[0]->computedKdhyper(dKdhp, x, hyper_param_name, ind))
        return true;
      if (_covariance_functions[1]->computedKdhyper(dKdhp, x, hyper_param_name, ind))
        return true;
      return false;
    }

    // ── Product ────────────────────────────────────────────────────────────
    // K = K1 .* K2
    //
    // Derivative (product rule):
    //   if hp belongs to K1:  dK/dhp1 = (dK1/dhp1) .* K2_base
    //   if hp belongs to K2:  dK/dhp2 = K1_base .* (dK2/dhp2)
    //
    // K1_base / K2_base are re-evaluated here with is_self_covariance = false,
    // consistent with computeCovarianceMatrix for Product.
    //
    // The intermediate dK_sub matrix is sized n x n because computedKdhyper
    // is always invoked with the square training-data Gram matrix (x == x).
    case Operation::Product:
    {
      const unsigned int n = x.rows();
      RealEigenMatrix dK_sub(n, n);

      // ── Try sub-kernel 1 ─────────────────────────────────────────────
      if (_covariance_functions[0]->computedKdhyper(dK_sub, x, hyper_param_name, ind))
      {
        // dK/dhp1 = (dK1/dhp1) .* K2_base
        RealEigenMatrix K2(n, n);
        _covariance_functions[1]->computeCovarianceMatrix(K2, x, x, false);
        dKdhp = dK_sub.cwiseProduct(K2);
        return true;
      }

      // ── Try sub-kernel 2 ─────────────────────────────────────────────
      if (_covariance_functions[1]->computedKdhyper(dK_sub, x, hyper_param_name, ind))
      {
        // dK/dhp2 = K1_base .* (dK2/dhp2)
        RealEigenMatrix K1(n, n);
        _covariance_functions[0]->computeCovarianceMatrix(K1, x, x, false);
        dKdhp = K1.cwiseProduct(dK_sub);
        return true;
      }

      return false;
    }
  }

  // Unreachable; silences compiler warnings on enum switches without default.
  return false;
}
