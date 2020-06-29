//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CovarianceFunction.h"
#include "GaussianProcessTrainer.h"
#include "MooseError.h"
#include "DataIO.h"
#include "libmesh/auto_ptr.h"

#include "libmesh/dense_matrix_impl.h"

#include <cmath>

namespace CovarianceFunction
{

MooseEnum
makeCovarianceKernelEnum()
{
  return MooseEnum("squared_exponential=0 exponential=1 matern=2 matern_half_int=3");
}

std::unique_ptr<CovarianceKernel>
makeCovarianceKernel(const MooseEnum & kernel_type, const GaussianProcessTrainer * gp)
{

  std::vector<Real> length_factor = gp->getParam<std::vector<Real>>("length_factor");
  Real signal_variance = gp->getParam<Real>("signal_variance");
  Real noise_variance = gp->getParam<Real>("noise_variance");

  if (kernel_type == 0)
  {
    std::unique_ptr<CovarianceKernel> ptr = libmesh_make_unique<SquaredExponential>();
    ptr->set_signal_variance(signal_variance);
    ptr->set_noise_variance(noise_variance);
    ptr->set_length_factor(length_factor);
    return ptr;
  }

  if (kernel_type == 1)
  {
    std::unique_ptr<CovarianceKernel> ptr = libmesh_make_unique<Exponential>();
    Real gamma = gp->getParam<Real>("gamma");
    ptr->set_signal_variance(signal_variance);
    ptr->set_noise_variance(noise_variance);
    ptr->set_length_factor(length_factor);
    ptr->set_gamma(gamma);
    return ptr;
  }

  if (kernel_type == 2)
  {
    ::mooseError("General Matern covariance function not yet implemented. Half-integer Matern "
                 "implemented in MaternHalfInt if desired.");
    return nullptr;
  }

  if (kernel_type == 3)
  {
    std::unique_ptr<CovarianceKernel> ptr = libmesh_make_unique<MaternHalfInt>();
    unsigned int p = gp->getParam<unsigned int>("p");
    ptr->set_signal_variance(signal_variance);
    ptr->set_noise_variance(noise_variance);
    ptr->set_length_factor(length_factor);
    ptr->set_p(p);
    return ptr;
  }

  ::mooseError("Unknown covariance function type");
  return nullptr;
}

void
CovarianceKernel::store(std::ostream & /*stream*/, void * /*context*/) const
{
  // Cannot be pure virtual because for dataLoad operations the base class must be constructed
  ::mooseError("Covariance Function child class must override 'store()' method.");
}

RealEigenMatrix
CovarianceKernel::compute_K(const RealEigenMatrix /*x*/,
                            const RealEigenMatrix /*xp*/,
                            const bool /*is_self_covariance*/) const
{
  ::mooseError("Covariance function has not been implemented.");
  RealEigenMatrix tmp(1, 1);
  return tmp;
}

void
CovarianceKernel::set_signal_variance(const Real sigma_f_squared)
{
  _sigma_f_squared = sigma_f_squared;
}

void
CovarianceKernel::set_noise_variance(const Real sigma_n_squared)
{
  _sigma_n_squared = sigma_n_squared;
}

void
CovarianceKernel::set_length_factor(const std::vector<Real> length_factor)
{
  _length_factor = length_factor;
}

void
CovarianceKernel::set_gamma(const Real /*gamma*/)
{
  ::mooseError(
      "Attempting to set gamma hyper parameter for covariance function which does not utilize.");
}

void
CovarianceKernel::set_p(const unsigned int /*p*/)
{
  ::mooseError(
      "Attempting to set p hyper parameter for covariance function which does not utilize.");
}

// begin squared_exponential
SquaredExponential::SquaredExponential() : CovarianceKernel() {}

RealEigenMatrix
SquaredExponential::compute_K(const RealEigenMatrix x,
                              const RealEigenMatrix xp,
                              const bool is_self_covariance) const
{
  unsigned int num_samples_x = x.rows();
  unsigned int num_samples_xp = xp.rows();
  unsigned int num_params_x = x.cols();

  mooseAssert(num_params_x == xp.cols(),"Number of parameters do not match in covariance kernel calculation");


  RealEigenMatrix K(num_samples_x, num_samples_xp);

  for (unsigned int ii = 0; ii < num_samples_x; ++ii)
  {
    for (unsigned int jj = 0; jj < num_samples_xp; ++jj)
    {
      // Compute distance per parameter, scaled by length factor
      Real r_squared_scaled = 0;
      for (unsigned int kk = 0; kk < num_params_x; ++kk)
        r_squared_scaled += std::pow((x(ii, kk) - xp(jj, kk))/_length_factor[kk], 2);
      K(ii, jj)  = _sigma_f_squared * std::exp(-r_squared_scaled / 2.0);
    }
    if (is_self_covariance)
      K(ii, ii) += _sigma_n_squared;
  }

  return K;
}

void
SquaredExponential::store(std::ostream & stream, void * context) const
{
  std::string type = "Squared Exponential";
  dataStore(stream, type, context);
  dataStore(stream, _sigma_f_squared, context);
  dataStore(stream, _sigma_n_squared, context);
  unsigned int n = _length_factor.size();
  dataStore(stream, n, context);
  for (unsigned int ii = 0; ii < n; ++ii)
    dataStore(stream, _length_factor[ii], context);
}
// end squared_exponential

// begin exponential
Exponential::Exponential() : CovarianceKernel() {}

RealEigenMatrix
Exponential::compute_K(const RealEigenMatrix x,
                       const RealEigenMatrix xp,
                       const bool is_self_covariance) const
{
  unsigned int num_samples_x = x.rows();
  unsigned int num_samples_xp = xp.rows();
  unsigned int num_params_x = x.cols();

  mooseAssert(num_params_x == xp.cols(),"Number of parameters do not match in covariance kernel calculation");

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

void
Exponential::store(std::ostream & stream, void * context) const
{
  std::string type = "Exponential";
  dataStore(stream, type, context);
  dataStore(stream, _sigma_f_squared, context);
  dataStore(stream, _sigma_n_squared, context);
  unsigned int n = _length_factor.size();
  dataStore(stream, n, context);
  for (unsigned int ii = 0; ii < n; ++ii)
    dataStore(stream, _length_factor[ii], context);
  dataStore(stream, _gamma, context);
}

void
Exponential::set_gamma(const Real gamma)
{
  _gamma = gamma;
}
// end exponential

// begin matern half integer
MaternHalfInt::MaternHalfInt() : CovarianceKernel() {}

RealEigenMatrix
MaternHalfInt::compute_K(const RealEigenMatrix x,
                         const RealEigenMatrix xp,
                         const bool is_self_covariance) const
{
  unsigned int num_samples_x = x.rows();
  unsigned int num_samples_xp = xp.rows();
  unsigned int num_params_x = x.cols();

  mooseAssert(num_params_x == xp.cols(),"Number of parameters do not match in covariance kernel calculation");

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

void
MaternHalfInt::store(std::ostream & stream, void * context) const
{
  std::string type = "MaternHalfInt";
  dataStore(stream, type, context);
  dataStore(stream, _sigma_f_squared, context);
  dataStore(stream, _sigma_n_squared, context);
  unsigned int n = _length_factor.size();
  dataStore(stream, n, context);
  for (unsigned int ii = 0; ii < n; ++ii)
    dataStore(stream, _length_factor[ii], context);
  dataStore(stream, _p, context);
}

void
MaternHalfInt::set_p(const unsigned int p)
{
  _p = p;
}
// end matern half integer

} // namespace

template <>
void
dataStore(std::ostream & stream,
          std::unique_ptr<CovarianceFunction::CovarianceKernel> & ptr,
          void * context)
{
  ptr->store(stream, context);
}

template <>
void
dataLoad(std::istream & stream,
         std::unique_ptr<CovarianceFunction::CovarianceKernel> & ptr,
         void * context)
{
  std::string covar_type;
  dataLoad(stream, covar_type, context);
  Real sigma_f_squared, sigma_n_squared;
  dataLoad(stream, sigma_f_squared, context);
  dataLoad(stream, sigma_n_squared, context);
  unsigned int n;
  dataLoad(stream, n, context);
  std::vector<Real> length_factor(n);
  for (unsigned int ii = 0; ii < n; ++ii)
    dataLoad(stream, length_factor[ii], context);
  if (covar_type == "Squared Exponential")
  {
    ptr = libmesh_make_unique<CovarianceFunction::SquaredExponential>();
    ptr->set_length_factor(length_factor);
    ptr->set_signal_variance(sigma_f_squared);
  }
  else if (covar_type == "Exponential")
  {
    ptr = libmesh_make_unique<CovarianceFunction::Exponential>();
    ptr->set_length_factor(length_factor);
    ptr->set_signal_variance(sigma_f_squared);
    Real gamma;
    dataLoad(stream, gamma, context);
    ptr->set_gamma(gamma);
  }
  else if (covar_type == "MaternHalfInt")
  {
    ptr = libmesh_make_unique<CovarianceFunction::MaternHalfInt>();
    ptr->set_length_factor(length_factor);
    ptr->set_signal_variance(sigma_f_squared);
    unsigned int p;
    dataLoad(stream, p, context);
    ptr->set_p(p);
  }
  else
    ::mooseError("Unknown Covariance Function: ", covar_type);
}
