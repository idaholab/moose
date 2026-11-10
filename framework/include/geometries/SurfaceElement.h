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

class Ball;
class LineSegment;

/// Base class for a single surface (boundary) element of a closed surface mesh.
///
/// Wraps a libMesh boundary element (an EDGE2 in 2D or a TRI3 in 3D) together
/// with its precomputed unit normal and provides the geometric queries needed by
/// point-containment and distance algorithms. intersect() and
/// computeBoundingBall() are pure virtual here so that callers can issue them
/// through a SurfaceElement reference without knowing the concrete geometry;
/// concrete derived classes (SurfaceEdge2, SurfaceTri3) also derive from a
/// GeometryBase primitive (LineSegment, Triangle) and forward to it.
class SurfaceElement
{
public:
  /**
   * Constructor takes a pointer to a surface element and its precomputed unit
   * normal. The normal is computed eagerly by derived classes (which know their
   * geometry) so the base can store it const and shared concurrent reads are safe.
   */
  SurfaceElement(const Elem * elem, const Point & normal);

  /// Virtual destructor to ensure proper cleanup in derived classes
  virtual ~SurfaceElement() = default;

  /// Getter for the underlying element
  const Elem & elem() const { return *_elem; }

  /// Getter for the normal vector
  const Point & normal() const { return _normal; }

  /**
   * Check if the given line segment intersects this surface element.
   * Implemented by derived classes by forwarding to their geometry primitive.
   */
  virtual bool intersect(const LineSegment & line_segment) const = 0;

  /// Getter of expected embedding solving mesh dimension
  /// Because the surface element is a face of the embedding mesh, its dimension is
  /// one less than the dimension of the embedding mesh.
  unsigned int expectedEmbeddingMeshDim() const { return _elem->dim() + 1; }

  /**
   * Compute a bounding ball for this surface element.
   * Implemented by derived classes by forwarding to their geometry primitive.
   */
  virtual Ball computeBoundingBall() const = 0;

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

private:
  /// Pointer to the libMesh element representing this surface face. Derived
  /// classes access it through elem() rather than this field directly.
  const Elem * _elem;
  /// Unit normal vector of the surface element.
  const Point _normal;
};
