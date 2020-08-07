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
  params.addRequiredParam<std::vector<Real>>("length_factor",
                                             "Length Factor to use for Covariance Kernel");
  params.addRequiredParam<Real>("signal_variance",
                                "Signal Variance ($\\sigma_f^2$) to use for kernel calculation.");
  params.addRequiredParam<Real>("noise_variance",
                                "Noise Variance ($\\sigma_n^2$) to use for kernel calculation.");
  params.addRequiredParam<Real>("gamma", "Gamma to use for Exponential Covariance Kernel");
  params.addParam<std::vector<std::string>>("tune_parameters",
                                            "Select hyperparameters to be tuned");
  params.addParam<std::vector<Real>>("tuning_min", "Minimum allowable tuning value");
  params.addParam<std::vector<Real>>("tuning_max", "Maximum allowable tuning value");
  return params;
}

ExponentialCovariance::ExponentialCovariance(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters),
    _length_factor(getParam<std::vector<Real>>("length_factor")),
    _sigma_f_squared(getParam<Real>("signal_variance")),
    _sigma_n_squared(getParam<Real>("noise_variance")),
    _gamma(getParam<Real>("gamma"))
{
  _num_tunable = 0;
  std::vector<std::string> tune_parameters(getParam<std::vector<std::string>>("tune_parameters"));
  // Error Checking
  if (isParamValid("tuning_min") &&
      (getParam<std::vector<Real>>("tuning_min").size() != tune_parameters.size()))
    ::mooseError("tuning_min size does not match tune_parameters");
  if (isParamValid("tuning_max") &&
      (getParam<std::vector<Real>>("tuning_max").size() != tune_parameters.size()))
    ::mooseError("tuning_max size does not match tune_parameters");
  // Fill Out Tunable Paramater information
  for (unsigned int ii = 0; ii < tune_parameters.size(); ++ii)
  {
    const auto & hp = tune_parameters[ii];
    if (!isParamValid(hp))
      ::mooseError("Parameter ", hp, " selected for tuning is not a valid parameter");
    if ((hp == "noise_variance") || (hp == "signal_variance"))
    {
      // For Scalar Hyperparameters
      Real min(isParamValid("tuning_min") ? getParam<std::vector<Real>>("tuning_min")[ii] : 1e-9);
      Real max(isParamValid("tuning_max") ? getParam<std::vector<Real>>("tuning_max")[ii]
                                          : PETSC_INFINITY);
      _tuning_data[hp] = std::make_tuple(_num_tunable, min, max);
      _num_tunable++;
    }
    else if (hp == "length_factor")
    {
      // For Vector Hyperparameters
      int vec_size = getParam<std::vector<Real>>("length_factor").size();
      Real min(isParamValid("tuning_min") ? getParam<std::vector<Real>>("tuning_min")[ii] : 1e-9);
      Real max(isParamValid("tuning_max") ? getParam<std::vector<Real>>("tuning_max")[ii]
                                          : PETSC_INFINITY);
      _tuning_data[hp] = std::make_tuple(_num_tunable, min, max);
      _num_tunable += vec_size;
    }
    else
      ::mooseError("Tuning not supported for parameter ", hp);
  }
}

void
ExponentialCovariance::buildHyperParamMap(
    std::unordered_map<std::string, Real> & map,
    std::unordered_map<std::string, std::vector<Real>> & vec_map) const
{
  map["noise_variance"] = _sigma_n_squared;
  map["signal_variance"] = _sigma_f_squared;
  map["gamma"] = _gamma;

  vec_map["length_factor"] = _length_factor;
}

void
ExponentialCovariance::buildHyperParamVec(libMesh::PetscVector<Number> & theta) const
{
  auto iter = _tuning_data.find("noise_variance");
  if (iter != _tuning_data.end())
    theta.set(std::get<0>(iter->second), _sigma_n_squared);

  iter = _tuning_data.find("signal_variance");
  if (iter != _tuning_data.end())
    theta.set(std::get<0>(iter->second), _sigma_f_squared);

  iter = _tuning_data.find("length_factor");
  if (iter != _tuning_data.end())
  {
    for (unsigned int ii = 0; ii < _length_factor.size(); ++ii)
    {
      theta.set(std::get<0>(iter->second) + ii, _length_factor[ii]);
    }
  }
}

void
ExponentialCovariance::buildHyperParamBounds(libMesh::PetscVector<Number> & theta_l,
                                             libMesh::PetscVector<Number> & theta_u) const
{
  auto iter = _tuning_data.find("noise_variance");
  if (iter != _tuning_data.end())
  {
    theta_l.set(std::get<0>(iter->second), std::get<1>(iter->second));
    theta_u.set(std::get<0>(iter->second), std::get<2>(iter->second));
  }

  iter = _tuning_data.find("signal_variance");
  if (iter != _tuning_data.end())
  {
    theta_l.set(std::get<0>(iter->second), std::get<1>(iter->second));
    theta_u.set(std::get<0>(iter->second), std::get<2>(iter->second));
  }

  iter = _tuning_data.find("length_factor");
  if (iter != _tuning_data.end())
  {
    for (unsigned int ii = 0; ii < _length_factor.size(); ++ii)
    {
      theta_l.set(std::get<0>(iter->second) + ii, std::get<1>(iter->second));
      theta_u.set(std::get<0>(iter->second) + ii, std::get<2>(iter->second));
    }
  }
}

void
ExponentialCovariance::loadHyperParamVec(libMesh::PetscVector<Number> & theta)
{
  auto iter = _tuning_data.find("noise_variance");
  if (iter != _tuning_data.end())
    _sigma_n_squared = theta(std::get<0>(iter->second));

  iter = _tuning_data.find("signal_variance");
  if (iter != _tuning_data.end())
    _sigma_f_squared = theta(std::get<0>(iter->second));

  iter = _tuning_data.find("length_factor");
  if (iter != _tuning_data.end())
  {
    for (unsigned int ii = 0; ii < _length_factor.size(); ++ii)
    {
      _length_factor[ii] = theta(std::get<0>(iter->second) + ii);
    }
  }
}

void
ExponentialCovariance::computeCovarianceMatrix(RealEigenMatrix & K,
                                               const RealEigenMatrix & x,
                                               const RealEigenMatrix & xp,
                                               const bool is_self_covariance) const
{
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
                                       unsigned int hyper_param_id) const
{
  auto iter = _tuning_data.find("noise_variance");
  if (iter != _tuning_data.end() && hyper_param_id == std::get<0>(iter->second))
    ExponentialFunction(dKdhp, x, x, _length_factor, 0, 1, _gamma, true);

  iter = _tuning_data.find("signal_variance");
  if (iter != _tuning_data.end() && hyper_param_id == std::get<0>(iter->second))
    ExponentialFunction(dKdhp, x, x, _length_factor, 1, 0, _gamma, false);

  iter = _tuning_data.find("length_factor");
  if (iter != _tuning_data.end() && hyper_param_id >= std::get<0>(iter->second) &&
      hyper_param_id < std::get<0>(iter->second) + _length_factor.size())
  {
    computedKdlf(dKdhp,
                 x,
                 _length_factor,
                 _sigma_f_squared,
                 _gamma,
                 hyper_param_id - std::get<0>(iter->second));
  }
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
