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
//#include "Uniform.h"
//#include "Normal.h"
//#include "BoostNormal.h"
//#include "CartesianProduct.h"

/**
 * Polynomials and quadratures based on defined distributions for Polynomial Chaos
 */

class GaussianProcessTrainer;

namespace CovarianceFunction
{
class CovarianceKernel;

std::unique_ptr<CovarianceKernel> makeCovarianceKernel(const MooseEnum & kernel_type, const GaussianProcessTrainer * gp);

MooseEnum makeCovarianceKernelEnum();

/**
 * General polynomial class with function for evaluating a polynomial of a given
 * order at a given point
 */
class CovarianceKernel
{
public:
  CovarianceKernel() {};
  virtual ~CovarianceKernel() = default;
  virtual void store(std::ostream & stream, void * context) const;
  //Generates the Covariance Matrix given two points
  virtual RealEigenMatrix compute_matrix(const RealEigenMatrix x, const RealEigenMatrix xp) const;
  virtual void set_signal_variance(const Real sig_f);
  virtual void set_length_factor(const std::vector<Real> length_factor);
  virtual void set_gamma(const Real gamma);
  // /// Computes the mth derivative of polynomial: d^mP_n/dx^m
  // virtual Real
  // computeDerivative(const unsigned int order, const Real x, const unsigned int m = 1) const;
  // virtual Real innerProduct(const unsigned int order) const = 0;
  //
  // virtual void gaussQuadrature(const unsigned int order,
  //                              std::vector<Real> & points,
  //                              std::vector<Real> & weights) const = 0;
  // virtual void clenshawQuadrature(const unsigned int order,
  //                                 std::vector<Real> & points,
  //                                 std::vector<Real> & weights) const;
  // Real productIntegral(const std::vector<unsigned int> order) const;
protected:
    Real _sigma_f=1;
    Real _sigma_n=0;
    std::vector<Real> _length_factor;
};

/**
 * Uniform distributions use Legendre polynomials
 */
class SquaredExponential : public CovarianceKernel
{
public:
  SquaredExponential();
  virtual void store(std::ostream & stream, void * context) const override;

  ///
  virtual RealEigenMatrix compute_matrix(const RealEigenMatrix x, const RealEigenMatrix xp) const override;

};

/**
 * Uniform distributions use Legendre polynomials
 */
class Exponential : public CovarianceKernel
{
public:
  Exponential();
  virtual void store(std::ostream & stream, void * context) const override;
  virtual void set_gamma(const Real gamma) override;

  ///
  virtual RealEigenMatrix compute_matrix(const RealEigenMatrix x, const RealEigenMatrix xp) const override;

private:
    Real _gamma;

};



}

template <>
void dataStore(std::ostream & stream,
               std::unique_ptr<CovarianceFunction::CovarianceKernel> & ptr,
               void * context);
template <>
void dataLoad(std::istream & stream,
              std::unique_ptr<CovarianceFunction::CovarianceKernel> & ptr,
              void * context);
