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

void
SquaredExponentialCovariance::computeCovarianceFD(torch::Tensor & K_fd,
                                                  const torch::Tensor & x,
                                                  const torch::Tensor & xd,
                                                  unsigned int dim) const
{
  // K_fd[i,j] = Cov[f(x_i), df(xd_j)/dx'_{dim}]
  //           = dK(x_i, xd_j)/dx'_{dim}
  //           = K(x_i,xd_j) * (x_{i,dim} - xd_{j,dim}) / ell_dim^2
  mooseAssert(dim < (unsigned)x.sizes()[1], "dim out of range for computeCovarianceFD");
  mooseAssert(x.sizes()[1] == xd.sizes()[1], "x and xd must have same number of columns");

  const auto options = x.options().dtype(at::kDouble);
  const auto lf = _length_factor.to(options);
  const auto l_factor = lf.unsqueeze(0);

  torch::Tensor base = torch::cdist(torch::div(x, l_factor), torch::div(xd, l_factor), 2.0);
  base.pow_(2).mul_(-0.5).exp_().mul_(_sigma_f_squared.to(options));
  const auto diff = x.select(1, dim).unsqueeze(1) - xd.select(1, dim).unsqueeze(0);
  K_fd = base.mul(diff).div(lf.select(0, dim).pow(2));
}

void
SquaredExponentialCovariance::computeCovarianceDf(torch::Tensor & K_df,
                                                  const torch::Tensor & xd,
                                                  const torch::Tensor & xp,
                                                  unsigned int dim) const
{
  // K_df[i,j] = Cov[df(xd_i)/dx_{dim}, f(xp_j)]
  //           = dK(xd_i, xp_j)/dx_{d,dim}
  //           = K(xd_i,xp_j) * (xp_{j,dim} - xd_{i,dim}) / ell_dim^2
  // Note: K_df = K_fd^T (transpose of computeCovarianceFD with swapped args)
  torch::Tensor K_fd;
  computeCovarianceFD(K_fd, xp, xd, dim);
  K_df = K_fd.transpose(0, 1).contiguous();
}

void
SquaredExponentialCovariance::computeCovarianceDD(torch::Tensor & K_dd,
                                                  const torch::Tensor & xd,
                                                  const torch::Tensor & xdp,
                                                  unsigned int dim_i,
                                                  unsigned int dim_j) const
{
  // K_dd[i,j] = d^2 K(xd_i, xdp_j) / (dx_{dim_i} dx'_{dim_j})
  //
  // For SE kernel:
  //   if dim_i == dim_j (call the dimension k):
  //     = K(xd_i,xdp_j) * [1/ell_k^2 - (xd_{i,k}-xdp_{j,k})^2/ell_k^4]
  //   if dim_i != dim_j:
  //     = K(xd_i,xdp_j) * [-(xd_{i,ki}-xdp_{j,ki})*(xd_{i,kj}-xdp_{j,kj})/(ell_ki^2*ell_kj^2)]
  mooseAssert(dim_i < (unsigned)xd.sizes()[1], "dim_i out of range for computeCovarianceDD");
  mooseAssert(dim_j < (unsigned)xd.sizes()[1], "dim_j out of range for computeCovarianceDD");
  mooseAssert(xd.sizes()[1] == xdp.sizes()[1], "xd and xdp must have same number of columns");

  const auto options = xd.options().dtype(at::kDouble);
  const auto lf = _length_factor.to(options);
  const auto l_factor = lf.unsqueeze(0);

  torch::Tensor base = torch::cdist(torch::div(xd, l_factor), torch::div(xdp, l_factor), 2.0);
  base.pow_(2).mul_(-0.5).exp_().mul_(_sigma_f_squared.to(options));

  const auto ell_i_sq = lf.select(0, dim_i).pow(2);
  const auto ell_j_sq = lf.select(0, dim_j).pow(2);
  const auto d_i = xd.select(1, dim_i).unsqueeze(1) - xdp.select(1, dim_i).unsqueeze(0);
  const auto d_j = xd.select(1, dim_j).unsqueeze(1) - xdp.select(1, dim_j).unsqueeze(0);

  const auto factor = (dim_i == dim_j)
                          ? (ell_i_sq.reciprocal() - d_i.mul(d_j).div(ell_i_sq.mul(ell_j_sq)))
                          : (d_i.mul(d_j).div(ell_i_sq.mul(ell_j_sq)).mul(-1.0));
  K_dd = base.mul(factor);
}

void
SquaredExponentialCovariance::computedKdhyper_cross(torch::Tensor & dKdhp,
                                                    const torch::Tensor & x,
                                                    const torch::Tensor & xc,
                                                    const std::string & hyper_param_name,
                                                    unsigned int ind) const
{
  // Derivative of K(x_i, xc) w.r.t. each hyperparameter.
  // Used for the approximate penalty constraint gradient.
  const auto options = x.options().dtype(at::kDouble);

  if (name().length() + 1 > hyper_param_name.length())
  {
    dKdhp = torch::zeros({x.size(0), xc.size(0)}, options);
    return;
  }

  const std::string name_without_prefix = hyper_param_name.substr(name().length() + 1);

  if (name_without_prefix == "noise_variance")
  {
    // Cross-covariance does not depend on noise variance
    dKdhp = torch::zeros({x.size(0), xc.size(0)}, options);
    return;
  }

  if (name_without_prefix == "signal_variance")
  {
    // d(K(x_i,xc))/d(sigma_f^2) = exp(-r^2/2) = K(x_i,xc)/sigma_f^2
    SquaredExponentialFunction(dKdhp,
                               x,
                               xc,
                               _length_factor,
                               torch::tensor(1.0, options),
                               torch::tensor(0.0, options),
                               false);
    return;
  }

  if (name_without_prefix == "length_factor")
  {
    // d(K(x_i, xc)) / d(ell_ind) = K(x_i, xc) * (x_{i,ind} - xc_{ind})^2 / ell_ind^3
    const auto lf = _length_factor.to(options);
    const auto l_factor = lf.unsqueeze(0);
    torch::Tensor base = torch::cdist(torch::div(x, l_factor), torch::div(xc, l_factor), 2.0);
    base.pow_(2).mul_(-0.5).exp_().mul_(_sigma_f_squared.to(options));
    const auto diff = x.select(1, ind).unsqueeze(1) - xc.select(1, ind).unsqueeze(0);
    dKdhp = base.mul(diff.pow(2)).div(lf.select(0, ind).pow(3));
    return;
  }

  dKdhp = torch::zeros({x.size(0), xc.size(0)}, options);
}

#endif
