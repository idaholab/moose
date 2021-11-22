//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseTypes.h"

// libMesh includes
#include "libmesh/dense_matrix.h"
#include "libmesh/point.h"

class RayTracingAngularQuadrature
{
public:
  /**
   * Constructor.
   *
   * @param dim The mesh dimension
   * @param polar_order Order for the polar quadrature
   * @param azimuthal_order Azimuthal order for the quadrature
   * @param mu_min Minimum mu for the quadrature
   * @param mu_max Maximum mu for the quadrature
   */
  RayTracingAngularQuadrature(const unsigned int dim,
                              const unsigned int polar_order,
                              const unsigned int azimuthal_order,
                              const Real mu_min,
                              const Real mu_max);

  /**
   * Builds Gauss-Legendre quadrature on [0, 1] (symmetric about 0.5), with
   * weights that sum to 1.
   *
   * @param order The quadrature order
   * @param x Vector to fill the points into
   * @param w Vector to fill the weights into
   */
  static void gaussLegendre(const unsigned int order, std::vector<Real> & x, std::vector<Real> & w);

  /**
   * Builds Chebyshev quadrature on [0, 2\pi] with weights that sum to 2\pi.
   *
   * @param order The quadrature order
   * @param x Vector to fill the points into
   * @param w Vector to fill the weights into
   */
  static void chebyshev(const unsigned int order, std::vector<Real> & x, std::vector<Real> & w);

  /**
   * Builds the rotation matrix for direction \p direction into \p matrix.
   */
  static void rotationMatrix(const libMesh::Point & direction, libMesh::DenseMatrix<Real> & matrix);
  /**
   * Gets the vector that is orthonormal to \p v.
   */
  static libMesh::Point orthonormalVector(const libMesh::Point & v);

  /**
   * Rotates the quadrature to a given direction.
   */
  void rotate(const libMesh::Point & rotation_direction);

  /**
   * Get the direction associated with direction \p l.
   *
   * This direction will be rotated per currentRotationDirection(),
   * set by setRotation().
   */
  const libMesh::Point & getDirection(const unsigned int l) const;
  /**
   * Get the weights associated with the direction \p l.
   *
   * The weights across all directions sum to 2\pi.
   *
   * In the case of 2D, the 3D directions projected into the
   * 2D plane may overlap. In this case, we combine the directions
   * into a single direction with multiple weights. This is the
   * reason for the vector return.
   */
  const std::vector<Real> & getWeights(const unsigned int l) const;
  /**
   * Gets the total of the weights associated with the direction \p l.
   *
   * The weights across all directions sum to 2\pi.
   *
   * In the case of 2D, the 3D directions projected into the
   * 2D plane may overlap. In this case, we combine the directions
   * into a single direction with multiple weights. This is the
   * reason why a getter for the "total" weight for a single
   * direction exists.
   */
  Real getTotalWeight(const unsigned int l) const;
  /**
   * Gets the polar sins for the direction \p l.
   *
   * In the case of 2D, the 3D directions projected into the
   * 2D plane may overlap. In this case, we combine the directions
   * into a single direction with multiple polar sins. This is the
   * reason for the vector return.
   */
  const std::vector<Real> & getPolarSins(const unsigned int l) const;

  /**
   * Get the number of directions in the rotated and projected quadrature.
   *
   * Note that in the case of 2D, we overlap 3D directions projected
   * into the 2D plane. Therefore, in 2D, this value may be less
   * than the number of directions in the 3D quadrature.
   */
  std::size_t numDirections() const { return _current_directions.size(); }
  /**
   * Whether or not the angular quadrature has direction \p l.
   */
  bool hasDirection(const unsigned int l) const { return l < _current_directions.size(); }

  /**
   * Throws a MooseError if the angular quadrature does not have direction \p l.
   */
  void checkDirection(const unsigned int l) const;

  /**
   * The number of polar directions associated with the given direction.
   *
   * In 2D, projecting the 3D directions into the 2D plane may result
   * in directions that overlap, in which case we end up with one
   * direction and multiple polars for said direction.
   *
   * In 3D, this will always be 1.
   */
  std::size_t numPolar(const unsigned int l) const;

  /**
   * Get the polar order
   */
  unsigned int polarOrder() const { return _polar_order; }
  /**
   * Get the azimuthal order
   */
  unsigned int azimuthalOrder() const { return _azimuthal_order; }
  /**
   * Get the minimum mu
   */
  Real muMin() const { return _mu_min; }
  /**
   * Get the maximum mu
   */
  Real muMax() const { return _mu_max; }

  /**
   * Get the current rotation direction
   */
  const libMesh::Point & currentRotationDirection() const { return _current_rotation_direction; }

  /**
   * Get the quadrature dimension
   */
  unsigned int dim() const { return _dim; }

private:
  /**
   * Build the quadrature
   */
  void build();

  /// The dimension
  const unsigned int _dim;
  /// The polar order
  const unsigned int _polar_order;
  /// The azimuthal order
  const unsigned int _azimuthal_order;
  /// The minimum mu
  const Real _mu_min;
  /// The maximum mu
  const Real _mu_max;

  /// Quadrature phi
  std::vector<Real> _phi;
  /// Quadrature mu
  std::vector<Real> _mu;
  /// Quadrature polar sin
  std::vector<Real> _polar_sin;
  /// Quadrature combined weights
  std::vector<Real> _w;

  /**
   * The current quadrature information.
   *
   * We need "current" information because the quadrature
   * can be rotated and because in 2D, there are cases where
   * projected directions that are the same in the 2D plane
   * are combined together with multiple weights and polar
   * sins.
   */
  ///@{
  std::vector<libMesh::Point> _current_directions;
  std::vector<std::vector<Real>> _current_weights;
  std::vector<std::vector<Real>> _current_polar_sins;
  ///@}

  /// The current rotation direction
  libMesh::Point _current_rotation_direction;
};
