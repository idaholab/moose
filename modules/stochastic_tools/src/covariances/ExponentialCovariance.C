//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExponentialCovariance.h"
#include <cmath>

registerMooseObject("StochasticToolsApp", ExponentialCovariance);

InputParameters
ExponentialCovariance::validParams()
{
  InputParameters params = CovarianceFunctionBase::validParams();
  params.addClassDescription("Exponential covariance function.");
  params.addRequiredParam<std::vector<Real>>("length_factor",
                                             "Length factors to use for Covariance Kernel");
  params.addRequiredParam<Real>("signal_variance",
                                "Signal Variance ($\\sigma_f^2$) to use for kernel calculation.");
  params.addParam<Real>(
      "noise_variance", 0.0, "Noise Variance ($\\sigma_n^2$) to use for kernel calculation.");
  params.addRequiredParam<Real>("gamma", "Gamma to use for Exponential Covariance Kernel");
  return params;
}

ExponentialCovariance::ExponentialCovariance(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters),
    _length_factor(addVectorRealHyperParameter(
        "length_factor", getParam<std::vector<Real>>("length_factor"), true)),
    _sigma_f_squared(
        addRealHyperParameter("signal_variance", getParam<Real>("signal_variance"), true)),
    _sigma_n_squared(
        addRealHyperParameter("noise_variance", getParam<Real>("noise_variance"), true)),
    _gamma(addRealHyperParameter("gamma", getParam<Real>("gamma"), false))
{
}

void
ExponentialCovariance::computeCovarianceMatrix(torch::Tensor & K,
                                               const torch::Tensor & x,
                                               const torch::Tensor & xp,
                                               const bool is_self_covariance) const
{
  if ((unsigned)x.sizes()[1] != _length_factor.size())
    mooseError("length_factor size does not match dimension of trainer input.");

  ExponentialFunction(
      K, x, xp, _length_factor, _sigma_f_squared, _sigma_n_squared, _gamma, is_self_covariance);
}

void
ExponentialCovariance::ExponentialFunction(torch::Tensor & K,
                                           const torch::Tensor & x,
                                           const torch::Tensor & xp,
                                           const std::vector<Real> & length_factor,
                                           const Real sigma_f_squared,
                                           const Real sigma_n_squared,
                                           const Real gamma,
                                           const bool is_self_covariance)
{
  unsigned int num_samples_x = x.sizes()[0];
  unsigned int num_samples_xp = xp.sizes()[0];
  unsigned int num_params_x = x.sizes()[1];

  mooseAssert(num_params_x == xp.sizes()[1],
              "Number of parameters do not match in covariance kernel calculation");

  auto K_accessor = K.accessor<Real, 2>();
  auto x_accessor = x.accessor<Real, 2>();
  auto xp_accessor = xp.accessor<Real, 2>();
  for (unsigned int ii = 0; ii < num_samples_x; ++ii)
  {
    for (unsigned int jj = 0; jj < num_samples_xp; ++jj)
    {
      // Compute distance per parameter, scaled by length factor
      Real r_scaled = 0;
      for (unsigned int kk = 0; kk < num_params_x; ++kk)
        r_scaled += pow((x_accessor[ii][kk] - xp_accessor[jj][kk]) / length_factor[kk], 2);
      r_scaled = sqrt(r_scaled);
      K_accessor[ii][jj] = sigma_f_squared * std::exp(-pow(r_scaled, gamma));
    }
    if (is_self_covariance)
      K_accessor[ii][ii] += sigma_n_squared;
  }
}

bool
ExponentialCovariance::computedKdhyper(torch::Tensor & dKdhp,
                                       const torch::Tensor & x,
                                       const std::string & hyper_param_name,
                                       unsigned int ind) const
{
  if (name().length() + 1 > hyper_param_name.length())
    return false;

  const std::string name_without_prefix = hyper_param_name.substr(name().length() + 1);

  if (name_without_prefix == "noise_variance")
  {
    ExponentialFunction(dKdhp, x, x, _length_factor, 0, 1, _gamma, true);
    return true;
  }

  if (name_without_prefix == "signal_variance")
  {
    ExponentialFunction(dKdhp, x, x, _length_factor, 1, 0, _gamma, false);
    return true;
  }

  if (name_without_prefix == "length_factor")
  {
    computedKdlf(dKdhp, x, _length_factor, _sigma_f_squared, _gamma, ind);
    return true;
  }

  return false;
}

void
ExponentialCovariance::computedKdlf(torch::Tensor & K,
                                    const torch::Tensor & x,
                                    const std::vector<Real> & length_factor,
                                    const Real sigma_f_squared,
                                    const Real gamma,
                                    const int ind)
{
  auto K_accessor = K.accessor<Real, 2>();
  auto x_accessor = x.accessor<Real, 2>();

  unsigned int num_samples_x = x.sizes()[0];
  unsigned int num_params_x = x.sizes()[1];

  mooseAssert(ind < x.sizes()[1], "Incorrect length factor index");

  for (unsigned int ii = 0; ii < num_samples_x; ++ii)
  {
    for (unsigned int jj = 0; jj < num_samples_x; ++jj)
    {
      // Compute distance per parameter, scaled by length factor
      Real r_scaled = 0;
      for (unsigned int kk = 0; kk < num_params_x; ++kk)
        r_scaled += pow((x_accessor[ii][kk] - x_accessor[jj][kk]) / length_factor[kk], 2);
      r_scaled = sqrt(r_scaled);
      if (r_scaled != 0)
      {
        K_accessor[ii][jj] = gamma * std::pow(r_scaled, gamma - 2) * sigma_f_squared *
                             std::exp(-pow(r_scaled, gamma));
        K_accessor[ii][jj] = std::pow(x_accessor[ii][ind] - x_accessor[jj][ind], 2) /
                             std::pow(length_factor[ind], 3) * K_accessor[ii][jj];
      }
      else // avoid div by 0. 0/0=0 scenario.
        K_accessor[ii][jj] = 0;
    }
  }
}
