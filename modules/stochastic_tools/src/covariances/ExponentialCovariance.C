//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  params.makeParamRequired<std::vector<Real>>("length_factor");
  params.makeParamRequired<Real>("signal_variance");
  params.makeParamRequired<Real>("noise_variance");
  params.addRequiredParam<Real>("gamma", "Gamma to use for Exponential Covariance Kernel");
  return params;
}

ExponentialCovariance::ExponentialCovariance(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters), _gamma(getParam<Real>("gamma"))
{
  _tunable_hp.insert("noise_variance");
  _tunable_hp.insert("signal_variance");
  _tunable_hp.insert("length_factor");
}

void
ExponentialCovariance::buildAdditionalHyperParamMap(
    std::unordered_map<std::string, Real> & map,
    std::unordered_map<std::string, std::vector<Real>> & /*vec_map*/) const
{
  map["gamma"] = _gamma;
}

void
ExponentialCovariance::loadAdditionalHyperParamMap(
    std::unordered_map<std::string, Real> & map,
    std::unordered_map<std::string, std::vector<Real>> & /*vec_map*/)
{
  _gamma = map["gamma"];
}

void
ExponentialCovariance::computeCovarianceMatrix(RealEigenMatrix & K,
                                               const RealEigenMatrix & x,
                                               const RealEigenMatrix & xp,
                                               const bool is_self_covariance) const
{
  if ((unsigned)x.cols() != _length_factor.size())
    mooseError("length_factor size does not match dimension of trainer input.");

  ExponentialFunction(
      K, x, xp, _length_factor, _sigma_f_squared, _sigma_n_squared, _gamma, is_self_covariance);
}

void
ExponentialCovariance::ExponentialFunction(RealEigenMatrix & K,
                                           const RealEigenMatrix & x,
                                           const RealEigenMatrix & xp,
                                           const std::vector<Real> & length_factor,
                                           const Real sigma_f_squared,
                                           const Real sigma_n_squared,
                                           const Real gamma,
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
      Real r_scaled = 0;
      for (unsigned int kk = 0; kk < num_params_x; ++kk)
        r_scaled += pow((x(ii, kk) - xp(jj, kk)) / length_factor[kk], 2);
      r_scaled = sqrt(r_scaled);
      K(ii, jj) = sigma_f_squared * std::exp(-pow(r_scaled, gamma));
    }
    if (is_self_covariance)
      K(ii, ii) += sigma_n_squared;
  }
}

void
ExponentialCovariance::computedKdhyper(RealEigenMatrix & dKdhp,
                                       const RealEigenMatrix & x,
                                       std::string hyper_param_name,
                                       unsigned int ind) const
{
  if (hyper_param_name == "noise_variance")
    ExponentialFunction(dKdhp, x, x, _length_factor, 0, 1, _gamma, true);

  if (hyper_param_name == "signal_variance")
    ExponentialFunction(dKdhp, x, x, _length_factor, 1, 0, _gamma, false);

  if (hyper_param_name == "length_factor")
    computedKdlf(dKdhp, x, _length_factor, _sigma_f_squared, _gamma, ind);
}

void
ExponentialCovariance::computedKdlf(RealEigenMatrix & K,
                                    const RealEigenMatrix & x,
                                    const std::vector<Real> & length_factor,
                                    const Real sigma_f_squared,
                                    const Real gamma,
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
      Real r_scaled = 0;
      for (unsigned int kk = 0; kk < num_params_x; ++kk)
        r_scaled += pow((x(ii, kk) - x(jj, kk)) / length_factor[kk], 2);
      r_scaled = sqrt(r_scaled);
      if (r_scaled != 0)
      {
        K(ii, jj) = gamma * std::pow(r_scaled, gamma - 2) * sigma_f_squared *
                    std::exp(-pow(r_scaled, gamma));
        K(ii, jj) =
            std::pow(x(ii, ind) - x(jj, ind), 2) / std::pow(length_factor[ind], 3) * K(ii, jj);
      }
      else // avoid div by 0. 0/0=0 scenario.
        K(ii, jj) = 0;
    }
  }
}
