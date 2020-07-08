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
  params.addParam<Real>("gamma", "Gamma to use for Exponential Covariance Kernel");
  return params;
}

ExponentialCovariance::ExponentialCovariance(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters),
    _gamma(!_hyperparams.empty() ? _hyperparams[3][0] : getParam<Real>("gamma"))
{
}

// ExponentialCovariance::ExponentialCovariance(
//     const std::vector<Real> & length_factor,
//     const Real & sigma_f_squared,
//     const Real & sigma_n_squared,
//     const Real gamma)
//   : CovarianceFunctionBase(length_factor, sigma_f_squared, sigma_n_squared), _gamma(gamma)
// {
// }
//
// ExponentialCovariance::ExponentialCovariance(
//     const std::vector<std::vector<Real>> & vec)
//   : CovarianceFunctionBase(vec)
// {
//   _length_factor = vec[0];
//   _sigma_f_squared = vec[1][0];
//   _sigma_n_squared = vec[2][0];
//   _gamma = vec[3][0];
// }

void
ExponentialCovariance::getHyperParameters(std::vector<std::vector<Real>> & vec) const
{
  vec.resize(4);

  vec[0] = _length_factor;
  vec[1].push_back(_sigma_f_squared);
  vec[2].push_back(_sigma_n_squared);
  vec[3].push_back(_gamma);
}

RealEigenMatrix
ExponentialCovariance::computeCovarianceMatrix(const RealEigenMatrix & x,
                                               const RealEigenMatrix & xp,
                                               const bool is_self_covariance) const
{
  unsigned int num_samples_x = x.rows();
  unsigned int num_samples_xp = xp.rows();
  unsigned int num_params_x = x.cols();

  mooseAssert(num_params_x == xp.cols(),
              "Number of parameters do not match in covariance kernel calculation");

  RealEigenMatrix K(num_samples_x, num_samples_xp);

  for (unsigned int ii = 0; ii < num_samples_x; ++ii)
  {
    for (unsigned int jj = 0; jj < num_samples_xp; ++jj)
    {
      // Compute distance per parameter, scaled by length factor
      Real r_scaled = 0;
      for (unsigned int kk = 0; kk < num_params_x; ++kk)
        r_scaled += pow((x(ii, kk) - xp(jj, kk)) / _length_factor[kk], 2);
      r_scaled = sqrt(r_scaled);
      K(ii, jj) = _sigma_f_squared * std::exp(-pow(r_scaled, _gamma));
    }
    if (is_self_covariance)
      K(ii, ii) += _sigma_n_squared;
  }

  return K;
}
