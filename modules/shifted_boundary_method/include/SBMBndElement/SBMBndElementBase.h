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

enum class IntersectionType
{
  NO_INTERSECT = 0,
  INTERSECT = 1,
  ON_GEOMETRY = 2
};

class Ball;
class LineSegment;

/// Abstract base class for SBM boundary elements
class SBMBndElementBase
{
public:
  /// Constructor takes a pointer to a boundary element
  SBMBndElementBase(const Elem * elem);

  /// Virtual destructor to ensure proper cleanup in derived classes
  virtual ~SBMBndElementBase() = default;

  /// Getter for the underlying element
  const Elem & elem() const { return *_elem; }

  /// Getter for the normal vector
  const Point & normal();

  /**
   * Check if the given line segment intersects this boundary element.
   * Delegates to the underlying geometry via dynamic_cast.
   */
  bool intersect(const LineSegment & line_segment) const;

  /// Getter of expected embedding solving mesh dimension
  /// Because the boundary element is a face of the embedding mesh, its dimension is
  /// one less than the dimension of the embedding mesh.
  unsigned int expectedEmbeddingMeshDim() const { return _elem->dim() + 1; }

  /**
   * Compute the distance vector from an arbitrary point to this boundary element.
   * Default: normal-based distance vector. Override in derived class if needed.
   * (a) First calculation of the normal-based distance and the projection point (b) If the
   * projection point is outside the element, return the distance to the closest vertex.
   */
  virtual Point distanceFrom(const Point & pt) const;

  /**
   * Compute a bounding ball for this boundary element.
   * Delegates to the underlying geometry via dynamic_cast.
   */
  Ball computeBoundingBall() const;

  /**
   * Compute the length of the element bounding-box diagonal projected onto
   * a plane orthogonal to a given direction.
   *
   * The projection plane is defined only by its normal direction; the
   * specific location (offset) of the plane is irrelevant for this operation.
   * Equivalently, this function removes the component of the bounding-box
   * diagonal along the given normal direction and returns the norm of the
   * remaining tangential component.
   *
   * @param normal_dir: A direction vector defining the plane normal.
   * @return The length of the longest diagonal of the projected rectangle.
   */
  Real getProjectedBoundingBoxDiagonal(const Point & normal_dir) const;

protected:
  ///< Pointer to the libMesh element representing this boundary face
  const Elem * _elem;
  ///< Normal vector of the boundary element
  mutable Point _normal;

  /**
   * Return the normal vector of the boundary element.
   * This is a pure virtual function and must be implemented in derived classes.
   */
  virtual const Point computeNormal() const = 0;

private:
  /// Checking whether the normal vector has already computed
  mutable bool _is_normal_initialized = false;

  /// Ensure the normal vector is initialized before use
  void ensureNormalInitialized() const;

  /// flip the normal direction of the boundary element
  void flipNormal() { _normal *= -1; };

  friend class SBMSurfaceMeshBuilder;
};
