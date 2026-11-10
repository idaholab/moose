//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurfaceElement.h"

#include "libmesh/bounding_box.h"

#include <memory>
#include <vector>

/**
 * A group of surface elements wrapped for point-containment / distance queries.
 *
 * This is the single place where a libMesh surface mesh (or a subset of its
 * elements) is turned into SurfaceElement wrappers. It owns the wrappers, the
 * matching per-element centroids, and the global axis-aligned bounding box (AABB)
 * of the group. The AABB is method-independent: it is the plain union of the
 * elements' loose bounding boxes with no algorithm-specific inflation.
 *
 * Construct via the factories:
 *   - fromMesh(mesh)      : wrap every active element of a whole surface mesh.
 *   - fromElements(elems) : wrap a caller-provided subset of elements
 *                           (e.g. one subdomain group).
 *
 * The referenced elements must outlive this object.
 */
class SurfaceElementSet
{
public:
  /// Build a set from every active element of a surface mesh.
  static SurfaceElementSet fromMesh(const MeshBase & mesh);

  /// Build a set from a caller-provided subset of surface elements.
  static SurfaceElementSet fromElements(const std::vector<const Elem *> & elems);

  /// Owned surface-element wrappers, one per input element (input order preserved).
  const std::vector<std::unique_ptr<SurfaceElement>> & elements() const { return _elements; }

  /// Per-element centroids (vertex averages), aligned index-for-index with elements().
  const std::vector<Point> & centroids() const { return _centroids; }

  /// Global AABB of the group (plain union of element loose bounding boxes).
  const libMesh::BoundingBox & boundingBox() const { return _bounding_box; }

  /// Number of surface elements in the group.
  std::size_t size() const { return _elements.size(); }

private:
  /// Built only through the factories.
  SurfaceElementSet() = default;

  /// Validate, wrap, and append a single element, growing centroids and the AABB.
  /// All elements must share one supported type (EDGE2 or TRI3).
  void addElement(const Elem * elem);

  std::vector<std::unique_ptr<SurfaceElement>> _elements;
  std::vector<Point> _centroids;
  libMesh::BoundingBox _bounding_box;
};
