//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolynomialQuadrature.h"

#include "MooseError.h"
#include "libmesh/auto_ptr.h"

// For computing legendre quadrature
#include "libmesh/dense_matrix_impl.h"

// For quickly computing polynomials
#ifdef LIBMESH_HAVE_EXTERNAL_BOOST
#include <boost/math/special_functions/legendre.hpp>
#include <boost/math/special_functions/hermite.hpp>
#endif

#include <cmath>

namespace PolynomialQuadrature
{

std::unique_ptr<const Polynomial>
makePolynomial(const Distribution * dist)
{
  const UniformDistribution * u_dist = dynamic_cast<const UniformDistribution *>(dist);
  if (u_dist)
    return libmesh_make_unique<const Legendre>(u_dist);

  const NormalDistribution * n_dist = dynamic_cast<const NormalDistribution *>(dist);
  if (n_dist)
    return libmesh_make_unique<const Hermite>(n_dist);

  const BoostNormalDistribution * bn_dist = dynamic_cast<const BoostNormalDistribution *>(dist);
  if (bn_dist)
    return libmesh_make_unique<const Hermite>(bn_dist);

  ::mooseError("Polynomials for '", dist->type(), "' distributions have not been implemented.");
  return nullptr;
}

Real
Polynomial::compute(const unsigned int /*order*/, const Real /*x*/, const bool /*normalize*/) const
{
  ::mooseError("Polynomial type has not been implemented.");
  return 0;
}

Real
Polynomial::productIntegral(const std::vector<unsigned int> order) const
{
  const unsigned int nprod = order.size();

  if (nprod == 1)
    return (order[0] == 0 ? 1.0 : 0.0);
  else if (nprod == 2)
    return (order[0] == order[1] ? innerProduct(order[0]) : 0.0);

  unsigned int poly_order = std::accumulate(order.begin(), order.end(), 0);
  unsigned int quad_order = (poly_order % 2 == 0 ? poly_order : poly_order - 1) / 2;

  std::vector<Real> xq;
  std::vector<Real> wq;
  quadrature(quad_order, xq, wq);

  Real val = 0.0;
  for (unsigned int q = 0; q < xq.size(); ++q)
  {
    Real prod = wq[q];
    for (unsigned int i = 0; i < nprod; ++i)
      prod *= compute(order[i], xq[q], false);
    val += prod;
  }

  return val / std::accumulate(wq.begin(), wq.end(), 0.0);
}

Legendre::Legendre(const UniformDistribution * dist)
  : Polynomial(),
    _lower_bound(dist->getParamTempl<Real>("lower_bound")),
    _upper_bound(dist->getParamTempl<Real>("upper_bound"))
{
}

Real
Legendre::compute(const unsigned int order, const Real x, const bool normalize) const
{
  if ((x < _lower_bound) || (x > _upper_bound))
    ::mooseError("The requested polynomial point is outside of distribution bounds.");

  Real val = legendre(order, x, _lower_bound, _upper_bound);
  if (normalize)
    val /= innerProduct(order);

  return val;
}

void
Legendre::quadrature(const unsigned int order,
                     std::vector<Real> & points,
                     std::vector<Real> & weights) const
{
  gauss_legendre(order, points, weights, _lower_bound, _upper_bound);
}

Real
legendre(const unsigned int order, const Real x, const Real lower_bound, const Real upper_bound)
{
  Real xref = 2 / (upper_bound - lower_bound) * (x - (upper_bound + lower_bound) / 2);
#ifdef LIBMESH_HAVE_EXTERNAL_BOOST
  // Using Legendre polynomials from boost library (if available)
  // https://www.boost.org/doc/libs/1_46_1/libs/math/doc/sf_and_dist/html/math_toolkit/special/sf_poly/legendre.html
  return boost::math::legendre_p(order, xref);
#else
  // Using explicit expression of polynomial coefficients:
  // P_n(x) = 1/2^n\sum_{k=1}^{floor(n/2)} (-1)^k * nchoosek(n, k) * nchoosek(2n-2k, n) * x^(n-2k)
  // https://en.wikipedia.org/wiki/Legendre_polynomials
  if (order < 16)
  {
    Real val = 0;
    for (unsigned int k = 0; k <= (order % 2 == 0 ? order / 2 : (order - 1) / 2); ++k)
    {
      Real coeff =
          Real(Utility::binomial(order, k)) * Real(Utility::binomial(2 * order - 2 * k, order));
      Real sgn = (k % 2 == 0 ? 1.0 : -1.0);
      unsigned int ord = order - 2 * k;
      val += sgn * coeff * pow(xref, ord);
    }
    return val / pow(2.0, order);
  }
  else
    return ((2.0 * (Real)order - 1.0) * xref * legendre(order - 1, xref) -
            ((Real)order - 1.0) * legendre(order - 2, xref)) /
           (Real)order;
#endif
}

Hermite::Hermite(const NormalDistribution * dist)
  : Polynomial(),
    _mu(dist->getParamTempl<Real>("mean")),
    _sig(dist->getParamTempl<Real>("standard_deviation"))
{
}

Hermite::Hermite(const BoostNormalDistribution * dist)
  : Polynomial(),
    _mu(dist->getParamTempl<Real>("mean")),
    _sig(dist->getParamTempl<Real>("standard_deviation"))
{
}

Real
Hermite::compute(const unsigned int order, const Real x, const bool normalize) const
{
  Real val = hermite(order, x, _mu, _sig);
  if (normalize)
    val /= innerProduct(order);
  return val;
}

void
Hermite::quadrature(const unsigned int order,
                    std::vector<Real> & points,
                    std::vector<Real> & weights) const
{
  gauss_hermite(order, points, weights, _mu, _sig);
}

Real
hermite(const unsigned int order, const Real x, const Real mu, const Real sig)
{
  Real xref = (x - mu) / sig;
#ifdef LIBMESH_HAVE_EXTERNAL_BOOST
  // Using Hermite polynomials from boost library (if available)
  // https://www.boost.org/doc/libs/1_46_1/libs/math/doc/sf_and_dist/html/math_toolkit/special/sf_poly/hermite.html
  // Need to do some modification since boost does physicists hermite polynomials:
  // H_n^prob(x) = 2^(-n/2)H_n^phys(x / sqrt(2))
  xref /= M_SQRT2; // 1 / sqrt(2)
  Real val = boost::math::hermite(order, xref);
  val /= pow(M_SQRT2, order); // 2^(-order / 2)
  return val;
#else
  // Using explicit expression of polynomial coefficients:
  // H_n(x) = n!\sum_{m=1}^{floor(n/2)} (-1)^m / (m!(n-2m)!) * x^(n-2m) / 2^m
  // https://en.wikipedia.org/wiki/Hermite_polynomials
  if (order < 13)
  {
    Real val = 0;
    for (unsigned int m = 0; m <= (order % 2 == 0 ? order / 2 : (order - 1) / 2); ++m)
    {
      Real sgn = (m % 2 == 0 ? 1.0 : -1.0);
      Real coeff =
          1.0 / Real(Utility::factorial(m) * Utility::factorial(order - 2 * m)) / pow(2.0, m);
      unsigned int ord = order - 2 * m;
      val += sgn * coeff * pow(xref, ord);
    }
    return val * Utility::factorial(order);
  }
  else
    return xref * hermite(order - 1, xref) - ((Real)order - 1.0) * hermite(order - 2, xref);
#endif
}

void
gauss_legendre(const unsigned int order,
               std::vector<Real> & points,
               std::vector<Real> & weights,
               const Real lower_bound,
               const Real upper_bound)
{
  unsigned int n = order + 1;
  points.resize(n);
  weights.resize(n);

  DenseMatrix<Real> mat(n, n);
  DenseVector<Real> lambda(n);
  DenseVector<Real> lambdai(n);
  DenseMatrix<Real> vec(n, n);
  for (unsigned int i = 1; i < n; ++i)
  {
    mat(i, i - 1) = (Real)i / std::sqrt(((2. * (Real)i - 1.) * (2. * (Real)i + 1.)));
    mat(i - 1, i) = mat(i, i - 1);
  }
  mat.evd_right(lambda, lambdai, vec);

  Real dx = (upper_bound - lower_bound) / 2.0;
  Real xav = (upper_bound + lower_bound) / 2.0;
  for (unsigned int i = 0; i < n; ++i)
  {
    points[i] = lambda(i) * dx + xav;
    weights[i] = vec(0, i) * vec(0, i);
  }
}

void
gauss_hermite(const unsigned int order,
              std::vector<Real> & points,
              std::vector<Real> & weights,
              const Real mu,
              const Real sig)
{
  // Number of points needed
  unsigned int n = order + 1;
  points.resize(n);
  weights.resize(n);

  DenseMatrix<Real> mat(n, n);
  DenseVector<Real> lambda(n);
  DenseVector<Real> lambdai(n);
  DenseMatrix<Real> vec(n, n);
  for (unsigned int i = 1; i < n; ++i)
  {
    mat(i, i - 1) = std::sqrt((Real)i);
    mat(i - 1, i) = mat(i, i - 1);
  }
  mat.evd_right(lambda, lambdai, vec);

  for (unsigned int i = 0; i < n; ++i)
  {
    points[i] = mu + lambda(i) * sig;
    weights[i] = vec(0, i) * vec(0, i);
  }
}

TensorGrid::TensorGrid(const std::vector<unsigned int> & npoints,
                       std::vector<std::unique_ptr<const Polynomial>> & poly)
{
  if (npoints.size() != poly.size())
    ::mooseError("List of number of 1D quadrature points must be same size as number of Polynomial "
                 "objects.");

  std::vector<std::vector<Real>> qpoints_1D(poly.size());
  std::vector<std::vector<Real>> qweights_1D(poly.size());
  for (unsigned int d = 0; d < poly.size(); ++d)
    poly[d]->quadrature(npoints[d] - 1, qpoints_1D[d], qweights_1D[d]);

  _quad = libmesh_make_unique<const StochasticTools::WeightedCartesianProduct<Real, Real>>(
      qpoints_1D, qweights_1D);
}

} // namespace PolynomialQuadrature
