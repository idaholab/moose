//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifdef MOOSE_LIBTORCH_ENABLED

#include "SquaredExponentialCovariance.h"
#include <cmath>

registerMooseObject("StochasticToolsApp", SquaredExponentialCovariance);

InputParameters
SquaredExponentialCovariance::validParams()
{
  InputParameters params = CovarianceFunctionBase::validParams();
  params.addClassDescription("Squared Exponential covariance function.");
  params.addRequiredParam<std::vector<Real>>("length_factor",
                                             "Length factors to use for Covariance Kernel");
  params.addRequiredParam<Real>("signal_variance",
                                "Signal Variance ($\\sigma_f^2$) to use for kernel calculation.");
  params.addParam<Real>(
      "noise_variance", 0.0, "Noise Variance ($\\sigma_n^2$) to use for kernel calculation.");
  return params;
}

SquaredExponentialCovariance::SquaredExponentialCovariance(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters),
    _length_factor(addVectorRealHyperParameter(
        "length_factor", getParam<std::vector<Real>>("length_factor"), true)),
    _sigma_f_squared(
        addRealHyperParameter("signal_variance", getParam<Real>("signal_variance"), true)),
    _sigma_n_squared(
        addRealHyperParameter("noise_variance", getParam<Real>("noise_variance"), true))
{
}

void
SquaredExponentialCovariance::computeCovarianceMatrix(torch::Tensor & K,
                                                      const torch::Tensor & x,
                                                      const torch::Tensor & xp,
                                                      const bool is_self_covariance) const
{
  if ((unsigned)x.sizes()[1] != _length_factor.numel())
    mooseError("length_factor size does not match dimension of trainer input.");

  SquaredExponentialFunction(
      K, x, xp, _length_factor, _sigma_f_squared, _sigma_n_squared, is_self_covariance);
}

void
SquaredExponentialCovariance::SquaredExponentialFunction(torch::Tensor & K,
                                                         const torch::Tensor & x,
                                                         const torch::Tensor & xp,
                                                         const torch::Tensor & length_factor,
                                                         const torch::Tensor & sigma_f_squared,
                                                         const torch::Tensor & sigma_n_squared,
                                                         const bool is_self_covariance)
{
  mooseAssert(x.sizes()[1] == xp.sizes()[1],
              "Number of parameters do not match in covariance kernel calculation");

  const auto l_factor = length_factor.unsqueeze(0);
  K = torch::cdist(torch::div(x, l_factor), torch::div(xp, l_factor), 2.0);
  K.pow_(2).mul_(-0.5).exp_().mul_(sigma_f_squared);
  if (is_self_covariance)
    K.diagonal().add_(sigma_n_squared);
}

bool
SquaredExponentialCovariance::computedKdhyper(torch::Tensor & dKdhp,
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
    SquaredExponentialFunction(dKdhp,
                               x,
                               x,
                               _length_factor,
                               torch::tensor(0.0, options),
                               torch::tensor(1.0, options),
                               true);
    return true;
  }

  if (name_without_prefix == "signal_variance")
  {
    const auto options = x.options().dtype(at::kDouble);
    SquaredExponentialFunction(dKdhp,
                               x,
                               x,
                               _length_factor,
                               torch::tensor(1.0, options),
                               torch::tensor(0.0, options),
                               false);
    return true;
  }

  if (name_without_prefix == "length_factor")
  {
    computedKdlf(dKdhp, x, _length_factor, _sigma_f_squared, ind);
    return true;
  }

  return false;
}

void
SquaredExponentialCovariance::computedKdlf(torch::Tensor & K,
                                           const torch::Tensor & x,
                                           const torch::Tensor & length_factor,
                                           const torch::Tensor & sigma_f_squared,
                                           const int ind)
{
  mooseAssert(ind < x.sizes()[1], "Incorrect length factor index");

  const auto l_factor = length_factor.unsqueeze(0);
  K = torch::cdist(torch::div(x, l_factor), torch::div(x, l_factor), 2.0);
  K.pow_(2).mul_(-0.5).exp_().mul_(sigma_f_squared);
  const auto coordinate = x.select(1, ind);
  const auto coordinate_distance_squared =
      torch::pow(coordinate.unsqueeze(1) - coordinate.unsqueeze(0), 2);
  const auto length_factor_ind = length_factor.select(0, ind);

  K.mul_(coordinate_distance_squared).div_(torch::pow(length_factor_ind, 3));
}

#endif
