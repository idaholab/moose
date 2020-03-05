//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "UniformDistribution.h"
#include "NormalDistribution.h"
#include "BoostNormalDistribution.h"
#include "libmesh/utility.h"

#include "CartesianProduct.h"

/**
 * Polynomials and quadratures based on defined distributions for Polynomial Chaos
 */
namespace PolynomialQuadrature
{
class Polynomial;

std::unique_ptr<const Polynomial> makePolynomial(const Distribution * dist);

/**
 * General polynomial class with function for evaluating a polynomial of a given
 * order at a given point
 */
class Polynomial
{
public:
  Polynomial() {}
  virtual ~Polynomial() = default;
  virtual Real compute(const unsigned int order, const Real x, const bool normalize = true) const;
  virtual Real innerProduct(const unsigned int order) const = 0;

  virtual void quadrature(const unsigned int order,
                          std::vector<Real> & points,
                          std::vector<Real> & weights) const = 0;
  Real productIntegral(const std::vector<unsigned int> order) const;
};

/**
 * Uniform distributions use Legendre polynomials
 */
class Legendre : public Polynomial
{
public:
  Legendre(const UniformDistribution * dist);
  /// Legendre polynomial using static function then scales by <P_n^2> = 1 / (2n+1)
  virtual Real
  compute(const unsigned int order, const Real x, const bool normalize = true) const override;
  virtual Real innerProduct(const unsigned int order) const override
  {
    return 1. / (2. * (Real)order + 1.);
  };

  /// Gauss-Legendre quadrature: sum(weights) = 2
  virtual void quadrature(const unsigned int order,
                          std::vector<Real> & points,
                          std::vector<Real> & weights) const override;

private:
  const Real & _lower_bound;
  const Real & _upper_bound;
};

/**
 * Normal distributions use Hermite polynomials
 */
class Hermite : public Polynomial
{
public:
  Hermite(const NormalDistribution * dist);
  Hermite(const BoostNormalDistribution * dist);
  /// Hermite polynomial using static function then scales by <P_n^2> = n!
  virtual Real
  compute(const unsigned int order, const Real x, const bool normalize = true) const override;
  virtual Real innerProduct(const unsigned int order) const override
  {
    return (Real)Utility::factorial(order);
  };

  /// Gauss-Hermite quadrature: sum(weights) = sqrt(2\pi)
  virtual void quadrature(const unsigned int order,
                          std::vector<Real> & points,
                          std::vector<Real> & weights) const override;

private:
  const Real & _mu;
  const Real & _sig;
};

/**
 * General multidimensional quadrature class
 */
class Quadrature
{
public:
  Quadrature() {}
  virtual ~Quadrature() = default;

  /// Resulting number of quadrature points in grid
  virtual unsigned int nPoints() const = 0;
  /// Inputted number of dimensions
  virtual unsigned int nDim() const = 0;

  /// Quadrature point n
  virtual std::vector<Real> quadraturePoint(const unsigned int n) const = 0;
  /// Quadrature point n for dimension dim
  virtual Real quadraturePoint(const unsigned int n, const unsigned int dim) const = 0;
  /// Weight for quadrature point n
  virtual Real quadratureWeight(const unsigned int n) const = 0;
};

/**
 * Full tensor product of 1D quadratures
 */
class TensorGrid : public Quadrature
{
public:
  TensorGrid(const std::vector<unsigned int> & npoints,
             std::vector<std::unique_ptr<const Polynomial>> & poly);
  TensorGrid(const unsigned int npoints, std::vector<std::unique_ptr<const Polynomial>> & poly)
    : TensorGrid(std::vector<unsigned int>(poly.size(), npoints), poly)
  {
  }

  virtual unsigned int nPoints() const override { return _quad->numRows(); };
  virtual unsigned int nDim() const override { return _quad->numCols(); };

  virtual std::vector<Real> quadraturePoint(const unsigned int n) const override
  {
    return _quad->computeRow(n);
  };
  virtual Real quadraturePoint(const unsigned int n, const unsigned int dim) const override
  {
    return _quad->computeValue(n, dim);
  };
  virtual Real quadratureWeight(const unsigned int n) const override
  {
    return _quad->computeWeight(n);
  };

private:
  std::unique_ptr<const StochasticTools::WeightedCartesianProduct<Real, Real>> _quad = nullptr;
};

/**
 * Legendre polynomial of specified order. Uses boost if available
 */
Real legendre(const unsigned int order,
              const Real x,
              const Real lower_bound = -1.0,
              const Real upper_bound = 1.0);

/**
 * Hermite polynomial of specified order. Uses boost if available.
 */
Real hermite(const unsigned int order, const Real x, const Real mu = 0.0, const Real sig = 1.0);

/**
 * Generalized formula for any polynomial order. Resulting number of points is then:
 *   N = order + 1
 * The sum of weights is 2
 * Uses the Golub-Welsch algorithm:
 * - Golub, Gene & Welsch, John ``Calculation of Gauss Quadrature Rules''.
 *   https://web.stanford.edu/class/cme335/S0025-5718-69-99647-1.pdf
 */
void gauss_legendre(const unsigned int order,
                    std::vector<Real> & points,
                    std::vector<Real> & weights,
                    const Real lower_bound,
                    const Real upper_bound);

/**
 * Generalized formula for any polynomial order. Resulting number of points is then:
 *   N = order + 1
 * The sum of weights is sqrt(2*\pi)
 * Uses the Golub-Welsch algorithm:
 * - Golub, Gene & Welsch, John ``Calculation of Gauss Quadrature Rules''.
 *   https://web.stanford.edu/class/cme335/S0025-5718-69-99647-1.pdf
 */
void gauss_hermite(const unsigned int order,
                   std::vector<Real> & points,
                   std::vector<Real> & weights,
                   const Real mu,
                   const Real sig);
}
