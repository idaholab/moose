//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayTracingAngularQuadrature.h"

// MOOSE includes
#include "MooseUtils.h"

// libMesh includes
#include "libmesh/dense_vector.h"

RayTracingAngularQuadrature::RayTracingAngularQuadrature(const unsigned int dim,
                                                         const unsigned int polar_order,
                                                         const unsigned int azimuthal_order,
                                                         const Real mu_min,
                                                         const Real mu_max)
  : _dim(dim),
    _polar_order(polar_order),
    _azimuthal_order(azimuthal_order),
    _mu_min(mu_min),
    _mu_max(mu_max)
{
  if (_polar_order == 0)
    mooseError("polar_order must be positive in RayTracingAngularQuadrature");
  if (_azimuthal_order == 0)
    mooseError("azimuthal_order must be positive in RayTracingAngularQuadrature");
  if (_mu_min >= _mu_max)
    mooseError("mu_min must be < mu_max in RayTracingAngularQuadrature");
  if (_mu_min < -1)
    mooseError("mu_min must be >= -1 in RayTracingAngularQuadrature");
  if (_mu_max > 1)
    mooseError("mu_max must be <= 1 in RayTracingAngularQuadrature");
  if (dim != 2 && dim != 3)
    mooseError("RayTracingAngularQuadrature only supports dimensions 2 and 3");

  // Build the quadrature
  build();

  // Default rotation is up
  rotate(libMesh::Point(0, 0, 1));
}

void
RayTracingAngularQuadrature::build()
{
  // Chebyshev quadrature on [0, 2\pi]
  std::vector<Real> chebyshev_x(_azimuthal_order);
  std::vector<Real> chebyshev_w(_azimuthal_order);
  chebyshev(_azimuthal_order, chebyshev_x, chebyshev_w);

  // Gauss-Legendre quadrature on [0, 1]
  std::vector<Real> gauss_legendre_x;
  std::vector<Real> gauss_legendre_w;
  gaussLegendre(_polar_order, gauss_legendre_x, gauss_legendre_w);

  _phi.resize(0);
  _mu.resize(0);
  _w.resize(0);

  // Build the product quadrature
  for (std::size_t i = 0; i < chebyshev_x.size(); ++i)
    for (std::size_t j = 0; j < gauss_legendre_x.size(); ++j)
    {
      // If 2D, throw away the downward mu
      if (_dim == 2 && gauss_legendre_x[j] < 0.5 - TOLERANCE * TOLERANCE)
        continue;

      _phi.push_back(chebyshev_x[i]);
      _mu.push_back(gauss_legendre_x[j] * (_mu_max - _mu_min) + _mu_min);
      _polar_sin.push_back(std::sin(std::acos(_mu.back())));

      const Real weight_factor =
          (_dim == 2 && !MooseUtils::absoluteFuzzyEqual(gauss_legendre_x[j], 0.5)) ? 2.0 : 1.0;
      _w.push_back(weight_factor * chebyshev_w[i] * gauss_legendre_w[j]);
    }
}

void
RayTracingAngularQuadrature::gaussLegendre(const unsigned int order,
                                           std::vector<Real> & x,
                                           std::vector<Real> & w)
{
  if (order == 0)
    mooseError("Order must be positive in gaussLegendre()");

  x.resize(order);
  w.resize(order);
  libMesh::DenseMatrix<Real> mat(order, order);
  libMesh::DenseVector<Real> lambda(order);
  libMesh::DenseVector<Real> lambdai(order);
  libMesh::DenseMatrix<Real> vec(order, order);

  for (unsigned int i = 1; i < order; ++i)
  {
    Real ri = i;
    mat(i, i - 1) = ri / std::sqrt(((2. * ri - 1.) * (2. * ri + 1.)));
    mat(i - 1, i) = mat(i, i - 1);
  }
  mat.evd_right(lambda, lambdai, vec);

  for (unsigned int i = 0; i < order; ++i)
  {
    x[i] = 0.5 * (lambda(i) + 1.0);
    w[i] = vec(0, i) * vec(0, i);
  }

  // Sort based on the points
  std::vector<std::size_t> sorted_indices(x.size());
  std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
  std::stable_sort(sorted_indices.begin(),
                   sorted_indices.end(),
                   [&x](size_t i1, size_t i2) { return x[i1] < x[i2]; });
  const auto x_copy = x;
  const auto w_copy = w;
  for (std::size_t i = 0; i < x.size(); ++i)
  {
    x[i] = x_copy[sorted_indices[i]];
    w[i] = w_copy[sorted_indices[i]];
  }
}

void
RayTracingAngularQuadrature::rotate(const libMesh::Point & rotation_direction)
{
  _current_rotation_direction = rotation_direction;

  libMesh::DenseMatrix<Real> rotation_matrix;
  rotationMatrix(rotation_direction.unit(), rotation_matrix);

  _current_directions.clear();
  _current_directions.reserve(_phi.size());

  _current_weights.clear();
  _current_weights.reserve(_w.size());

  _current_polar_sins.clear();
  _current_polar_sins.reserve(_polar_sin.size());

  libMesh::DenseVector<Real> omega(3);
  libMesh::DenseVector<Real> omega_p(3);
  Point direction;

  for (std::size_t q = 0; q < _phi.size(); ++q)
  {
    // Get direction as a DenseVector for rotation
    omega(0) = sqrt(1 - _mu[q] * _mu[q]) * cos(_phi[q]);
    omega(1) = sqrt(1 - _mu[q] * _mu[q]) * sin(_phi[q]);
    omega(2) = _mu[q];

    // Rotate and create the direction
    rotation_matrix.vector_mult(omega_p, omega);
    direction(0) = omega_p(0);
    direction(1) = omega_p(1);
    direction(2) = omega_p(2);

    // The index for the new direction in the "current" data
    // structures. By default - we add a new one. However,
    // in 2D we may have a duplicate projected direction
    // so we need to keep track of this in the case
    // that we are adding information to an already
    // existant direction
    std::size_t l = _current_directions.size();

    // If 2D, project to the xy plane and see if any other
    // projected directions are a duplicate
    if (_dim == 2)
    {
      direction(2) = 0;
      direction /= direction.norm();

      // If we loop through all current directions and find
      // no duplicates, this will set l back to _current_directions.size()
      for (l = 0; l < _current_directions.size(); ++l)
        if (_current_directions[l].absolute_fuzzy_equals(direction))
          break;
    }

    // If l is the current size of _current_directions, it means
    // that no duplicates were found and that we are inserting
    // a new direction
    if (l == _current_directions.size())
    {
      _current_directions.push_back(direction);
      _current_weights.resize(l + 1);
      _current_polar_sins.resize(l + 1);
    }
    else
      mooseAssert(_dim == 2, "Should only have duplicates in 2D");

    // Add to weights and sins for this direction
    _current_weights[l].push_back(_w[q]);
    _current_polar_sins[l].push_back(_polar_sin[q]);
  }
}

const libMesh::Point &
RayTracingAngularQuadrature::getDirection(const unsigned int l) const
{
  checkDirection(l);
  return _current_directions[l];
}

const std::vector<Real> &
RayTracingAngularQuadrature::getWeights(const unsigned int l) const
{
  checkDirection(l);
  return _current_weights[l];
}

Real
RayTracingAngularQuadrature::getTotalWeight(const unsigned int l) const
{
  checkDirection(l);
  return std::accumulate(_current_weights[l].begin(), _current_weights[l].end(), (Real)0);
}

const std::vector<Real> &
RayTracingAngularQuadrature::getPolarSins(const unsigned int l) const
{
  checkDirection(l);
  return _current_polar_sins[l];
}

std::size_t
RayTracingAngularQuadrature::numPolar(const unsigned int l) const
{
  checkDirection(l);
  if (_dim == 3)
    mooseAssert(_current_weights[l].size() == 1, "Should be 1 polar in 3D");
  return _current_polar_sins[l].size();
}

void
RayTracingAngularQuadrature::checkDirection(const unsigned int l) const
{
  if (!hasDirection(l))
    mooseError("RayTracingAngularQuadrature does not have direction ", l);
}

void
RayTracingAngularQuadrature::chebyshev(const unsigned int order,
                                       std::vector<Real> & x,
                                       std::vector<Real> & w)
{
  x.resize(order);
  w.resize(order);

  for (std::size_t i = 0; i < order; ++i)
  {
    x[i] = 2 * (Real)i * M_PI / (Real)order;
    w[i] = 2 * M_PI / (Real)order;
  }
}

void
RayTracingAngularQuadrature::rotationMatrix(const libMesh::Point & direction,
                                            libMesh::DenseMatrix<Real> & matrix)
{
  matrix.resize(3, 3);
  matrix.zero();

  // Create a local coordinate system around direction
  const libMesh::Point tx = orthonormalVector(direction);
  const libMesh::Point ty = direction.cross(tx);

  // Create the rotation matrix
  for (unsigned int j = 0; j < 3; ++j)
  {
    matrix(j, 0) = tx(j);
    matrix(j, 1) = ty(j);
    matrix(j, 2) = direction(j);
  }
}

libMesh::Point
RayTracingAngularQuadrature::orthonormalVector(const libMesh::Point & v)
{
  if (MooseUtils::absoluteFuzzyLessEqual(v.norm(), 0))
    ::mooseError("Vector v has norm close to 0 in orthonormalVector()");

  if (MooseUtils::absoluteFuzzyEqual(v(0), 0))
    return libMesh::Point(1, 0, 0);
  if (MooseUtils::absoluteFuzzyEqual(v(1), 0))
    return libMesh::Point(0, 1, 0);
  if (MooseUtils::absoluteFuzzyEqual(v(2), 0))
    return libMesh::Point(0, 0, 1);

  return libMesh::Point(-v(1), v(0), 0).unit();
}
