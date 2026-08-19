//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearCovariance.h"

registerMooseObject("StochasticToolsApp", LinearCovariance);

InputParameters
LinearCovariance::validParams()
{
  InputParameters params = CovarianceFunctionBase::validParams();
  params.addClassDescription(
      "Linear (non-stationary) covariance function for Gaussian Processes. "
      "Evaluates K(x, x') = sigma_b^2 + sigma_v^2 * (x - c)^T (x' - c), "
      "with an optional noise term sigma_n^2 added to the diagonal.");
  params.addRequiredParam<std::vector<Real>>(
      "c",
      "Offset per input dimension ($\\mathbf{c}$). Sets the pivot point of the linear kernel; "
      "use all zeros to obtain a plain dot-product kernel.");
  params.addRequiredParam<Real>(
      "signal_variance",
      "Signal variance ($\\sigma_v^2$) that scales the inner-product term.");
  params.addRequiredParam<Real>(
      "bias_variance",
      "Bias variance ($\\sigma_b^2$) added as a constant offset to all covariance entries.");
  params.addParam<Real>(
      "noise_variance",
      0.0,
      "Noise variance ($\\sigma_n^2$) added to the diagonal for self-covariance.");
  return params;
}

LinearCovariance::LinearCovariance(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters),
    _c(addVectorRealHyperParameter("c", getParam<std::vector<Real>>("c"), true)),
    _sigma_v_squared(
        addRealHyperParameter("signal_variance", getParam<Real>("signal_variance"), true)),
    _sigma_b_squared(
        addRealHyperParameter("bias_variance", getParam<Real>("bias_variance"), true)),
    _sigma_n_squared(
        addRealHyperParameter("noise_variance", getParam<Real>("noise_variance"), true))
{
}

void
LinearCovariance::computeCovarianceMatrix(RealEigenMatrix & K,
                                          const RealEigenMatrix & x,
                                          const RealEigenMatrix & xp,
                                          const bool is_self_covariance) const
{
  if ((unsigned int)x.cols() != _c.size())
    mooseError("c size does not match dimension of trainer input.");

  LinearFunction(
      K, x, xp, _c, _sigma_v_squared, _sigma_b_squared, _sigma_n_squared, is_self_covariance);
}

void
LinearCovariance::LinearFunction(RealEigenMatrix & K,
                                  const RealEigenMatrix & x,
                                  const RealEigenMatrix & xp,
                                  const std::vector<Real> & c,
                                  const Real sigma_v_squared,
                                  const Real sigma_b_squared,
                                  const Real sigma_n_squared,
                                  const bool is_self_covariance)
{
  const unsigned int num_samples_x = x.rows();
  const unsigned int num_samples_xp = xp.rows();
  const unsigned int num_params_x = x.cols();

  mooseAssert(num_params_x == (unsigned int)xp.cols(),
              "Number of parameters do not match in covariance kernel calculation");

  for (unsigned int ii = 0; ii < num_samples_x; ++ii)
  {
    for (unsigned int jj = 0; jj < num_samples_xp; ++jj)
    {
      // Compute sigma_b^2 + sigma_v^2 * (x_i - c)^T (x'_j - c)
      Real inner_product = 0.0;
      for (unsigned int kk = 0; kk < num_params_x; ++kk)
        inner_product += (x(ii, kk) - c[kk]) * (xp(jj, kk) - c[kk]);
      K(ii, jj) = sigma_b_squared + sigma_v_squared * inner_product;
    }
    // Add noise variance to the diagonal for self-covariance
    if (is_self_covariance)
      K(ii, ii) += sigma_n_squared;
  }
}

bool
LinearCovariance::computedKdhyper(RealEigenMatrix & dKdhp,
                                   const RealEigenMatrix & x,
                                   const std::string & hyper_param_name,
                                   unsigned int ind) const
{
  // Hyperparameter names are stored with the object name as a prefix ("objname:param"),
  // so bail out early if the string is too short to contain any prefix.
  if (name().length() + 1 > hyper_param_name.length())
    return false;

  const std::string name_without_prefix = hyper_param_name.substr(name().length() + 1);

  // dK / d(sigma_n^2) = I  (identity, added only on the diagonal)
  if (name_without_prefix == "noise_variance")
  {
    LinearFunction(dKdhp, x, x, _c, 0.0, 0.0, 1.0, true);
    return true;
  }

  // dK(x_i, x_j) / d(sigma_v^2) = sum_k (x_ik - c_k)(x_jk - c_k)
  if (name_without_prefix == "signal_variance")
  {
    LinearFunction(dKdhp, x, x, _c, 1.0, 0.0, 0.0, false);
    return true;
  }

  // dK(x_i, x_j) / d(sigma_b^2) = 1  for all i, j
  if (name_without_prefix == "bias_variance")
  {
    LinearFunction(dKdhp, x, x, _c, 0.0, 1.0, 0.0, false);
    return true;
  }

  // dK(x_i, x_j) / d(c_ind) = -sigma_v^2 * [(x_i,ind - c_ind) + (x_j,ind - c_ind)]
  if (name_without_prefix == "c")
  {
    computedKdc(dKdhp, x, _c, _sigma_v_squared, ind);
    return true;
  }

  return false;
}

void
LinearCovariance::computedKdc(RealEigenMatrix & K,
                               const RealEigenMatrix & x,
                               const std::vector<Real> & c,
                               const Real sigma_v_squared,
                               const int ind)
{
  const unsigned int num_samples_x = x.rows();

  mooseAssert(ind < (int)x.cols(), "Incorrect offset parameter (c) index");

  // The kernel cross-term involving dimension 'ind' is:
  //   f(c_ind) = (x_i,ind - c_ind)(x_j,ind - c_ind)
  //
  // Applying the product rule:
  //   df/dc_ind = -(x_j,ind - c_ind) - (x_i,ind - c_ind)
  //             = -[(x_i,ind - c_ind) + (x_j,ind - c_ind)]
  //
  // All other dimensions are independent of c_ind, so they vanish.
  for (unsigned int ii = 0; ii < num_samples_x; ++ii)
    for (unsigned int jj = 0; jj < num_samples_x; ++jj)
      K(ii, jj) =
          -sigma_v_squared * ((x(ii, ind) - c[ind]) + (x(jj, ind) - c[ind]));
}
