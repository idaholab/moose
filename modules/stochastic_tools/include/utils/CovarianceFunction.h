//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/utility.h"

#include "DataIO.h"
#include "MooseEnum.h"

class GaussianProcessTrainer;

namespace CovarianceFunction
{
class CovarianceKernel;

std::unique_ptr<CovarianceKernel> makeCovarianceKernel(const MooseEnum & kernel_type,
                                                       const GaussianProcessTrainer * gp);

MooseEnum makeCovarianceKernelEnum();

/**
 * General Covariacne Kernel Function class with operators necessary for evaluation and alering
 * hyperparameters
 */
class CovarianceKernel
{
public:
  CovarianceKernel(){};
  virtual ~CovarianceKernel() = default;

  /// Helper for dataStore
  virtual void store(std::ostream & stream, void * context) const;

  /// Generates the Covariance Matrix given two points in the parameter space
  virtual RealEigenMatrix
  compute_K(const RealEigenMatrix x, const RealEigenMatrix xp, const bool is_self_covariance) const;

  /// Sets the signal variance (\sigma_f^2) for the kernel
  virtual void set_signal_variance(const Real sigma_f_squared);

  /// Sets the noise variance (\sigma_n^2) for the kernel
  virtual void set_noise_variance(const Real sigma_n_squared);

  /// Sets the lengh factor (\ell) for the kernel. In vector form
  virtual void set_length_factor(const std::vector<Real> length_factor);

  /// Sets the exponential factor \gamma for use in Exponential kernel
  virtual void set_gamma(const Real gamma);

  /// Sets the p integer (p>=0) for used with Matern Half Integer kernel
  virtual void set_p(const unsigned int p);

protected:

  /// signal variance (\sigma_f^2)
  Real _sigma_f_squared;

  /// noise variance (\sigma_n^2)
  Real _sigma_n_squared;

  /// lengh factor (\ell) for the kernel, in vector form for multiple parameters
  std::vector<Real> _length_factor;

};

class SquaredExponential : public CovarianceKernel
{
public:
  SquaredExponential();

  /// Helper for dataStore
  virtual void store(std::ostream & stream, void * context) const override;

  /// Generates the Covariance Matrix given two points in the parameter space
  virtual RealEigenMatrix compute_K(const RealEigenMatrix x,
                                    const RealEigenMatrix xp,
                                    const bool is_self_covariance) const override;
};

class Exponential : public CovarianceKernel
{
public:
  Exponential();

  /// Helper for dataStore
  virtual void store(std::ostream & stream, void * context) const override;

  /// Sets the exponential factor \gamma for use in Exponential kernel
  virtual void set_gamma(const Real gamma) override;

  /// Generates the Covariance Matrix given two points in the parameter space
  virtual RealEigenMatrix compute_K(const RealEigenMatrix x,
                                    const RealEigenMatrix xp,
                                    const bool is_self_covariance) const override;

private:

  /// gamma exponential factor for use in kernel
  Real _gamma;
};

class MaternHalfInt : public CovarianceKernel
{
public:
  MaternHalfInt();

  /// Helper for dataStore
  virtual void store(std::ostream & stream, void * context) const override;

  /// Sets the p integer (p>=0) for used with Matern Half Integer kernel
  virtual void set_p(const unsigned int p) override;

  /// Generates the Covariance Matrix given two points in the parameter space
  virtual RealEigenMatrix compute_K(const RealEigenMatrix x,
                                    const RealEigenMatrix xp,
                                    const bool is_self_covariance) const override;

private:
  /// non-negative p factore for use in Matern half-int. \nu = p+(1/2) in terms of general Matern
  unsigned int _p;
};

} // end namespace

template <>
void dataStore(std::ostream & stream,
               std::unique_ptr<CovarianceFunction::CovarianceKernel> & ptr,
               void * context);
template <>
void dataLoad(std::istream & stream,
              std::unique_ptr<CovarianceFunction::CovarianceKernel> & ptr,
              void * context);
