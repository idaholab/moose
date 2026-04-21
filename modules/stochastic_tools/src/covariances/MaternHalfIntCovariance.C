//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifdef MOOSE_LIBTORCH_ENABLED

#include "MaternHalfIntCovariance.h"
#include <cmath>

registerMooseObject("StochasticToolsApp", MaternHalfIntCovariance);

InputParameters
MaternHalfIntCovariance::validParams()
{
  InputParameters params = CovarianceFunctionBase::validParams();
  params.addClassDescription("Matern half-integer covariance function.");
  params.addRequiredParam<std::vector<Real>>("length_factor",
                                             "Length factors to use for Covariance Kernel");
  params.addRequiredParam<Real>("signal_variance",
                                "Signal Variance ($\\sigma_f^2$) to use for kernel calculation.");
  params.addParam<Real>(
      "noise_variance", 0.0, "Noise Variance ($\\sigma_n^2$) to use for kernel calculation.");
  params.addRequiredParam<unsigned int>(
      "p", "Integer p to use for Matern Half Integer Covariance Kernel");
  return params;
}

MaternHalfIntCovariance::MaternHalfIntCovariance(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters),
    _length_factor(addVectorRealHyperParameter(
        "length_factor", getParam<std::vector<Real>>("length_factor"), true)),
    _sigma_f_squared(
        addRealHyperParameter("signal_variance", getParam<Real>("signal_variance"), true)),
    _sigma_n_squared(
        addRealHyperParameter("noise_variance", getParam<Real>("noise_variance"), true)),
    _p(addRealHyperParameter("p", getParam<unsigned int>("p"), false))
{
}

void
MaternHalfIntCovariance::computeCovarianceMatrix(torch::Tensor & K,
                                                 const torch::Tensor & x,
                                                 const torch::Tensor & xp,
                                                 const bool is_self_covariance) const
{
  if ((unsigned)x.sizes()[1] != _length_factor.numel())
    mooseError("length_factor size does not match dimension of trainer input.");

  maternHalfIntFunction(
      K, x, xp, _length_factor, _sigma_f_squared, _sigma_n_squared, _p, is_self_covariance);
}

void
MaternHalfIntCovariance::maternHalfIntFunction(torch::Tensor & K,
                                               const torch::Tensor & x,
                                               const torch::Tensor & xp,
                                               const torch::Tensor & length_factor,
                                               const torch::Tensor & sigma_f_squared,
                                               const torch::Tensor & sigma_n_squared,
                                               const torch::Tensor & p,
                                               const bool is_self_covariance)
{
  const auto length_factor_accessor = length_factor.accessor<Real, 1>();
  const auto sigma_f_squared_value = sigma_f_squared.item<Real>();
  const auto sigma_n_squared_value = sigma_n_squared.item<Real>();
  const auto p_value = cast_int<unsigned int>(p.item<Real>());
  auto K_accessor = K.accessor<Real, 2>();
  auto x_accessor = x.accessor<Real, 2>();
  auto xp_accessor = xp.accessor<Real, 2>();

  const unsigned int num_samples_x = x.sizes()[0];
  const unsigned int num_samples_xp = xp.sizes()[0];
  const unsigned int num_params_x = x.sizes()[1];

  mooseAssert(num_params_x == xp.sizes()[1],
              "Number of parameters do not match in covariance kernel calculation");

  // This factor is used over and over, don't calculate each time
  Real factor = sqrt(2 * p_value + 1);

  for (unsigned int ii = 0; ii < num_samples_x; ++ii)
  {
    for (unsigned int jj = 0; jj < num_samples_xp; ++jj)
    {
      // Compute distance per parameter, scaled by length factor
      Real r_scaled = 0;
      for (unsigned int kk = 0; kk < num_params_x; ++kk)
        r_scaled += pow((x_accessor[ii][kk] - xp_accessor[jj][kk]) / length_factor_accessor[kk], 2);
      r_scaled = sqrt(r_scaled);
      // tgamma(x+1) == x! when x is a natural number, which should always be the case for
      // MaternHalfInt
      Real summation = 0;
      for (unsigned int tt = 0; tt < p_value + 1; ++tt)
        summation += (tgamma(p_value + tt + 1) / (tgamma(tt + 1) * tgamma(p_value - tt + 1))) *
                     pow(2 * factor * r_scaled, p_value - tt);
      K_accessor[ii][jj] = sigma_f_squared_value * std::exp(-factor * r_scaled) *
                           (tgamma(p_value + 1) / (tgamma(2 * p_value + 1))) * summation;
    }
    if (is_self_covariance)
      K_accessor[ii][ii] += sigma_n_squared_value;
  }
}

bool
MaternHalfIntCovariance::computedKdhyper(torch::Tensor & dKdhp,
                                         const torch::Tensor & x,
                                         const std::string & hyper_param_name,
                                         unsigned int ind) const
{
  if (name().length() + 1 > hyper_param_name.length())
    return false;

  const std::string name_without_prefix = hyper_param_name.substr(name().length() + 1);

  if (name_without_prefix == "noise_variance")
  {
    maternHalfIntFunction(dKdhp,
                          x,
                          x,
                          _length_factor,
                          torch::tensor(0.0, at::kDouble),
                          torch::tensor(1.0, at::kDouble),
                          _p,
                          true);
    return true;
  }

  if (name_without_prefix == "signal_variance")
  {
    maternHalfIntFunction(dKdhp,
                          x,
                          x,
                          _length_factor,
                          torch::tensor(1.0, at::kDouble),
                          torch::tensor(0.0, at::kDouble),
                          _p,
                          false);
    return true;
  }

  if (name_without_prefix == "length_factor")
  {
    computedKdlf(dKdhp, x, _length_factor, _sigma_f_squared, _p, ind);
    return true;
  }

  return false;
}

void
MaternHalfIntCovariance::computedKdlf(torch::Tensor & K,
                                      const torch::Tensor & x,
                                      const torch::Tensor & length_factor,
                                      const torch::Tensor & sigma_f_squared,
                                      const torch::Tensor & p,
                                      const int ind)
{
  const auto length_factor_accessor = length_factor.accessor<Real, 1>();
  const auto sigma_f_squared_value = sigma_f_squared.item<Real>();
  const auto p_value = cast_int<unsigned int>(p.item<Real>());
  auto K_accessor = K.accessor<Real, 2>();
  auto x_accessor = x.accessor<Real, 2>();

  unsigned int num_samples_x = x.sizes()[0];
  unsigned int num_params_x = x.sizes()[1];

  mooseAssert(ind < x.sizes()[1], "Incorrect length factor index");

  // This factor is used over and over, don't calculate each time
  Real factor = sqrt(2 * p_value + 1);

  for (unsigned int ii = 0; ii < num_samples_x; ++ii)
  {
    for (unsigned int jj = 0; jj < num_samples_x; ++jj)
    {
      // Compute distance per parameter, scaled by length factor
      Real r_scaled = 0;
      for (unsigned int kk = 0; kk < num_params_x; ++kk)
        r_scaled += pow((x_accessor[ii][kk] - x_accessor[jj][kk]) / length_factor_accessor[kk], 2);
      r_scaled = sqrt(r_scaled);
      if (r_scaled != 0)
      {
        // product rule to compute dK/dr_scaled
        // u'v
        Real summation = 0;
        for (unsigned int tt = 0; tt < p_value + 1; ++tt)
          summation += (tgamma(p_value + tt + 1) / (tgamma(tt + 1) * tgamma(p_value - tt + 1))) *
                       pow(2 * factor * r_scaled, p_value - tt);
        K_accessor[ii][jj] = -factor * std::exp(-factor * r_scaled) * summation;
        // uv'
        // dont need tt=p, (p-tt) factor ->0. Also avoids unsigned integer subtraction wraparound
        summation = 0;
        for (unsigned int tt = 0; tt < p_value; ++tt)
          summation += (tgamma(p_value + tt + 1) / (tgamma(tt + 1) * tgamma(p_value - tt + 1))) *
                       2 * factor * (p_value - tt) * pow(2 * factor * r_scaled, p_value - tt - 1);
        K_accessor[ii][jj] += std::exp(-factor * r_scaled) * summation;
        // Apply chain rule for dr_scaled/dl_i
        K_accessor[ii][jj] *= -std::pow(x_accessor[ii][ind] - x_accessor[jj][ind], 2) /
                              (std::pow(length_factor_accessor[ind], 3) * r_scaled) *
                              sigma_f_squared_value *
                              (tgamma(p_value + 1) / (tgamma(2 * p_value + 1)));
      }
      else // avoid div by 0. 0/0=0 scenario.
        K_accessor[ii][jj] = 0;
    }
  }
}

#endif
