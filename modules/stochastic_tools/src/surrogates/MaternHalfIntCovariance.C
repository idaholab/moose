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
  params.addClassDescription("Matern half-integer covariance function.");
  params.addRequiredParam<std::vector<Real>>("length_factor",
                                             "Length Factor to use for Covariance Kernel");
  params.addRequiredParam<Real>("signal_variance",
                                "Signal Variance ($\\sigma_f^2$) to use for kernel calculation.");
  params.addRequiredParam<Real>("noise_variance",
                                "Noise Variance ($\\sigma_n^2$) to use for kernel calculation.");
  params.addRequiredParam<unsigned int>(
      "p", "Integer p to use for Matern Half Integer Covariance Kernel");
  return params;
}

MaternHalfIntCovariance::MaternHalfIntCovariance(const InputParameters & parameters)
  : CovarianceFunctionBase(parameters),
    _length_factor(getParam<std::vector<Real>>("length_factor")),
    _sigma_f_squared(getParam<Real>("signal_variance")),
    _sigma_n_squared(getParam<Real>("noise_variance")),
    _p(getParam<unsigned int>("p"))
{
}

void
MaternHalfIntCovariance::buildHyperParamMap(
    std::unordered_map<std::string, Real> & map,
    std::unordered_map<std::string, std::vector<Real>> & vec_map) const
{
  map["signal_variance"] = _sigma_f_squared;
  map["noise_variance"] = _sigma_n_squared;
  map["p"] = _p;

  vec_map["length_factor"] = _length_factor;
}

void
MaternHalfIntCovariance::computeCovarianceMatrix(RealEigenMatrix & K,
                                                 const RealEigenMatrix & x,
                                                 const RealEigenMatrix & xp,
                                                 const bool is_self_covariance) const
{
  maternHalfIntFunction(
      K, x, xp, _length_factor, _sigma_f_squared, _sigma_n_squared, _p, is_self_covariance);
}

void
MaternHalfIntCovariance::maternHalfIntFunction(RealEigenMatrix & K,
                                               const RealEigenMatrix & x,
                                               const RealEigenMatrix & xp,
                                               const std::vector<Real> & length_factor,
                                               const Real sigma_f_squared,
                                               const Real sigma_n_squared,
                                               const unsigned int p,
                                               const bool is_self_covariance)
{
  unsigned int num_samples_x = x.rows();
  unsigned int num_samples_xp = xp.rows();
  unsigned int num_params_x = x.cols();

  mooseAssert(num_params_x == xp.cols(),
              "Number of parameters do not match in covariance kernel calculation");

  // This factor is used over and over, don't calculate each time
  Real factor = sqrt(2 * p + 1);

  for (unsigned int ii = 0; ii < num_samples_x; ++ii)
  {
    for (unsigned int jj = 0; jj < num_samples_xp; ++jj)
    {
      // Compute distance per parameter, scaled by length factor
      Real r_scaled = 0;
      for (unsigned int kk = 0; kk < num_params_x; ++kk)
        r_scaled += pow((x(ii, kk) - xp(jj, kk)) / length_factor[kk], 2);
      r_scaled = sqrt(r_scaled);
      Real summation = 0;
      // tgamma(x+1) == x! when x is a natural number, which should always be the case for
      // MaternHalfInt
      for (unsigned int tt = 0; tt < p + 1; ++tt)
        summation += (tgamma(p + tt + 1) / (tgamma(tt + 1) * tgamma(p - tt + 1))) *
                     pow(2 * factor * r_scaled, p - tt);
      K(ii, jj) = sigma_f_squared * std::exp(-factor * r_scaled) *
                  (tgamma(p + 1) / (tgamma(2 * p + 1))) * summation;
    }
    if (is_self_covariance)
      K(ii, ii) += sigma_n_squared;
  }
}
