//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
                                             "Length Factor to use for Covariance Kernel");
  params.addRequiredParam<Real>("signal_variance",
                                "Signal Variance ($\\sigma_f^2$) to use for kernel calculation.");
  params.addRequiredParam<Real>("noise_variance",
                                "Noise Variance ($\\sigma_n^2$) to use for kernel calculation.");
  return params;
}

SquaredExponentialCovariance::SquaredExponentialCovariance(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters),
    _length_factor(getParam<std::vector<Real>>("length_factor")),
    _sigma_f_squared(getParam<Real>("signal_variance")),
    _sigma_n_squared(getParam<Real>("noise_variance"))
{
  _num_tunable = 2 + _length_factor.size();
}

void
SquaredExponentialCovariance::buildHyperParamMap(
    std::unordered_map<std::string, Real> & map,
    std::unordered_map<std::string, std::vector<Real>> & vec_map) const
{
  map["noise_variance"] = _sigma_n_squared;
  map["signal_variance"] = _sigma_f_squared;

  vec_map["length_factor"] = _length_factor;
}

void
SquaredExponentialCovariance::buildHyperParamVec(libMesh::PetscVector<Number> & theta) const
{
  theta.set(0, _sigma_n_squared);
  theta.set(1, _sigma_f_squared);
  for (unsigned int ii = 0; ii < _length_factor.size(); ++ii)
  {
    theta.set(2 + ii, _length_factor[ii]);
  }
}

void
SquaredExponentialCovariance::buildHyperParamBounds(libMesh::PetscVector<Number> & theta_l,
                                                    libMesh::PetscVector<Number> & theta_u) const
{
  theta_l.set(0, 0);
  theta_u.set(0, 1e16);
  theta_l.set(1, 1e-9);
  theta_u.set(1, 1e16);
  for (unsigned int ii = 0; ii < _length_factor.size(); ++ii)
  {
    theta_l.set(2 + ii, 1e-9);
    theta_u.set(2 + ii, 1e16);
  }
}

void
SquaredExponentialCovariance::loadHyperParamVec(libMesh::PetscVector<Number> & theta)
{
  _sigma_n_squared = theta(0);
  _sigma_f_squared = theta(1);
  for (unsigned int ii = 0; ii < _length_factor.size(); ++ii)
  {
    _length_factor[ii] = theta(2 + ii);
  }
}

void
SquaredExponentialCovariance::computeCovarianceMatrix(RealEigenMatrix & K,
                                                      const RealEigenMatrix & x,
                                                      const RealEigenMatrix & xp,
                                                      const bool is_self_covariance) const
{
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

void
SquaredExponentialCovariance::computedKdhyper(RealEigenMatrix & dKdhp,
                                              const RealEigenMatrix & x,
                                              unsigned int hyper_param_id) const
{
  if (hyper_param_id == 0) // sigma_n
  {
    SquaredExponentialFunction(dKdhp, x, x, _length_factor, 0, 1, true);
  }
  else if (hyper_param_id == 1) // sigma_f
  {
    SquaredExponentialFunction(dKdhp, x, x, _length_factor, 1, 0, false);
  }
  else if (hyper_param_id < _num_tunable)
  {
    computedKdlf(dKdhp, x, _length_factor, _sigma_f_squared, hyper_param_id - 2);
  }
  else
  {
    ::mooseError("Invalid hyperparam index");
  }
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
