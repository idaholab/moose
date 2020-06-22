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
  virtual void store(std::ostream & stream, void * context) const;
  // Generates the Covariance Matrix given two points
  virtual RealEigenMatrix
  compute_K(const RealEigenMatrix x, const RealEigenMatrix xp, const bool is_self_covariance) const;
  virtual void set_signal_variance(const Real sigma_f_squared);
  virtual void set_noise_variance(const Real sigma_n_squared);
  virtual void set_length_factor(const std::vector<Real> length_factor);
  virtual void set_gamma(const Real gamma);
  virtual void set_p(const unsigned int p);

protected:
  Real _sigma_f_squared;
  Real _sigma_n_squared;
  std::vector<Real> _length_factor;
};

class SquaredExponential : public CovarianceKernel
{
public:
  SquaredExponential();
  virtual void store(std::ostream & stream, void * context) const override;

  ///
  virtual RealEigenMatrix compute_K(const RealEigenMatrix x,
                                    const RealEigenMatrix xp,
                                    const bool is_self_covariance) const override;
};

class Exponential : public CovarianceKernel
{
public:
  Exponential();
  virtual void store(std::ostream & stream, void * context) const override;
  virtual void set_gamma(const Real gamma) override;

  ///
  virtual RealEigenMatrix compute_K(const RealEigenMatrix x,
                                    const RealEigenMatrix xp,
                                    const bool is_self_covariance) const override;

private:
  Real _gamma;
};

class MaternHalfInt : public CovarianceKernel
{
public:
  MaternHalfInt();
  virtual void store(std::ostream & stream, void * context) const override;
  virtual void set_p(const unsigned int p) override;

  ///
  virtual RealEigenMatrix compute_K(const RealEigenMatrix x,
                                    const RealEigenMatrix xp,
                                    const bool is_self_covariance) const override;

private:
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
