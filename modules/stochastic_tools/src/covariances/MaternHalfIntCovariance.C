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
#include "LibtorchUtils.h"
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
  const auto p_value = cast_int<unsigned int>(LibtorchUtils::toCPUContiguous(p).item<Real>());
  mooseAssert(x.sizes()[1] == xp.sizes()[1],
              "Number of parameters do not match in covariance kernel calculation");

  const auto l_factor = length_factor.unsqueeze(0);
  const auto scaled_distance = torch::cdist(torch::div(x, l_factor), torch::div(xp, l_factor), 2.0);
  const Real factor = std::sqrt(2 * p_value + 1);
  const Real normalization = std::tgamma(p_value + 1) / std::tgamma(2 * p_value + 1);

  auto summation = torch::zeros_like(scaled_distance);
  for (const auto tt : make_range(p_value + 1))
  {
    const Real coefficient =
        std::tgamma(p_value + tt + 1) / (std::tgamma(tt + 1) * std::tgamma(p_value - tt + 1));
    summation =
        summation + coefficient * torch::pow(2.0 * factor * scaled_distance, Real(p_value - tt));
  }

  K = sigma_f_squared * torch::exp(-factor * scaled_distance) * normalization * summation;
  if (is_self_covariance)
    K = K + sigma_n_squared * torch::eye(K.size(0), K.options());
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
    const auto options = x.options().dtype(at::kDouble);
    maternHalfIntFunction(dKdhp,
                          x,
                          x,
                          _length_factor,
                          torch::tensor(0.0, options),
                          torch::tensor(1.0, options),
                          _p,
                          true);
    return true;
  }

  if (name_without_prefix == "signal_variance")
  {
    const auto options = x.options().dtype(at::kDouble);
    maternHalfIntFunction(dKdhp,
                          x,
                          x,
                          _length_factor,
                          torch::tensor(1.0, options),
                          torch::tensor(0.0, options),
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
  const auto p_value = cast_int<unsigned int>(LibtorchUtils::toCPUContiguous(p).item<Real>());

  mooseAssert(ind < x.sizes()[1], "Incorrect length factor index");

  const auto l_factor = length_factor.unsqueeze(0);
  const auto scaled_distance = torch::cdist(torch::div(x, l_factor), torch::div(x, l_factor), 2.0);
  const auto nonzero_distance = scaled_distance > 0;
  const auto safe_scaled_distance =
      torch::where(nonzero_distance, scaled_distance, torch::ones_like(scaled_distance));
  const Real factor = std::sqrt(2 * p_value + 1);
  const Real normalization = std::tgamma(p_value + 1) / std::tgamma(2 * p_value + 1);

  auto summation = torch::zeros_like(safe_scaled_distance);
  for (const auto tt : make_range(p_value + 1))
  {
    const Real coefficient =
        std::tgamma(p_value + tt + 1) / (std::tgamma(tt + 1) * std::tgamma(p_value - tt + 1));
    summation = summation +
                coefficient * torch::pow(2.0 * factor * safe_scaled_distance, Real(p_value - tt));
  }

  auto summation_derivative = torch::zeros_like(safe_scaled_distance);
  for (const auto tt : make_range(p_value))
  {
    const Real coefficient =
        std::tgamma(p_value + tt + 1) / (std::tgamma(tt + 1) * std::tgamma(p_value - tt + 1));
    summation_derivative =
        summation_derivative +
        coefficient * 2.0 * factor * (p_value - tt) *
            torch::pow(2.0 * factor * safe_scaled_distance, Real(p_value - tt - 1));
  }

  const auto coordinate = x.select(1, ind);
  const auto coordinate_distance_squared =
      torch::pow(coordinate.unsqueeze(1) - coordinate.unsqueeze(0), 2);
  const auto length_factor_ind = length_factor.select(0, ind);
  const auto dr_dlength_factor =
      -coordinate_distance_squared / (torch::pow(length_factor_ind, 3) * safe_scaled_distance);
  const auto dK_dlength_factor = sigma_f_squared * normalization *
                                 torch::exp(-factor * safe_scaled_distance) *
                                 (summation_derivative - factor * summation) * dr_dlength_factor;

  K = torch::where(nonzero_distance, dK_dlength_factor, torch::zeros_like(dK_dlength_factor));
}

#endif
