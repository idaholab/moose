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
#include "DataIO.h"

// For computing legendre quadrature
#include "libmesh/dense_matrix_impl.h"

// For quickly computing polynomials
#ifdef LIBMESH_HAVE_EXTERNAL_BOOST
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
#include <boost/math/special_functions/legendre.hpp>
#include <boost/math/special_functions/hermite.hpp>
#pragma GCC diagnostic pop
#endif

#include <cmath>
#include <memory>

namespace PolynomialQuadrature
{

std::unique_ptr<const Polynomial>
makePolynomial(const Distribution * dist)
{
  const Uniform * u_dist = dynamic_cast<const Uniform *>(dist);
  if (u_dist)
    return std::make_unique<const Legendre>(dist->getParam<Real>("lower_bound"),
                                            dist->getParam<Real>("upper_bound"));

  const Normal * n_dist = dynamic_cast<const Normal *>(dist);
  if (n_dist)
    return std::make_unique<const Hermite>(dist->getParam<Real>("mean"),
                                           dist->getParam<Real>("standard_deviation"));

  ::mooseError("Polynomials for '", dist->type(), "' distributions have not been implemented.");
  return nullptr;
}

void
Polynomial::store(std::ostream & /*stream*/, void * /*context*/) const
{
  // Cannot be pure virtual because for dataLoad operations the base class must be constructed
  ::mooseError("Polynomial child class must override this method.");
}

void
Polynomial::store(nlohmann::json & /*json*/) const
{
  // Cannot be pure virtual because for dataLoad operations the base class must be constructed
  ::mooseError("Polynomial child class must override this method.");
}

Real
Polynomial::compute(const unsigned int /*order*/, const Real /*x*/, const bool /*normalize*/) const
{
  ::mooseError("Polynomial type has not been implemented.");
  return 0;
}

Real
Polynomial::computeDerivative(const unsigned int /*order*/,
                              const Real /*x*/,
                              const unsigned int /*n*/) const
{
  ::mooseError("Polynomial type has not been implemented.");
  return 0;
}

void
Polynomial::clenshawQuadrature(const unsigned int /*order*/,
                               std::vector<Real> & /*points*/,
                               std::vector<Real> & /*weights*/) const
{
  ::mooseError("Clenshaw quadrature has not been implemented for this polynomial type.");
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
  gaussQuadrature(quad_order, xq, wq);

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

Legendre::Legendre(const Real lower_bound, const Real upper_bound)
  : Polynomial(), _lower_bound(lower_bound), _upper_bound(upper_bound)
{
}

void
Legendre::store(std::ostream & stream, void * context) const
{
  std::string type = "Legendre";
  dataStore(stream, type, context);
  dataStore(stream, _lower_bound, context);
  dataStore(stream, _upper_bound, context);
}

void
Legendre::store(nlohmann::json & json) const
{
  json["type"] = "Legendre";
  json["lower_bound"] = _lower_bound;
  json["upper_bound"] = _upper_bound;
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

Real
Legendre::computeDerivative(const unsigned int order, const Real x, const unsigned int m) const
{
  if ((x < _lower_bound) || (x > _upper_bound))
    ::mooseError("The requested polynomial point is outside of distribution bounds.");

  Real xref = 2.0 / (_upper_bound - _lower_bound) * (x - (_upper_bound + _lower_bound) / 2.0);
  Real Jac = pow(2.0 / (_upper_bound - _lower_bound), m);
  return Jac * computeDerivativeRef(order, xref, m);
}

Real
Legendre::computeDerivativeRef(const unsigned int order,
                               const Real xref,
                               const unsigned int m) const
{
  if (m == 0)
    return legendre(order, xref);
  else if (m > order)
    return 0.0;
  else if (order == 1)
    return 1.0;
  else
    return static_cast<Real>(order + m - 1) * computeDerivativeRef(order - 1, xref, m - 1) +
           xref * computeDerivativeRef(order - 1, xref, m);
}

void
Legendre::gaussQuadrature(const unsigned int order,
                          std::vector<Real> & points,
                          std::vector<Real> & weights) const
{
  gauss_legendre(order, points, weights, _lower_bound, _upper_bound);
}

void
Legendre::clenshawQuadrature(const unsigned int order,
                             std::vector<Real> & points,
                             std::vector<Real> & weights) const
{
  clenshaw_curtis(order, points, weights);
  Real dx = (_upper_bound - _lower_bound) / 2.0;
  Real xav = (_upper_bound + _lower_bound) / 2.0;
  for (unsigned int i = 0; i < points.size(); ++i)
    points[i] = points[i] * dx + xav;
}

Real
Legendre::innerProduct(const unsigned int order) const
{
  return 1. / (2. * static_cast<Real>(order) + 1.);
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
  {
    Real ord = order;
    return ((2.0 * ord - 1.0) * xref * legendre(order - 1, xref) -
            (ord - 1.0) * legendre(order - 2, xref)) /
           ord;
  }
#endif
}

Hermite::Hermite(const Real mu, const Real sig) : Polynomial(), _mu(mu), _sig(sig) {}

Real
Hermite::innerProduct(const unsigned int order) const
{
  return (Real)Utility::factorial(order);
}

void
Hermite::store(std::ostream & stream, void * context) const
{
  std::string type = "Hermite";
  dataStore(stream, type, context);
  dataStore(stream, _mu, context);
  dataStore(stream, _sig, context);
}

void
Hermite::store(nlohmann::json & json) const
{
  json["type"] = "Hermite";
  json["mu"] = _mu;
  json["sig"] = _sig;
}

Real
Hermite::compute(const unsigned int order, const Real x, const bool normalize) const
{
  Real val = hermite(order, x, _mu, _sig);
  if (normalize)
    val /= innerProduct(order);
  return val;
}

Real
Hermite::computeDerivative(const unsigned int order, const Real x, const unsigned int m) const
{
  if (m > order)
    return 0.0;

  Real xref = (x - _mu) / _sig;
  Real val = hermite(order - m, xref);
  for (unsigned int i = 0; i < m; ++i)
    val *= static_cast<Real>(order - i) / _sig;

  return val;
}

void
Hermite::gaussQuadrature(const unsigned int order,
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
    return xref * hermite(order - 1, xref) -
           (static_cast<Real>(order) - 1.0) * hermite(order - 2, xref);
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
    Real ri = i;
    mat(i, i - 1) = ri / std::sqrt(((2. * ri - 1.) * (2. * ri + 1.)));
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
    mat(i, i - 1) = std::sqrt(static_cast<Real>(i));
    mat(i - 1, i) = mat(i, i - 1);
  }
  mat.evd_right(lambda, lambdai, vec);

  for (unsigned int i = 0; i < n; ++i)
  {
    points[i] = mu + lambda(i) * sig;
    weights[i] = vec(0, i) * vec(0, i);
  }
}

void
clenshaw_curtis(const unsigned int order, std::vector<Real> & points, std::vector<Real> & weights)
{
  // Number of points needed
  unsigned int N = order + (order % 2);
  points.resize(N + 1);
  weights.resize(N + 1);

  if (N == 0)
  {
    points[0] = 0;
    weights[0] = 1;
    return;
  }

  std::vector<Real> dk(N / 2 + 1);
  for (unsigned int k = 0; k <= (N / 2); ++k)
    dk[k] = ((k == 0 || k == (N / 2)) ? 1.0 : 2.0) / (1.0 - 4.0 * (Real)k * (Real)k);

  for (unsigned int n = 0; n <= (N / 2); ++n)
  {
    Real theta = (Real)n * M_PI / ((Real)N);
    points[n] = -std::cos(theta);
    for (unsigned int k = 0; k <= (N / 2); ++k)
    {
      Real Dnk =
          ((n == 0 || n == (N / 2)) ? 0.5 : 1.0) * std::cos((Real)k * theta * 2.0) / ((Real)N);
      weights[n] += Dnk * dk[k];
    }
  }

  for (unsigned int n = 0; n < (N / 2); ++n)
  {
    points[N - n] = -points[n];
    weights[N - n] = weights[n];
  }
  weights[N / 2] *= 2.0;
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
    poly[d]->gaussQuadrature(npoints[d] - 1, qpoints_1D[d], qweights_1D[d]);

  _quad = std::make_unique<const StochasticTools::WeightedCartesianProduct<Real, Real>>(
      qpoints_1D, qweights_1D);
}

SmolyakGrid::SmolyakGrid(const unsigned int max_order,
                         std::vector<std::unique_ptr<const Polynomial>> & poly)
  : _ndim(poly.size())
{

  // Compute full tensor tuple
  std::vector<std::vector<unsigned int>> tuple_1d(_ndim);
  for (unsigned int d = 0; d < _ndim; ++d)
  {
    tuple_1d[d].resize(max_order);
    for (unsigned int i = 0; i < max_order; ++i)
      tuple_1d[d][i] = i;
  }
  StochasticTools::CartesianProduct<unsigned int> tensor_tuple(tuple_1d);

  _npoints.push_back(0);
  unsigned int maxq = max_order - 1;
  unsigned int minq = (max_order > _ndim ? max_order - _ndim : 0);
  for (std::size_t p = 0; p < tensor_tuple.numRows(); ++p)
  {
    std::vector<unsigned int> dorder = tensor_tuple.computeRow(p);
    unsigned int q = std::accumulate(dorder.begin(), dorder.end(), 0);
    if (q <= maxq && q >= minq)
    {
      int sgn = ((max_order - q - 1) % 2 == 0 ? 1 : -1);
      _coeff.push_back(sgn * Utility::binomial(_ndim - 1, _ndim + q - max_order));

      std::vector<std::vector<Real>> qpoints_1D(_ndim);
      std::vector<std::vector<Real>> qweights_1D(_ndim);
      for (unsigned int d = 0; d < poly.size(); ++d)
        poly[d]->gaussQuadrature(dorder[d], qpoints_1D[d], qweights_1D[d]);

      _quad.push_back(std::make_unique<const StochasticTools::WeightedCartesianProduct<Real, Real>>(
          qpoints_1D, qweights_1D));
      _npoints.push_back(_npoints.back() + _quad.back()->numRows());
    }
  }
}

std::vector<Real>
SmolyakGrid::quadraturePoint(const unsigned int n) const
{
  unsigned int ind = gridIndex(n);
  return _quad[ind]->computeRow(n - _npoints[ind]);
}

Real
SmolyakGrid::quadraturePoint(const unsigned int n, const unsigned int dim) const
{
  unsigned int ind = gridIndex(n);
  return _quad[ind]->computeValue(n - _npoints[ind], dim);
}

Real
SmolyakGrid::quadratureWeight(const unsigned int n) const
{
  unsigned int ind = gridIndex(n);
  return static_cast<Real>(_coeff[ind]) * _quad[ind]->computeWeight(n - _npoints[ind]);
}

unsigned int
SmolyakGrid::gridIndex(const unsigned int n) const
{
  for (unsigned int i = 0; i < _npoints.size() - 1; ++i)
    if (_npoints[i + 1] > n)
      return i;

  ::mooseError("Point index requested is greater than number of points.");

  return 0;
}

ClenshawCurtisGrid::ClenshawCurtisGrid(const unsigned int max_order,
                                       std::vector<std::unique_ptr<const Polynomial>> & poly)
  : _ndim(poly.size())
{
  // Compute full tensor tuple
  std::vector<std::vector<unsigned int>> tuple_1d(_ndim);
  for (unsigned int d = 0; d < _ndim; ++d)
  {
    tuple_1d[d].resize(max_order);
    for (unsigned int i = 0; i < max_order; ++i)
      tuple_1d[d][i] = i;
  }
  StochasticTools::CartesianProduct<unsigned int> tensor_tuple(tuple_1d);

  // Curtis clenshaw has a lot of nested points,
  // so it behooves us to avoid duplicate points
  std::map<std::vector<Real>, Real> quad_map;
  unsigned int maxq = max_order - 1;
  unsigned int minq = (max_order > _ndim ? max_order - _ndim : 0);
  for (std::size_t p = 0; p < tensor_tuple.numRows(); ++p)
  {
    std::vector<unsigned int> dorder = tensor_tuple.computeRow(p);
    unsigned int q = std::accumulate(dorder.begin(), dorder.end(), 0);
    if (q <= maxq && q >= minq)
    {
      int sgn = ((max_order - q - 1) % 2 == 0 ? 1 : -1);
      Real coeff = static_cast<Real>(sgn * Utility::binomial(_ndim - 1, _ndim + q - max_order));

      std::vector<std::vector<Real>> qpoints_1D(_ndim);
      std::vector<std::vector<Real>> qweights_1D(_ndim);
      for (unsigned int d = 0; d < poly.size(); ++d)
        poly[d]->clenshawQuadrature(dorder[d], qpoints_1D[d], qweights_1D[d]);

      StochasticTools::WeightedCartesianProduct<Real, Real> quad(qpoints_1D, qweights_1D);

      for (unsigned int i = 0; i < quad.numRows(); ++i)
        quad_map[quad.computeRow(i)] += coeff * quad.computeWeight(i);
    }
  }

  _quadrature.reserve(quad_map.size());
  for (const auto & it : quad_map)
    _quadrature.push_back(it);
}
} // namespace PolynomialQuadrature

template <>
void
dataStore(std::ostream & stream,
          std::unique_ptr<const PolynomialQuadrature::Polynomial> & ptr,
          void * context)
{
  ptr->store(stream, context);
}

template <>
void
dataLoad(std::istream & stream,
         std::unique_ptr<const PolynomialQuadrature::Polynomial> & ptr,
         void * context)
{
  std::string poly_type;
  dataLoad(stream, poly_type, context);
  if (poly_type == "Legendre")
  {
    Real lower_bound, upper_bound;
    dataLoad(stream, lower_bound, context);
    dataLoad(stream, upper_bound, context);
    ptr = std::make_unique<const PolynomialQuadrature::Legendre>(lower_bound, upper_bound);
  }
  else if (poly_type == "Hermite")
  {
    Real mean, stddev;
    dataLoad(stream, mean, context);
    dataLoad(stream, stddev, context);
    ptr = std::make_unique<const PolynomialQuadrature::Hermite>(mean, stddev);
  }
  else
    ::mooseError("Unknown Polynomaial type: ", poly_type);
}
