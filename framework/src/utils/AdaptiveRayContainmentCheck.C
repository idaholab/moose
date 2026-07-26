//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdaptiveRayContainmentCheck.h"
#include "LineSegment.h"
#include "Ball.h"
#include "libmesh/plane.h"
#include "libmesh/utility.h"

#include <cmath>
#include <limits>

AdaptiveRayContainmentCheck::AdaptiveRayContainmentCheck(
    const std::vector<std::unique_ptr<SurfaceElement>> & bd_elements,
    const std::vector<Point> & centroids,
    const RayDirectionOptions & ray_options,
    const Real eps_on_surface,
    const int leaf_max_size,
    const FileName & obb_file_name,
    const FileName & ray_file_name,
    const libMesh::Parallel::Communicator * comm)
  : _bd_elements(bd_elements),
    _centroids(centroids),
    _ray_direction(ray_options.direction),
    _eps_on_surface(eps_on_surface),
    _leaf_max_size(leaf_max_size),
    _obb_file_name(obb_file_name),
    _ray_file_name(ray_file_name),
    _comm(comm),
    _plane_origin(Point(0.0, 0.0, 0.0))
{
  mooseAssert(
      !_bd_elements.empty(),
      "AdaptiveRayContainmentCheck: boundary elements should not be empty or uninitialized.");
  _num_elements = _bd_elements.size();
  _dim = _bd_elements[0]->expectedEmbeddingMeshDim();

  _auto_ray_direction = (ray_options.mode == RayDirectionMode::AUTO_PCA);

  if (!_auto_ray_direction)
  {
    // USER_SPECIFIED policy: use the direction exactly. Validate only that it is usable, then
    // normalize. Degeneracy (a ray grazing a vertex/edge or tangent to the surface) is the
    // user's responsibility; the engine never auto-corrects or switches the direction.
    for (const auto i : make_range(3u))
      if (!std::isfinite(_ray_direction(i)))
        mooseError(
            "AdaptiveRayContainmentCheck: a user-selected ray_direction must be finite; got ",
            _ray_direction,
            ".");
    if (MooseUtils::absoluteFuzzyEqual(_ray_direction.norm(), 0.0))
      mooseError("AdaptiveRayContainmentCheck: a user-selected ray_direction must be non-zero.");
    if (_dim == 2 && !MooseUtils::absoluteFuzzyEqual(_ray_direction(2), 0.0))
      mooseError("AdaptiveRayContainmentCheck: a user-selected ray_direction for a 2D surface must "
                 "lie in the mesh plane (its z component must be zero); got ",
                 _ray_direction,
                 ".");
    _ray_direction = _ray_direction.unit();
  }

  // PCA is only used to auto-select the direction; a user-selected ray never runs it.
  if (_auto_ray_direction)
    preparePCASVD();
  buildObbKdtreeAndMaxProjectedDiagonal(
      1e-2 /*safe protect: expanded box length in each direction and both sides*/);
}

SurfaceSide
AdaptiveRayContainmentCheck::sideness(const Point & p) const
{
  if (isOutsideBoundingBox(p))
    return SurfaceSide::OUTSIDE;

  const std::array<Point, 2> ray_starts =
      _auto_ray_direction ? std::array<Point, 2>{generateRayStart(p), generateRayStart(p, true)}
                          : std::array<Point, 2>{rayStartOutsideAABB(p, _ray_direction, false),
                                                 rayStartOutsideAABB(p, _ray_direction, true)};

  if (const auto side = sidenessFromRayPair(p, ray_starts))
    return *side;

  if (!_auto_ray_direction)
  {
    std::ostringstream oss;
    oss << p;
    mooseError("AdaptiveRayContainmentCheck: the user-selected ray_direction ",
               _ray_direction,
               " gives an ambiguous (grazing or tangent) intersection at point ",
               oss.str(),
               "; choose a different ray_direction or use the auto (pca_ray) method.");
  }

  // The primary rays disagreed on parity. Probe the remaining PCA variance directions: if any
  // probe ray escapes without crossing the surface, the point is outside. Otherwise the query is
  // undecidable.
  const std::vector<Point> axis_dirs =
      (_dim == 3) ? std::vector<Point>{_second_variance_vector, _max_variance_vector}
                  : std::vector<Point>{_max_variance_vector};

  for (const auto idx : index_range(axis_dirs))
  {
    const auto & dir = axis_dirs[idx];
    const std::array<Point, 2> probe_starts = {generateRayStart(p, false /*inverted*/, idx),
                                               generateRayStart(p, true /*inverted*/, idx)};

    for (const auto i : make_range(2))
    {
      // p is never on the surface here (the primary pair already returned ON if it were), so the
      // point_on_surface flag stays false and the probe only uses the crossing count.
      bool point_on_surface = false;
      if (countCrossings(probe_starts[i], dir, p, point_on_surface) == 0)
        return SurfaceSide::OUTSIDE;
    }
  }

  std::ostringstream oss;
  oss << p;
  mooseError("AdaptiveRayContainmentCheck: No decision could be made for point " + oss.str());
}

std::optional<SurfaceSide>
AdaptiveRayContainmentCheck::sidenessFromRayPair(const Point & p,
                                                 const std::array<Point, 2> & ray_starts) const
{
  std::array<int, 2> counts = {0, 0};

  // Shoot the two (opposite) rays and count intersections with the boundary elements.
  for (const auto i : make_range(2))
  {
    bool point_on_surface = false;
    counts[i] = countCrossings(ray_starts[i], p - ray_starts[i], p, point_on_surface);

    if (point_on_surface)
      return SurfaceSide::ON;

    // A ray that never crosses the closed surface proves the point is outside.
    if (counts[i] == 0)
      return SurfaceSide::OUTSIDE;
  }

  // Consistent parity gives a definite decision; conflicting parity is undecided (nullopt) and
  // left to the caller's policy.
  if ((counts[0] % 2) == (counts[1] % 2))
    return (counts[0] % 2 == 1) ? SurfaceSide::INSIDE : SurfaceSide::OUTSIDE;
  return std::nullopt;
}

int
AdaptiveRayContainmentCheck::countCrossings(const Point & ray_start,
                                            const Point & bbox_dir,
                                            const Point & p,
                                            bool & point_on_surface) const
{
  point_on_surface = false;
  int count = 0;
  for (const auto & elemID : collectCandidateElementIDs(p))
  {
    const auto & elem = _bd_elements[elemID].get();
    const auto ball = computeBoundingBall(elem);

    if (isOutsideRayBBox(ray_start, bbox_dir, ball))
      continue;
    if (isOutsideBoundingRegion(ray_start, bbox_dir, ball))
      continue;

    if (elem->elem().contains_point(p, _eps_on_surface))
    {
      point_on_surface = true;
      return count;
    }

    if (rayIntersectGeometry(ray_start, p, elem))
      count++;
  }
  return count;
}

bool
AdaptiveRayContainmentCheck::rayIntersectGeometry(const Point & ray_start,
                                                  const Point & ray_end,
                                                  const SurfaceElement * elem) const
{
  LineSegment ray_segment(ray_start, ray_end);
  return elem->intersect(ray_segment);
}

Ball
AdaptiveRayContainmentCheck::computeBoundingBall(const SurfaceElement * elem) const
{
  return elem->computeBoundingBall();
}

bool
AdaptiveRayContainmentCheck::isOutsideBoundingBox(const Point & query_point) const
{
  return (_build_obb) ? !_obb_bounds.contains(query_point, _eps_on_surface)
                      : !_bounds.contains_point(query_point);
}

bool
AdaptiveRayContainmentCheck::isOutsideRayBBox(const Point & orig,
                                              const Point & dir,
                                              const Ball & ball) const
{
  Point lb, ub;
  const auto & center = ball.center();
  const Real radius = ball.radius();

  for (const auto i : make_range(_dim))
  {
    lb(i) = std::min(orig(i), orig(i) + dir(i)) - radius;
    ub(i) = std::max(orig(i), orig(i) + dir(i)) + radius;
  }

  for (const auto i : make_range(_dim))
  {
    if (center(i) < lb(i) || center(i) > ub(i))
      return true;
  }

  return false;
}

bool
AdaptiveRayContainmentCheck::isOutsideBoundingRegion(const Point & orig,
                                                     const Point & dir,
                                                     const Ball & ball) const
{
  const auto & center = ball.center();
  const auto radius = ball.radius();

  const auto w = center - orig;

  Real b = (w * dir) / (dir * dir);
  Point Pb = orig + b * dir;

  Real distance_squared = 0.0;
  for (const auto i : make_range(_dim))
    distance_squared += Utility::pow<2>(Pb(i) - center(i));

  return (distance_squared > radius * radius);
}

BoundingBox
AdaptiveRayContainmentCheck::computeGlobalBoundingBox()
{
  _bounds_ready = true;
  const auto & first_elem = _bd_elements[0]->elem();
  BoundingBox bbox = first_elem.loose_bounding_box();

  for (const auto & bd_elem : _bd_elements)
    bbox.union_with(bd_elem->elem().loose_bounding_box());

  const Real eps = _eps_on_surface;
  Point min_pt = bbox.min();
  Point max_pt = bbox.max();

  for (const auto d : make_range(3u))
  {
    min_pt(d) -= eps;
    max_pt(d) += eps;
  }

  return BoundingBox(min_pt, max_pt);
}

const Point
AdaptiveRayContainmentCheck::generateRayStart(const Point & point,
                                              const bool inverted,
                                              const int number_to_larger_variance) const
{
  mooseAssert(_build_obb,
              "AdaptiveRayContainmentCheck::generateRayStart: OBB-based ray start is only used by "
              "the auto (PCA) policy.");

  const Real SAFE_FACTOR = 1.1;

  const Real last_axis_length = _obb_bounds.getAxisLength(_dim - 1 - number_to_larger_variance);
  const Real half_axis_length = last_axis_length / 2.0;

  Point projection_plane_corner;
  Real direction_multiplier;

  if (_obb_bounds.getProjectedLength(point, _dim - 1 - number_to_larger_variance) <
      half_axis_length)
  {
    projection_plane_corner =
        inverted ? _obb_bounds.getMaximalCorner() : _obb_bounds.getMinimalCorner();
    direction_multiplier = inverted ? -1.0 : 1.0;
  }
  else
  {
    projection_plane_corner =
        inverted ? _obb_bounds.getMinimalCorner() : _obb_bounds.getMaximalCorner();
    direction_multiplier = inverted ? 1.0 : -1.0;
  }

  // Select the plane normal corresponding to number_to_larger_variance: 0 is the
  // ray direction, _dim - 1 the second-variance axis (3D only), and _dim the
  // maximum-variance axis.
  Point plane_normal;
  if (number_to_larger_variance == 0)
    plane_normal = _ray_direction;
  else if (_dim - number_to_larger_variance == 0)
    plane_normal = _max_variance_vector;
  else if (_dim - number_to_larger_variance == 1 /* for 3D only*/)
    plane_normal = _second_variance_vector;
  else
    mooseError("AdaptiveRayContainmentCheck::generateRayStart: invalid "
               "number_to_larger_variance ",
               number_to_larger_variance,
               " for dimension ",
               _dim,
               "; expected 0, ",
               _dim - 1,
               ", or ",
               _dim,
               ".");

  Point starting_point = projectPointOntoPlane(point, projection_plane_corner, plane_normal);
  return starting_point - (SAFE_FACTOR * last_axis_length * direction_multiplier) * _ray_direction;
}

///  Finalize the ray direction (auto -> PCA, user -> as given) and its bounding box.
void
AdaptiveRayContainmentCheck::initializeRayDirection()
{
  if (_auto_ray_direction)
  {
    // Auto ray: adopt the PCA-selected direction and use the oriented bounding box.
    _ray_direction = (_dim == 3) ? _min_variance_vector : _second_variance_vector;
    _build_obb = true;
  }
  else
    // User-selected axis-aligned ray: keep the user's (normalized) direction and use a
    // global axis-aligned bounding box.
    _bounds = computeGlobalBoundingBox();
}

Point
AdaptiveRayContainmentCheck::rayStartOutsideAABB(const Point & point,
                                                 const Point & unit_direction,
                                                 const bool inverted) const
{
  // Project the 8 AABB corners onto the direction to find the box's extent along it. The ray
  // start is then placed just past the far side, so it is provably outside the box while moving
  // only the distance needed (a full-diagonal displacement would make unnecessarily long rays).
  const Point & lo = _bounds.min();
  const Point & hi = _bounds.max();

  Real min_projection = std::numeric_limits<Real>::max();
  Real max_projection = std::numeric_limits<Real>::lowest();
  for (const auto c : make_range(8u))
  {
    const Point corner(
        (c & 1u) ? hi(0) : lo(0), (c & 2u) ? hi(1) : lo(1), (c & 4u) ? hi(2) : lo(2));
    const Real projection = corner * unit_direction;
    min_projection = std::min(min_projection, projection);
    max_projection = std::max(max_projection, projection);
  }

  // Scale-aware padding so the start never lands exactly on the box boundary.
  const Real padding = _eps_on_surface + 1e-2 * (max_projection - min_projection);
  const Real target = inverted ? max_projection + padding : min_projection - padding;
  return point + (target - point * unit_direction) * unit_direction;
}

Point
AdaptiveRayContainmentCheck::projectPointOntoPlane(const Point & point_to_project,
                                                   const Point & plane_point,
                                                   const Point & plane_normal) const
{
  // Delegate to libMesh::Plane::closest_point, which returns the orthogonal
  // projection of the point onto the plane. `plane_normal` is assumed to be a
  // unit vector (closest_point does not normalize it).
  return libMesh::Plane(plane_point, plane_normal).closest_point(point_to_project);
}

void
AdaptiveRayContainmentCheck::preparePCASVD()
{
  Point centroid_sum;

  std::vector<Point> nodal_points;
  for (const auto & elem : _bd_elements)
  {
    const auto & e = elem->elem();
    for (const auto i : make_range(e.n_nodes()))
    {
      const Node * node = e.node_ptr(i);
      mooseAssert(node, "Node pointer is null!");
      nodal_points.push_back(*node);
      centroid_sum += *node;
    }
  }

  const unsigned int N = nodal_points.size();
  mooseAssert(N >= 3, "At least 3 points required");

  // (a) Compute the centroid
  _centroid_nodal_points = centroid_sum / static_cast<Real>(N);

  // (b) Build the mean-centered matrix X (N x 3)
  DenseMatrix<Real> X(N, 3);
  for (const auto i : make_range(N))
  {
    const Point d = nodal_points[i] - _centroid_nodal_points;
    X(i, 0) = d(0);
    X(i, 1) = d(1);
    X(i, 2) = d(2);
  }

  // (c) Perform SVD: X = U * sigma * V^T
  DenseVector<Real> sigma;
  DenseMatrix<Real> U, VT;
  X.svd(sigma, U, VT); // VT is 3x3, each row is a principal direction

  // (d) Extract principal directions
  _max_variance_vector = Point(VT(0, 0), VT(0, 1), VT(0, 2));    // max variance
  _second_variance_vector = Point(VT(1, 0), VT(1, 1), VT(1, 2)); // second largest variance
  _min_variance_vector = Point(VT(2, 0), VT(2, 1), VT(2, 2));    // min variance

  // (e) normalize them to be safe (unit() returns a normalized copy)
  _max_variance_vector = _max_variance_vector.unit();
  _second_variance_vector = _second_variance_vector.unit();
  _min_variance_vector = _min_variance_vector.unit();

  // (f) Canonicalize the sign of each principal direction. SVD singular vectors
  // are only defined up to sign, and LAPACK can return opposite signs on
  // different platforms or versions. Because the auto-selected ray direction is
  // one of these vectors, an unstable sign makes the in-out classification of
  // borderline elements non-reproducible across platforms. Fix a deterministic
  // convention: make the largest-magnitude component positive (ties broken by
  // the lowest index).
  auto canonicalize_sign = [](Point & v)
  {
    unsigned int i_max = 0;
    for (const auto i : make_range(1, 3))
      if (std::abs(v(i)) > std::abs(v(i_max)))
        i_max = i;
    if (v(i_max) < 0.0)
      v *= -1.0;
  };
  canonicalize_sign(_max_variance_vector);
  canonicalize_sign(_second_variance_vector);
  canonicalize_sign(_min_variance_vector);

  mooseAssert(
      MooseUtils::absoluteFuzzyEqual(_max_variance_vector * _second_variance_vector, 0.0) &&
          MooseUtils::absoluteFuzzyEqual(_max_variance_vector * _min_variance_vector, 0.0) &&
          MooseUtils::absoluteFuzzyEqual(_second_variance_vector * _min_variance_vector, 0.0),
      "Principal directions are not orthogonal.");
}

void
AdaptiveRayContainmentCheck::buildObbKdtreeAndMaxProjectedDiagonal(const Real expand_box_length)
{
  if (!_centroids.empty())
    mooseAssert(_centroids.size() >= 3, "Need at least three points.");

  // (a) Prepare KD-tree data (optional) and track PCA-space extents

  // Initialize the ray direction if not set
  initializeRayDirection();

  // Global min / max along the three PCA axes
  Real u_min = std::numeric_limits<Real>::max();
  Real u_max = std::numeric_limits<Real>::lowest();
  Real v_min = u_min, v_max = u_max;
  Real w_min = u_min, w_max = u_max;

  _projected_centroids.resize(_num_elements);

  _max_projected_diag_length = 0.0;

  for (const auto i : make_range(_num_elements))
  {
    // Per-element KD-tree data
    {
      const Point & pt =
          (!_centroids.empty() ? _centroids[i] : _bd_elements[i]->elem().vertex_average());

      _projected_centroids[i] = projectPointOntoPlane(pt, _plane_origin, _ray_direction);

      if (_dim == 2) // flatten Z in 2-D mode
        _projected_centroids[i](2) = 0.0;

      _max_projected_diag_length =
          std::max(_max_projected_diag_length,
                   _bd_elements[i]->getProjectedBoundingBoxDiagonal(_ray_direction));
    }

    if (_build_obb)
    {
      // Update PCA-space bounding box
      const Elem & e = _bd_elements[i]->elem();

      for (const auto j : make_range(e.n_nodes()))
      {
        const Point d = *(e.node_ptr(j)) - _centroid_nodal_points;
        const Real u = d * _max_variance_vector;
        const Real v = d * _second_variance_vector;
        const Real w = (_dim == 3) ? d * _min_variance_vector : 0.0;

        u_min = std::min(u_min, u);
        u_max = std::max(u_max, u);
        v_min = std::min(v_min, v);
        v_max = std::max(v_max, v);
        if (_dim == 3)
        {
          w_min = std::min(w_min, w);
          w_max = std::max(w_max, w);
        }
      }
    }
  }

  if (_build_obb)
  {
    // (b) Build the oriented bounding box (OBB)
    const Point min_corner =
        _centroid_nodal_points + u_min * _max_variance_vector + v_min * _second_variance_vector +
        ((_dim == 3) ? w_min * _min_variance_vector : Point()) -
        expand_box_length * _max_variance_vector - expand_box_length * _second_variance_vector -
        ((_dim == 3) ? expand_box_length * _min_variance_vector : Point());

    std::vector<std::pair<Point, Point>> axis_pairs{
        {min_corner,
         min_corner + (u_max - u_min) * _max_variance_vector +
             2 * expand_box_length *
                 _max_variance_vector /*2 because we subtract 1 in min_corner*/},

        {min_corner,
         min_corner + (v_max - v_min) * _second_variance_vector +
             2 * expand_box_length * _second_variance_vector}};

    if (_dim == 3)
      axis_pairs.emplace_back(min_corner,
                              min_corner + (w_max - w_min) * _min_variance_vector +
                                  2 * expand_box_length * _min_variance_vector);

    _obb_bounds = OrientedBoundingBox(axis_pairs);

    if (_obb_file_name != "")
    {
      if (!_comm)
        mooseError("A communicator is required to write the OBB mesh file '", _obb_file_name, "'.");
      std::filesystem::path obb_path(_obb_file_name.c_str());
      _obb_bounds.writeMesh(obb_path, *_comm);
    }

    if (_ray_file_name != "")
    {
      if (!_comm)
        mooseError("A communicator is required to write the ray mesh file '", _ray_file_name, "'.");
      std::filesystem::path ray_path(_ray_file_name.c_str());
      _obb_bounds.writeRayAlongShortestAxis(ray_path, *_comm);
    }
  }

  // (c) Finalise KD-tree
  _kd_tree = std::make_unique<KDTree>(_projected_centroids, _leaf_max_size);
}

std::vector<unsigned int>
AdaptiveRayContainmentCheck::collectCandidateElementIDs(const Point & query_point) const
{
  std::vector<unsigned int> elem_ids;

  // KD-tree radius search in projected PCA space
  Point proj = projectPointOntoPlane(query_point, _plane_origin, _ray_direction);
  if (_dim == 2)
    proj(2) = 0.0; // flatten Z for 2-D

  std::vector<nanoflann::ResultItem<std::size_t, Real>> matches;
  _kd_tree->radiusSearch(proj, _max_projected_diag_length, matches);

  elem_ids.reserve(matches.size());
  for (const auto & m : matches)
    elem_ids.push_back(static_cast<unsigned int>(m.first));

  return elem_ids;
}
