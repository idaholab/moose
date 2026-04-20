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

#include <iosfwd>
#include <memory>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Utility for reading a watertight STL manifold and querying point containment.
 *
 * The class owns a triangle soup parsed from an STL file, validates that the soup forms a closed
 * 2-manifold, re-orients the triangles consistently, builds a lightweight acceleration structure,
 * and finally exposes `contains()` for point-in-solid queries.
 *
 * Geometry is transformed in the exact order requested by the generator API:
 *
 * 1. scale
 * 2. rotation
 * 3. translation
 *
 * This object is intentionally self-contained because the parsing, topological validation, and
 * geometric classification logic are tightly coupled.
 */
class STLManifold
{
public:
  /**
   * Build a manifold representation from an STL file.
   *
   * @param file_name STL file to parse. ASCII and binary STL are both supported.
   * @param scale Per-axis scaling applied before rotation and translation.
   * @param rotation Extrinsic Euler angles in degrees.
   * @param translation Translation applied after scaling and rotation.
   * @param surface_tolerance Absolute tolerance used for manifold validation and near-surface
   * classification. This should be chosen relative to the STL length scale and expected vertex
   * noise from the export pipeline.
   */
  STLManifold(const std::string & file_name,
              const RealVectorValue & scale,
              const RealVectorValue & rotation,
              const RealVectorValue & translation,
              const Real surface_tolerance);

  /**
   * @return True if the point is inside the manifold or lies on the surface.
   */
  bool contains(const Point & point) const;

  /**
   * @return The manifold bounding box.
   */
  const libMesh::BoundingBox & boundingBox() const;

  /**
   * @return The number of triangles in the loaded manifold.
   */
  std::size_t numTriangles() const;

private:
  /**
   * Minimal geometric record for one STL triangle.
   *
   * We cache a bounding box per triangle because it is used repeatedly in both the near-surface
   * test and the positive-x intersection-counting pass used for inside/outside classification.
   */
  struct STLTriangle
  {
    /// Construct the triangle and its bounding box from transformed vertices.
    STLTriangle(const Point & vertex0, const Point & vertex1, const Point & vertex2);

    /// First transformed vertex.
    Point v0;

    /// Second transformed vertex.
    Point v1;

    /// Third transformed vertex.
    Point v2;

    /// Axis-aligned bounds of the transformed triangle.
    libMesh::BoundingBox bbox;
  };

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

  /// Parse the STL file and dispatch to the ASCII or binary reader.
  void parse(const std::string & file_name,
             const RealVectorValue & scale,
             const RealVectorValue & rotation,
             const RealVectorValue & translation);

  /// Parse an ASCII STL stream.
  void parseASCII(std::istream & input,
                  const RealVectorValue & scale,
                  const RealVectorValue & rotation,
                  const RealVectorValue & translation);

  /// Parse a binary STL stream.
  void parseBinary(std::istream & input,
                   const RealVectorValue & scale,
                   const RealVectorValue & rotation,
                   const RealVectorValue & translation);

  /// Complete post-parse validation and acceleration-structure setup.
  void finalize();

  /// Verify manifoldness and orient each connected surface component consistently.
  void validateAndOrient();

  /// Build the yz-plane lookup grid used to accelerate +x ray queries.
  void buildCandidateGrid();

  /// Transform one triangle and append it to the manifold triangle list.
  void addTriangle(const Point & v0,
                   const Point & v1,
                   const Point & v2,
                   const RealVectorValue & scale,
                   const RealTensorValue & rotation_matrix,
                   const Point & translation);

  /// Cheap global bounding-box rejection for containment queries.
  bool pointInsideBoundingBox(const Point & point) const;

  /// Detect whether a query point lies on or extremely near the manifold surface.
  bool pointOnSurface(const Point & point) const;

  /// Intersect a positive x-direction ray with a single triangle.
  RayIntersection rayIntersectsTriangle(const Point & point, const STLTriangle & tri) const;

  /// Robust fallback containment query based on accumulated solid angle.
  bool containsBySolidAngle(const Point & point) const;

  /// Get the subset of triangles whose yz extents may intersect the query ray.
  std::vector<std::size_t> rayCandidates(const Point & point) const;

  /// Squared distance from a point to a triangle.
  Real pointTriangleDistanceSq(const Point & point, const STLTriangle & tri) const;

  /// Squared distance from a point to a line segment.
  Real pointSegmentDistanceSq(const Point & point, const Point & a, const Point & b) const;

  /// Signed solid angle subtended by one oriented triangle at the query point.
  Real solidAngle(const Point & point, const STLTriangle & tri) const;

  /// All transformed triangles parsed from the STL file.
  std::vector<STLTriangle> _triangles;

  /// Global bounding box of the transformed manifold.
  std::unique_ptr<libMesh::BoundingBox> _bounding_box;

  /// Connected components of the oriented triangle graph, used by the solid-angle fallback.
  std::vector<std::vector<std::size_t>> _components;

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
  std::unordered_map<std::uint64_t, std::vector<std::size_t>> _ray_grid;

  /// Original STL file name, retained for error messages.
  const std::string _file_name;

  /// Absolute tolerance used throughout validation and geometric classification.
  const Real _surface_tolerance;
};
