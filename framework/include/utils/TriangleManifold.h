//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include "libmesh/bounding_box.h"
#include "libmesh/face_tri3.h"
#include "libmesh/mesh_tet_interface.h"

#include <iosfwd>
#include <memory>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class TriangleManifold
{
public:
  TriangleManifold(MeshBase & mesh, const Real surface_tolerance);

  /**
   * @return True if the point is inside the manifold or lies on the surface.
   */
  bool contains(const Point & point) const;

  /**
   * @return The manifold bounding box.
   */
  const libMesh::BoundingBox & boundingBox() const { return _bounding_box; };

  /**
   * @return The number of triangles in the loaded manifold.
   */
  std::size_t numTriangles() const { return _mesh.n_active_elem(); }

private:
  /**
   * Result of intersecting the positive x-direction ray with a triangle.
   *
   * `Ambiguous` is returned when the hit is too close to an edge, vertex, or the ray origin. In
   * that case we abandon parity counting and fall back to the more expensive but robust solid-angle
   * test.
   */
  enum class RayIntersection
  {
    Miss,
    Hit,
    Ambiguous
  };

  /// Complete post-parse validation and acceleration-structure setup.
  void finalize();

  /// Build the yz-plane lookup grid used to accelerate +x ray queries.
  void buildCandidateGrid();

  /// Cheap global bounding-box rejection for containment queries.
  bool pointInsideBoundingBox(const Point & point) const;

  /// Detect whether a query point lies on or extremely near the manifold surface.
  bool pointOnSurface(const Point & point) const;

  /// Intersect a positive x-direction ray with a single triangle.
  RayIntersection rayIntersectsTriangle(const Point & point, const libMesh::Elem & tri) const;

  /// Robust fallback containment query based on accumulated solid angle.
  bool containsBySolidAngle(const Point & point) const;

  /// Get the subset of triangles whose yz extents may intersect the query ray.
  std::vector<dof_id_type> rayCandidates(const Point & point) const;

  /// Number of yz-grid bins in the y direction.
  std::size_t _num_y_cells = 1;

  /// Number of yz-grid bins in the z direction.
  std::size_t _num_z_cells = 1;

  /// Minimum global y coordinate used to map query points into yz-grid bins.
  Real _y_min = 0.0;

  /// Minimum global z coordinate used to map query points into yz-grid bins.
  Real _z_min = 0.0;

  /// Width of one yz-grid cell in the y direction.
  Real _y_cell_size = 1.0;

  /// Width of one yz-grid cell in the z direction.
  Real _z_cell_size = 1.0;

  /// Lookup from packed yz-grid cell index to triangles that could intersect the +x query ray.
  std::unordered_map<dof_id_type, std::vector<dof_id_type>> _ray_grid;

  MeshBase & _mesh;

  /// Absolute tolerance used throughout validation and geometric classification.
  const Real _surface_tolerance;

  /// Global bounding box of the transformed manifold.
  const libMesh::BoundingBox _bounding_box;

  // Pre-built point locator for fast close to surface check
  const std::unique_ptr<libMesh::PointLocatorBase> _point_locator;
};

class SurfaceChecker : public libMesh::MeshTetInterface
{
public:
  explicit SurfaceChecker(UnstructuredMesh & mesh) : libMesh::MeshTetInterface(mesh) {}
  void triangulate() override { mooseError("SurfaceChecker is not meant for triangulation."); }
  std::string improveAndValidate();
};
