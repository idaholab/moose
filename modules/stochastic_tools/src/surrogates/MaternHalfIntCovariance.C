//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaternHalfIntCovariance.h"
#include <cmath>

registerMooseObject("StochasticToolsApp", MaternHalfIntCovariance);

InputParameters
MaternHalfIntCovariance::validParams()
{
  InputParameters params = CovarianceFunctionBase::validParams();
  params.addParam<unsigned int>(
      "p", "Integer p to use for Matern Hald Integer Covariance Kernel");
  return params;
}

MaternHalfIntCovariance::MaternHalfIntCovariance(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters),
  _p(!_hyperparams.empty() ? round(_hyperparams[3][0]): getParam<unsigned int>("p"))
{
}

// MaternHalfIntCovariance::MaternHalfIntCovariance(
//     const std::vector<Real> & length_factor,
//     const Real & sigma_f_squared,
//     const Real & sigma_n_squared,
//     const unsigned int & p)
//   : CovarianceFunctionBase(length_factor, sigma_f_squared, sigma_n_squared), _p(p)
// {
// }
//
// MaternHalfIntCovariance::MaternHalfIntCovariance(
//     const std::vector<std::vector<Real>> & vec)
//   : CovarianceFunctionBase(vec)
// {
//   _length_factor = vec[0];
//   _sigma_f_squared = vec[1][0];
//   _sigma_n_squared = vec[2][0];
//   _p = (unsigned int)round(vec[3][0]);
// }

void
MaternHalfIntCovariance::getHyperParameters(std::vector<std::vector<Real>> & vec) const
{
  vec.resize(4);

  vec[0] = _length_factor;
  vec[1].push_back(_sigma_f_squared);
  vec[2].push_back(_sigma_n_squared);
  vec[3].push_back(_p);
}

RealEigenMatrix
MaternHalfIntCovariance::computeCovarianceMatrix(const RealEigenMatrix & x,
                                                 const RealEigenMatrix & xp,
                                                 const bool is_self_covariance) const
{
  unsigned int num_samples_x = x.rows();
  unsigned int num_samples_xp = xp.rows();
  unsigned int num_params_x = x.cols();

  mooseAssert(num_params_x == xp.cols(),
              "Number of parameters do not match in covariance kernel calculation");

  RealEigenMatrix K(num_samples_x, num_samples_xp);

  // This factor is used over and over, don't calculate each time
  Real factor = sqrt(2 * _p + 1);

  for (unsigned int ii = 0; ii < num_samples_x; ++ii)
  {
    for (unsigned int jj = 0; jj < num_samples_xp; ++jj)
    {
      // Compute distance per parameter, scaled by length factor
      Real r_scaled = 0;
      for (unsigned int kk = 0; kk < num_params_x; ++kk)
        r_scaled += pow((x(ii, kk) - xp(jj, kk)) / _length_factor[kk], 2);
      r_scaled = sqrt(r_scaled);
      Real summation = 0;
      // tgamma(x+1) == x! when x is a natural number, which should always be the case for
      // MaternHalfInt
      for (unsigned int tt = 0; tt < _p + 1; ++tt)
        summation += (tgamma(_p + tt + 1) / (tgamma(tt + 1) * tgamma(_p - tt + 1))) *
                     pow(2 * factor * r_scaled, _p - tt);
      K(ii, jj) = _sigma_f_squared * std::exp(-factor * r_scaled) *
                  (tgamma(_p + 1) / (tgamma(2 * _p + 1))) * summation;
    }
    if (is_self_covariance)
      K(ii, ii) += _sigma_n_squared;
  }
  return K;
}
