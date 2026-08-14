//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurfaceSide.h"
#include "MooseTypes.h"

#include "libmesh/bounding_box.h"
#include "libmesh/parallel.h"

#include <memory>

namespace libMesh
{
class MeshBase;
}
class SurfaceElementSet;
class AdaptiveRayContainmentCheck;
class TriangleManifold;

/**
 * Unified wrapper (facade) over the point-containment backends.
 *
 * Selects and constructs exactly one backend and exposes a single result type
 * (SurfaceSide). It adds no geometry logic of its own beyond backend selection
 * and the uniform contains() mapping; the ray/solid-angle math lives entirely in
 * the backends (AdaptiveRayContainmentCheck and TriangleManifold).
 */
class PointContainmentClassifier
{
public:
  /// Backend algorithm used for point-containment queries.
  enum class Method
  {
    /// AdaptiveRayContainmentCheck with a PCA-selected ray (default SBM behavior).
    PCA_RAY,
    /// AdaptiveRayContainmentCheck with a user-supplied ray_direction, used exactly as given
    /// (no PCA, no auto-selection). Any finite non-zero direction is accepted, including oblique;
    /// for a 2D surface it must lie in the mesh plane.
    USER_SELECTED_RAY,
    /// TriangleManifold engine (fixed +x ray parity + solid-angle fallback). TRI3
    /// surfaces only.
    FIXED_X_RAY
  };

  /// Ray-backend tuning + debug output; ignored by FIXED_X_RAY (TriangleManifold).
  struct RayOptions
  {
    /// Direction used by USER_SELECTED_RAY; ignored by PCA_RAY.
    Point ray_direction;
    int leaf_max_size = 10;
    FileName obb_file_name = "";
    FileName ray_file_name = "";
    const libMesh::Parallel::Communicator * comm = nullptr; // debug output only
  };

  /**
   * Builds PCA_RAY with default ray options or FIXED_X_RAY without unused options.
   * USER_SELECTED_RAY requires the overload accepting explicit RayOptions.
   */
  PointContainmentClassifier(libMesh::MeshBase & mesh,
                             const SurfaceElementSet * set,
                             Method method,
                             Real tolerance);

  /**
   * Builds the selected backend with explicit ray options. `mesh` is builder-owned
   * and must outlive this object (serialized for FIXED_X_RAY / TriangleManifold).
   * `set` is required for PCA_RAY/USER_SELECTED_RAY (its elements/centroids are
   * referenced by the backend and must also outlive this object) and ignored (may
   * be null) for FIXED_X_RAY.
   */
  PointContainmentClassifier(libMesh::MeshBase & mesh,
                             const SurfaceElementSet * set,
                             Method method,
                             Real tolerance,
                             const RayOptions & options);

  ~PointContainmentClassifier();

  /// Classify a query point relative to the closed surface.
  SurfaceGeometry::SurfaceSide sideness(const Point & point) const;

  /// Convenience API aligned with TriangleManifold: INSIDE and ON both map to true.
  bool contains(const Point & point) const
  {
    return sideness(point) != SurfaceGeometry::SurfaceSide::OUTSIDE;
  }

  /// Method-independent global axis-aligned bounding box of the surface.
  const libMesh::BoundingBox & boundingBox() const { return _bounding_box; }

  /// Number of surface elements (triangles) backing the classifier.
  std::size_t numElements() const { return _num_elements; }

  /// The resolved ray direction of the ray-casting backend (user-selected direction for
  /// USER_SELECTED_RAY, PCA-selected direction for PCA_RAY). Errors for FIXED_X_RAY, which
  /// has no such backend.
  Point rayDirection() const;

private:
  const Method _method;

  /// One of the two backends is constructed; the other stays null.
  std::unique_ptr<AdaptiveRayContainmentCheck> _pca; ///< PCA_RAY or USER_SELECTED_RAY
  std::unique_ptr<TriangleManifold> _tri;            ///< FIXED_X_RAY

  /// Cached method-independent AABB and element count for the accessors.
  libMesh::BoundingBox _bounding_box;
  std::size_t _num_elements = 0;
};
