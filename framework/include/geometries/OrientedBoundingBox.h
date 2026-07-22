//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "MooseMesh.h"
#include "MooseError.h"

/**
 * @brief Oriented bounding box in 2 D or 3 D.
 *
 * An N-D (N = 2 or 3) oriented bounding box defined by one common origin,
 * N orthonormal directions, and a length along each direction.
 *
 * Construction is based on N "(min,max)" pairs that share a common min point:
 *   - axis_pairs[i].first  = shared origin
 *   - axis_pairs[i].second = far-end point on axis *i*
 *
 * Example (3-D): {{o, o+dx}, {o, o+dy}, {o, o+dz}}
 *
 * The sequence of pairs is usually supplied in the order main axis, secondary axis, then minor
 * axis (in other class).
 */
struct OrientedBoundingBox
{
  // --- Construction ---

  /// Default-constructs an *empty* box (zero dimension, no axes).
  OrientedBoundingBox();

  /**
   * @brief Build the box from a set of axis end-points.
   *
   * Each element in axis_pairs is a (origin, far-end) pair defining one axis.
   * The first element's first point is taken as the shared origin. The vectors
   *   v_i = (axis_pairs[i].second - origin)
   * are normalised to obtain an orthonormal basis; their norms become the edge
   * lengths. If the supplied axes are not mutually orthogonal, construction
   * fails with a run-time assertion.
   */
  explicit OrientedBoundingBox(const std::vector<std::pair<Point, Point>> & axis_pairs);

  // --- Queries / introspection ---

  /**
   * @brief Test whether a point lies inside or on the box.
   *
   * @param pt        Query point.
   * @param tolerance Fuzzy tolerance applied along each axis (default: libMesh::TOLERANCE).
   * @return `true` if the point projection along every axis falls in the range
   *         `[0, len_i]` within the specified tolerance.
   */
  bool contains(const Point & pt, const Real tolerance = libMesh::TOLERANCE) const;

  /// @return Geometric centroid of the box.
  Point centroid() const;

  /// @return Unit direction vector of axis i.
  Point getAxisDirection(unsigned int i) const;

  /// @return Length of axis i.
  Real getAxisLength(unsigned int i) const;

  /// @return The shared minimal corner of the box.
  Point getMinimalCorner() const;

  /// @return The shared maximal corner of the box.
  Point getMaximalCorner() const;

  // --- I/O helpers ---

  /// Print a summary (dimension, origin, axes).
  void print(std::ostream & os) const;

  /**
   * @brief Write the oriented box surface to an ASCII VTK file.
   *
   * - **3-D:** exports 6 `VTK_QUAD` faces (cell type 9)
   * - **2-D:** exports 4 `VTK_LINE` edges (cell type 3)
   */
  void writeVTK(const std::filesystem::path & path) const;

  /**
   * @brief Write a line-segment VTK representing a "ray" emanating from the box.
   *
   * (a) The ray originates at the centre of the face (2-D) or face-centre (3-D)
   *     orthogonal to the shortest axis.
   * (b) It is aligned with that shortest axis and its length equals the
   *     corresponding edge length.
   */
  void writeRayAlongShortestAxis(const std::filesystem::path & ray_path) const;

  /**
   * @brief Get the length of the projection of a point onto axis i.
   *
   * This computes the length of the projection of the point `pt` onto the
   * orthonormal basis vector `dirs[i]`, relative to the minimal corner.
   *
   * @param pt Query point.
   * @param i Axis index (0 for first, 1 for second, etc.).
   * @return Length of the projection along axis i.
   */
  Real getProjectedLength(const Point & pt, unsigned int i) const;

private:
  Point _minimal_corner;    ///< shared minimal corner
  Point _maximal_corner;    ///< shared maximal corner
  std::vector<Point> _dirs; ///< orthonormal basis vectors (size = _dim)
  std::vector<Real> _len;   ///< length along each basis (size = _dim)
  unsigned int _dim = 0u;   ///< spatial dimension (2 or 3)
};
