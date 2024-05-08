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
                                             "Length factors to use for Covariance Kernel");
  params.addRequiredParam<Real>("signal_variance",
                                "Signal Variance ($\\sigma_f^2$) to use for kernel calculation.");
  params.addParam<Real>(
      "noise_variance", 0.0, "Noise Variance ($\\sigma_n^2$) to use for kernel calculation.");
  return params;
}

SquaredExponentialCovariance::SquaredExponentialCovariance(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters),
    _length_factor(getParam<std::vector<Real>>("length_factor")),
    _sigma_f_squared(getParam<Real>("signal_variance")),
    _sigma_n_squared(getParam<Real>("noise_variance"))
{
  _tunable_hp.insert("noise_variance");
  _tunable_hp.insert("signal_variance");
  _tunable_hp.insert("length_factor");
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
SquaredExponentialCovariance::loadHyperParamMap(
    std::unordered_map<std::string, Real> & map,
    std::unordered_map<std::string, std::vector<Real>> & vec_map)
{
  _sigma_n_squared = map["noise_variance"];
  _sigma_f_squared = map["signal_variance"];
  _length_factor = vec_map["length_factor"];
}

void
SquaredExponentialCovariance::getTuningData(std::string name,
                                            unsigned int & size,
                                            Real & min,
                                            Real & max) const
{
  if ((name == "noise_variance") || (name == "signal_variance"))
  {
    min = 1e-9;
    max = 1e9;
    size = 1;
  }
  else if (name == "length_factor")
  {
    min = 1e-9;
    max = 1e9;
    size = _length_factor.size();
  }
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

void
SquaredExponentialCovariance::computedKdhyper(RealEigenMatrix & dKdhp,
                                              const RealEigenMatrix & x,
                                              std::string hyper_param_name,
                                              unsigned int ind) const
{
  if (hyper_param_name == "noise_variance")
    SquaredExponentialFunction(dKdhp, x, x, _length_factor, 0, 1, true);

  if (hyper_param_name == "signal_variance")
    SquaredExponentialFunction(dKdhp, x, x, _length_factor, 1, 0, false);

  if (hyper_param_name == "length_factor")
    computedKdlf(dKdhp, x, _length_factor, _sigma_f_squared, ind);
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
