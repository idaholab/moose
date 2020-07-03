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
  return params;
}

SquaredExponentialCovariance::SquaredExponentialCovariance(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters)
{
}

// SquaredExponentialCovariance::SquaredExponentialCovariance(
//     const std::vector<Real> & length_factor,
//     const Real & sigma_f_squared,
//     const Real & sigma_n_squared)
//   : CovarianceFunctionBase(length_factor, sigma_f_squared, sigma_n_squared)
// {
// }
//
// SquaredExponentialCovariance::SquaredExponentialCovariance(
//     const std::vector<std::vector<Real>> & vec)
//   : CovarianceFunctionBase(vec)
// {
//   _length_factor = vec[0];
//   _sigma_f_squared = vec[1][0];
//   _sigma_n_squared = vec[2][0];
// }

void
SquaredExponentialCovariance::getHyperParameters(std::vector<std::vector<Real>> & vec) const
{
  vec.resize(3);

  vec[0] = _length_factor;
  vec[1].push_back(_sigma_f_squared);
  vec[2].push_back(_sigma_n_squared);
}

RealEigenMatrix
SquaredExponentialCovariance::computeCovarianceMatrix(const RealEigenMatrix & x,
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
      Real r_squared_scaled = 0;
      for (unsigned int kk = 0; kk < num_params_x; ++kk)
        r_squared_scaled += std::pow((x(ii, kk) - xp(jj, kk)) / _length_factor[kk], 2);
      K(ii, jj) = _sigma_f_squared * std::exp(-r_squared_scaled / 2.0);
    }
    if (is_self_covariance)
      K(ii, ii) += _sigma_n_squared;
  }
  return K;
}
