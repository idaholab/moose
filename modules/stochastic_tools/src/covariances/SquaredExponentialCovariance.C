//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
SquaredExponentialCovariance::computeCovarianceMatrix(RealEigenMatrix & K,
                                                      const RealEigenMatrix & x,
                                                      const RealEigenMatrix & xp,
                                                      const bool is_self_covariance) const
{
  if ((unsigned)x.cols() != _length_factor.size())
    mooseError("length_factor size does not match dimension of trainer input.");

  SquaredExponentialFunction(
      K, x, xp, _length_factor, _sigma_f_squared, _sigma_n_squared, is_self_covariance);
}

void
SquaredExponentialCovariance::SquaredExponentialFunction(RealEigenMatrix & K,
                                                         const RealEigenMatrix & x,
                                                         const RealEigenMatrix & xp,
                                                         const std::vector<Real> & length_factor,
                                                         const Real sigma_f_squared,
                                                         const Real sigma_n_squared,
                                                         const bool is_self_covariance)
{
  unsigned int num_samples_x = x.rows();
  unsigned int num_samples_xp = xp.rows();
  unsigned int num_params_x = x.cols();

  mooseAssert(num_params_x == xp.cols(),
              "Number of parameters do not match in covariance kernel calculation");

  for (unsigned int ii = 0; ii < num_samples_x; ++ii)
  {
    for (unsigned int jj = 0; jj < num_samples_xp; ++jj)
    {
      // Compute distance per parameter, scaled by length factor
      Real r_squared_scaled = 0;
      for (unsigned int kk = 0; kk < num_params_x; ++kk)
        r_squared_scaled += std::pow((x(ii, kk) - xp(jj, kk)) / length_factor[kk], 2);
      K(ii, jj) = sigma_f_squared * std::exp(-r_squared_scaled / 2.0);
    }
    if (is_self_covariance)
      K(ii, ii) += sigma_n_squared;
  }
}

bool
SquaredExponentialCovariance::computedKdhyper(RealEigenMatrix & dKdhp,
                                              const RealEigenMatrix & x,
                                              const std::string & hyper_param_name,
                                              unsigned int ind) const
{
  if (name().length() + 1 > hyper_param_name.length())
    return false;

  const std::string name_without_prefix = hyper_param_name.substr(name().length() + 1);

  if (name_without_prefix == "noise_variance")
  {
    SquaredExponentialFunction(dKdhp, x, x, _length_factor, 0, 1, true);
    return true;
  }

  if (name_without_prefix == "signal_variance")
  {
    SquaredExponentialFunction(dKdhp, x, x, _length_factor, 1, 0, false);
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
SquaredExponentialCovariance::computedKdlf(RealEigenMatrix & K,
                                           const RealEigenMatrix & x,
                                           const std::vector<Real> & length_factor,
                                           const Real sigma_f_squared,
                                           const int ind)
{
  unsigned int num_samples_x = x.rows();
  unsigned int num_params_x = x.cols();

  mooseAssert(ind < x.cols(), "Incorrect length factor index");

  for (unsigned int ii = 0; ii < num_samples_x; ++ii)
  {
    for (unsigned int jj = 0; jj < num_samples_x; ++jj)
    {
      // Compute distance per parameter, scaled by length factor
      Real r_squared_scaled = 0;
      for (unsigned int kk = 0; kk < num_params_x; ++kk)
        r_squared_scaled += std::pow((x(ii, kk) - x(jj, kk)) / length_factor[kk], 2);
      K(ii, jj) = sigma_f_squared * std::exp(-r_squared_scaled / 2.0);
      K(ii, jj) =
          std::pow(x(ii, ind) - x(jj, ind), 2) / std::pow(length_factor[ind], 3) * K(ii, jj);
    }
  }
}

void
SquaredExponentialCovariance::computeCovarianceFD(RealEigenMatrix & K_fd,
                                                  const RealEigenMatrix & x,
                                                  const RealEigenMatrix & xd,
                                                  unsigned int dim) const
{
  // K_fd[i,j] = Cov[f(x_i), df(xd_j)/dx'_{dim}]
  //           = dK(x_i, xd_j)/dx'_{dim}
  //           = K(x_i,xd_j) * (x_{i,dim} - xd_{j,dim}) / ell_dim^2
  const unsigned int n_x = x.rows();
  const unsigned int n_xd = xd.rows();
  const unsigned int n_params = x.cols();
  mooseAssert(dim < n_params, "dim out of range for computeCovarianceFD");
  mooseAssert((unsigned)xd.cols() == n_params, "x and xd must have same number of columns");

  K_fd.resize(n_x, n_xd);
  for (unsigned int ii = 0; ii < n_x; ++ii)
  {
    for (unsigned int jj = 0; jj < n_xd; ++jj)
    {
      Real r_sq = 0.0;
      for (unsigned int kk = 0; kk < n_params; ++kk)
        r_sq += std::pow((x(ii, kk) - xd(jj, kk)) / _length_factor[kk], 2);
      const Real k_val = _sigma_f_squared * std::exp(-r_sq / 2.0);
      K_fd(ii, jj) =
          k_val * (x(ii, dim) - xd(jj, dim)) / (_length_factor[dim] * _length_factor[dim]);
    }
  }
}

void
SquaredExponentialCovariance::computeCovarianceDf(RealEigenMatrix & K_df,
                                                  const RealEigenMatrix & xd,
                                                  const RealEigenMatrix & xp,
                                                  unsigned int dim) const
{
  // K_df[i,j] = Cov[df(xd_i)/dx_{dim}, f(xp_j)]
  //           = dK(xd_i, xp_j)/dx_{d,dim}
  //           = K(xd_i,xp_j) * (xp_{j,dim} - xd_{i,dim}) / ell_dim^2
  // Note: K_df = K_fd^T (transpose of computeCovarianceFD with swapped args)
  RealEigenMatrix K_fd;
  computeCovarianceFD(K_fd, xp, xd, dim);
  K_df = K_fd.transpose();
}

void
SquaredExponentialCovariance::computeCovarianceDD(RealEigenMatrix & K_dd,
                                                  const RealEigenMatrix & xd,
                                                  const RealEigenMatrix & xdp,
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
  const unsigned int n_i = xd.rows();
  const unsigned int n_j = xdp.rows();
  const unsigned int n_params = xd.cols();
  mooseAssert(dim_i < n_params, "dim_i out of range for computeCovarianceDD");
  mooseAssert(dim_j < n_params, "dim_j out of range for computeCovarianceDD");
  mooseAssert((unsigned)xdp.cols() == n_params, "xd and xdp must have same number of columns");

  K_dd.resize(n_i, n_j);
  const Real ell_i_sq = _length_factor[dim_i] * _length_factor[dim_i];
  const Real ell_j_sq = _length_factor[dim_j] * _length_factor[dim_j];

  for (unsigned int ii = 0; ii < n_i; ++ii)
  {
    for (unsigned int jj = 0; jj < n_j; ++jj)
    {
      Real r_sq = 0.0;
      for (unsigned int kk = 0; kk < n_params; ++kk)
        r_sq += std::pow((xd(ii, kk) - xdp(jj, kk)) / _length_factor[kk], 2);
      const Real k_val = _sigma_f_squared * std::exp(-r_sq / 2.0);

      const Real d_i = xd(ii, dim_i) - xdp(jj, dim_i);
      const Real d_j = xd(ii, dim_j) - xdp(jj, dim_j);

      if (dim_i == dim_j)
        K_dd(ii, jj) = k_val * (1.0 / ell_i_sq - d_i * d_j / (ell_i_sq * ell_j_sq));
      else
        K_dd(ii, jj) = k_val * (-d_i * d_j / (ell_i_sq * ell_j_sq));
    }
  }
}

void
SquaredExponentialCovariance::computedKdhyper_cross(RealEigenMatrix & dKdhp,
                                                    const RealEigenMatrix & x,
                                                    const RealEigenMatrix & xc,
                                                    const std::string & hyper_param_name,
                                                    unsigned int ind) const
{
  // Derivative of K(x_i, xc) w.r.t. each hyperparameter.
  // Used for the approximate penalty constraint gradient.
  if (name().length() + 1 > hyper_param_name.length())
  {
    dKdhp.setZero(x.rows(), xc.rows());
    return;
  }

  const std::string name_without_prefix = hyper_param_name.substr(name().length() + 1);

  if (name_without_prefix == "noise_variance")
  {
    // Cross-covariance does not depend on noise variance
    dKdhp.setZero(x.rows(), xc.rows());
    return;
  }

  if (name_without_prefix == "signal_variance")
  {
    // d(K(x_i,xc))/d(sigma_f^2) = exp(-r^2/2) = K(x_i,xc)/sigma_f^2
    SquaredExponentialFunction(dKdhp, x, xc, _length_factor, 1.0, 0.0, false);
    return;
  }

  if (name_without_prefix == "length_factor")
  {
    // d(K(x_i, xc)) / d(ell_ind) = K(x_i, xc) * (x_{i,ind} - xc_{ind})^2 / ell_ind^3
    const unsigned int n_x = x.rows();
    const unsigned int n_xc = xc.rows();
    const unsigned int n_params = x.cols();
    dKdhp.resize(n_x, n_xc);
    for (unsigned int ii = 0; ii < n_x; ++ii)
    {
      for (unsigned int jj = 0; jj < n_xc; ++jj)
      {
        Real r_sq = 0.0;
        for (unsigned int kk = 0; kk < n_params; ++kk)
          r_sq += std::pow((x(ii, kk) - xc(jj, kk)) / _length_factor[kk], 2);
        const Real k_val = _sigma_f_squared * std::exp(-r_sq / 2.0);
        dKdhp(ii, jj) =
            k_val * std::pow(x(ii, ind) - xc(jj, ind), 2) / std::pow(_length_factor[ind], 3);
      }
    }
    return;
  }

  dKdhp.setZero(x.rows(), xc.rows());
}
