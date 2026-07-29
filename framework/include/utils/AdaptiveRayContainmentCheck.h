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
#include "libmesh/point.h"
#include "SurfaceElement.h"
#include "SurfaceSide.h"
#include "KDTree.h"
#include "OrientedBoundingBox.h"
#include "RayDirectionOptions.h"
#include "MooseError.h"

#include <array>
#include <optional>

class Ball;

/// Ray-casting point-in-solid engine over a closed surface mesh represented as a
/// collection of SurfaceElement wrappers. The shooting direction follows one of two
/// explicit policies (RayDirectionMode):
///  - AUTO_PCA: the engine auto-selects a robust direction via PCA, builds an oriented
///    bounding box (OBB), and may use a parity-tie fallback along other variance axes.
///  - USER_SPECIFIED: the caller's direction is used exactly. There is no PCA, no OBB,
///    and no fallback; the engine uses a global axis-aligned bounding box (AABB) and, on
///    a genuinely ambiguous (grazing/tangent) query, errors rather than silently changing
///    direction. The direction is validated only to be finite, non-zero, and in-plane for
///    a 2D surface.
class AdaptiveRayContainmentCheck final
{
public:
  AdaptiveRayContainmentCheck(const std::vector<std::unique_ptr<SurfaceElement>> & bd_elements,
                              const std::vector<Point> & centroids,
                              const RayDirectionOptions & ray_options,
                              const Real eps_on_surface = libMesh::TOLERANCE,
                              const int leaf_max_size = 10,
                              const FileName & obb_file_name = "",
                              const FileName & ray_file_name = "",
                              const libMesh::Parallel::Communicator * comm = nullptr);

  /// Main function: Determine if a point is inside the geometry
  SurfaceSide sideness(const Point & p) const;

  /// The resolved ray direction actually used for shooting: the (normalized) user
  /// direction for a user-selected ray, or the PCA-selected direction for an auto ray.
  const Point & rayDirection() const { return _ray_direction; }

private:
  /// @brief  The centroid of the boundary elements' node points
  /// (prepare inside this class).
  Point _centroid_nodal_points;

  /// @brief Projected centroids of the elements in the boundary mesh.
  /// (prepare inside this class)
  /// This is used to construct the KDTree in this struct for fast searching.
  std::vector<Point> _projected_centroids;

  /// The KDTree is constructed using the projected centroids of the elements in the boundary mesh.
  /// (prepare inside this class)
  std::unique_ptr<KDTree> _kd_tree;

  /// The maximum diagonal length of the projected bounding box from the boundary elements.
  Real _max_projected_diag_length;

  /// pass into the constructor for the surface elements
  const std::vector<std::unique_ptr<SurfaceElement>> & _bd_elements;

  /// pass into the constructor for the surface element centroids
  const std::vector<Point> & _centroids;

  /// The dimension of the embedding mesh.
  int _dim = -1;

  /// The number of elements in the boundary mesh.
  std::size_t _num_elements = 0;

  /// Ray shooting direction
  Point _ray_direction;

  /// Epsilon value for checking if a point is on the surface of the geometry
  Real _eps_on_surface;

  /// Configures KDTree leaf node size for performance tuning.
  int _leaf_max_size;

  /// The file name for the OBB
  FileName _obb_file_name;

  /// The file name for the ray
  FileName _ray_file_name;

  /// Communicator used only for writing the debug OBB/ray mesh files.
  /// Null when no debug output is requested.
  const libMesh::Parallel::Communicator * _comm = nullptr;

  /// The oriented bounding box (OBB).
  OrientedBoundingBox _obb_bounds;

  /// The bounding box AABB.
  BoundingBox _bounds;

  /// Whether the ray direction is auto-selected via PCA (true) or user-selected (false).
  /// Set once in the constructor from the RayDirectionOptions mode. Only an auto ray runs
  /// PCA and may have its direction chosen automatically.
  bool _auto_ray_direction = false;

  /// When the ray direction is auto-selected (PCA) we build an Oriented Bounding Box (OBB);
  /// a user-selected ray uses only the global Axis-Aligned Bounding Box (AABB) instead.
  bool _build_obb = false;

  /// The origin of the plane used to ensure that every projected point is correctly aligned and lies on the same plane.
  /// This is for fast k-d tree searching to select the candidate elements to check the intersection.
  Point _plane_origin = Point(0.0, 0.0, 0.0);

  /// max variance vector
  Point _max_variance_vector;

  /// second max variance vector
  Point _second_variance_vector;

  /// min variance vector (only used for 3D)
  Point _min_variance_vector;

  /// Ray-element intersection (e.g., ray-line for 2D, ray-triangle for 3D)
  bool rayIntersectGeometry(const Point & ray_start,
                            const Point & ray_end,
                            const SurfaceElement * elem) const;

  /// Check if point is outside global bounding box
  bool isOutsideBoundingBox(const Point & query_point) const;

  /// Check if element center is outside ray bounding box
  bool isOutsideRayBBox(const Point & orig, const Point & dir, const Ball & ball) const;

  /// Check if element center is outside ray bounding circle/sphere
  bool isOutsideBoundingRegion(const Point & orig, const Point & dir, const Ball & ball) const;

  /// Compute the global bounding box of all boundary elements
  BoundingBox computeGlobalBoundingBox();

  // Perform Principal Component Analysis (PCA) using Singular Value Decomposition (SVD)
  // to compute the principal directions:
  // - _max_variance_vector: Direction with the largest variance (first principal component).
  // - _second_variance_vector: Direction with the second-largest variance (second principal
  // component).
  // - _min_variance_vector: Direction with the smallest variance (typically the surface normal).
  void preparePCASVD();

  /// Finalizes the ray direction and the matching bounding box. For an auto ray it sets
  /// _ray_direction to the PCA-selected direction and requests the OBB; for a user-selected
  /// ray it retains the user's direction and builds the global AABB.
  void initializeRayDirection();

  /// Determine sideness from a pair of opposite rays. A conflicting parity returns std::nullopt
  /// so the caller can apply the selected ambiguity policy.
  std::optional<SurfaceSide> sidenessFromRayPair(const Point & p,
                                                 const std::array<Point, 2> & ray_starts) const;

  /// True if `p` lies on the surface (within `_eps_on_surface`), i.e. some candidate element
  /// contains it. This is a property of `p` alone, independent of any ray direction.
  bool isOnSurface(const Point & p) const;

  /// Count how many times the segment from `ray_start` to `ray_end` crosses the surface. The
  /// primary direction uses KD-tree candidates; PCA fallback directions scan all elements because
  /// the KD-tree projection is aligned only with the primary direction. Callers must rule out the
  /// on-surface case (via isOnSurface) before interpreting the count.
  int countCrossings(const Point & ray_start,
                     const Point & ray_end,
                     const bool use_primary_direction = true) const;

  /// Shared traversal for the 2D and 3D crossing counts: walk the candidate elements (KD-tree
  /// candidates for the primary direction, all elements otherwise), apply the ray-bbox and
  /// bounding-region pruning, and count those for which is_crossing(surface) is true. The
  /// per-dimension crossing test is supplied by the caller so the pruning loop lives in exactly one
  /// place. Defined in the .C: only the two in-file callers instantiate it.
  template <typename CrossingTest>
  int countFilteredCrossings(const Point & ray_start,
                             const Point & ray_end,
                             const bool use_primary_direction,
                             CrossingTest is_crossing) const;

  /// 2D crossing count using a half-open side-based crossing rule: an edge is counted when its two
  /// endpoints lie on strictly opposite sides of the ray line (a vertex or collinear edge exactly on
  /// the line is thus counted consistently, with no tolerance) and the crossing is on the ray_start
  /// side of the query point. See countCrossings() for the parameters.
  int countCrossings2D(const Point & ray_start,
                       const Point & ray_end,
                       const bool use_primary_direction) const;

  /// Ray start strictly outside the global AABB along `unit_direction`, for any direction.
  /// The 8 AABB corners are projected onto the direction to find the box extent, and the
  /// start is placed just past the far side (a scale-aware padding), moving only the
  /// distance needed. `inverted` shoots from the opposite side. `unit_direction` must be
  /// normalized.
  Point
  rayStartOutsideAABB(const Point & point, const Point & unit_direction, const bool inverted) const;

  /**
   * Computes the starting point of an OBB-based ray (auto/PCA policy) for a given query point.
   * The point is projected onto the OBB face normal to `ray_direction`, selected by `obb_axis`,
   * and displaced along that same direction so the ray originates outside the geometry.
   */
  Point rayStartOutsideOBB(const Point & point,
                           const Point & ray_direction,
                           const unsigned int obb_axis,
                           const bool inverted = false) const;

  /// Orthogonally project `point_to_project` onto the plane defined by `plane_point`
  /// and unit normal `plane_normal`. `plane_normal` is assumed to be a unit vector.
  Point projectPointOntoPlane(const Point & point_to_project,
                              const Point & plane_point,
                              const Point & plane_normal) const;

  /// Constructs an oriented bounding box (OBB) using the results of PCA and the KD-tree.
  /// During this process, it also finds the maximum projected diagonal length.
  void buildObbKdtreeAndMaxProjectedDiagonal(const Real expand_box_length);

  /// Use the kd-tree to collect candidate element IDs to check intersections.
  std::vector<unsigned int> collectCandidateElementIDs(const Point & query_point) const;
};
