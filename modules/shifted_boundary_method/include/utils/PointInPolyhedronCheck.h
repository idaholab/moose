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
#include "SBMBndElementBase.h"
#include "KDTree.h"
#include "OrientedBoundingBox.h"
#include "MooseError.h"

class Ball;

/// The side of the surface where the point is located.
enum class SurfaceSide
{
  INSIDE,
  OUTSIDE,
  ON
};

struct PointInPolyhedronCheck final
{
public:
  PointInPolyhedronCheck(const std::vector<std::unique_ptr<SBMBndElementBase>> & bd_elements,
                         const std::vector<Point> & centroids,
                         const Point ray_direction,
                         bool brute_force_looping_all_bndelements = false,
                         const Real eps_on_surface = libMesh::TOLERANCE,
                         const int leaf_max_size = 10,
                         const FileName & obb_file_name = "",
                         const FileName & ray_file_name = "",
                         const libMesh::Parallel::Communicator * comm = nullptr);

  /// Main function: Determine if a point is inside the geometry
  SurfaceSide sideness(const Point & p);

private:
  /// Maximum direction and second maximum direction from PCA
  Point _max_dir, _second_max_dir;

  /// @brief  The boundary elements' node points
  /// (prepare inside this class).
  std::vector<Point> _nodal_points;

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

  /// pass into the constructor for the sbm boundary elements
  const std::vector<std::unique_ptr<SBMBndElementBase>> & _bd_elements;

  /// pass into the constructor for the sbm boundary centroids
  const std::vector<Point> & _centroids;

  /// The dimension of the embedding mesh.
  int _dim = -1;

  /// The number of elements in the boundary mesh.
  std::size_t _num_elements = 0;

  /// Ray shooting direction
  Point _ray_direction;

  /// Brute force means that we loop over all boundary elements
  /// to check if the point is inside.
  bool _brute_force_looping_all_bndelements;

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

  /// Whether the bounding box has been computed and is ready for use.
  bool _bounds_ready = false;

  /// If the ray x-dir, y-dir, or z-dir is set to 1.0, we just need to build the Axis-Aligned Bounding Box (AABB)
  /// and do not need to build Oriented Bounding Box (OBB)
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

  /// For bounding region checks (e.g., sphere/circle around element)
  Ball computeBoundingBall(const SBMBndElementBase * elem) const;

  /// Ray-element intersection (e.g., ray-line for 2D, ray-triangle for 3D)
  bool rayIntersectGeometry(const Point & ray_start,
                            const Point & ray_end,
                            const SBMBndElementBase * elem) const;

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

  /// Sets the ray direction for ray shooting. If the ray direction is explicitly provided by the user as a Point,
  /// this function will retain the user-defined direction.
  /// Otherwise, it will automatically determine the optimal ray direction based on the geometry.
  void initializeRayDirection();

  /// Returns true if _ray_direction is (approximately) one of the coordinate axes X, Y, or Z.
  /// Assumes _ray_direction has been normalized, so a unit vector with any component equal to 1.0
  /// is necessarily axis-aligned. A non-axis direction (including the default (0,0,0) sentinel)
  /// means the ray direction must be chosen automatically via PCA.
  bool rayDirectionIsAxisAligned() const;

  /**
   * Computes the starting point of a ray for a given query point.
   * The ray is cast in the orthogonal direction to the binning direction.
   * The starting point is initialized to the input point and then displaced
   * along the ray direction to ensure it starts outside the bounding box,
   * guaranteeing it originates outside the geometry.
   */
  const Point generateRayStart(const Point & point,
                               const bool inverted = false,
                               const std::optional<Point> & forced_ray_direction_XYZ = std::nullopt,
                               const int number_to_larger_variance = 0) const;

  /// Orthogonally project `point_to_project` onto the plane defined by `plane_point`
  /// and unit normal `plane_normal`. `plane_normal` is assumed to be a unit vector.
  Point projectPointOntoPlane(const Point & point_to_project,
                              const Point & plane_point,
                              const Point & plane_normal) const;

  /// Constructs an oriented bounding box (OBB) using the results of PCA and the KD-tree.
  /// During this process, it also finds the maximum projected diagonal length.
  void buildObbKdtreeAndMaxProjectedDiagonal(const Real expand_box_length);

  /// Depends on brute force or kd-tree to collect candidate element IDs to check intersections.
  std::vector<unsigned int> collectCandidateElementIDs(const Point & query_point) const;
};
