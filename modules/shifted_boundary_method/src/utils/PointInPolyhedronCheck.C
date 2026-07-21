//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointInPolyhedronCheck.h"
#include "LineSegment.h"
#include "Ball.h"

PointInPolyhedronCheck::PointInPolyhedronCheck(
    const std::vector<std::unique_ptr<SBMBndElementBase>> & bd_elements,
    const std::vector<Point> & centroids,
    const Point ray_direction,
    bool brute_force_looping_all_bndelements,
    const Real eps_on_surface,
    const int leaf_max_size,
    const FileName & obb_file_name,
    const FileName & ray_file_name)
  : _bd_elements(bd_elements),
    _centroids(centroids),
    _ray_direction(ray_direction),
    _brute_force_looping_all_bndelements(brute_force_looping_all_bndelements),
    _eps_on_surface(eps_on_surface),
    _leaf_max_size(leaf_max_size),
    _obb_file_name(obb_file_name),
    _ray_file_name(ray_file_name),
    _plane_origin(Point(0.0, 0.0, 0.0))
{
  mooseAssert(!_bd_elements.empty(),
              "PointInPolyhedronCheck: boundary elements should not be empty or uninitialized.");
  _num_elements = _bd_elements.size();
  _dim = _bd_elements[0]->expectedEmbeddingMeshDim();

  if (!MooseUtils::absoluteFuzzyEqual(_ray_direction(0), 1.0) or
      !MooseUtils::absoluteFuzzyEqual(_ray_direction(1), 1.0) or
      !MooseUtils::absoluteFuzzyEqual(_ray_direction(2), 1.0))
    preparePCASVD();
  buildObbKdtreeAndMaxProjectedDiagonal(
      1e-2 /*safe protect: expanded box length in each direction and both sides*/);
}

SurfaceSide
PointInPolyhedronCheck::sideness(const Point & p)
{
  if (isOutsideBoundingBox(p))
    return SurfaceSide::OUTSIDE;

  // --- (a) Define first two rays ---
  std::array<Point, 2> ray_starts = {generateRayStart(p), generateRayStart(p, true)};

  std::array<Point, 2> ray_dirs = {p - ray_starts[0], p - ray_starts[1]};

  std::array<int, 2> counts = {0, 0};

  for (int i = 0; i < 2; ++i)
  {
    for (const auto & elemID : collectCandidateElementIDs(p))
    {
      const auto & elem = _bd_elements[elemID].get();
      const auto ball = computeBoundingBall(elem);

      if (isOutsideRayBBox(ray_starts[i], ray_dirs[i], ball))
        continue;
      if (isOutsideBoundingRegion(ray_starts[i], ray_dirs[i], ball))
        continue;

      if (elem->elem().contains_point(p, _eps_on_surface))
        return SurfaceSide::ON;

      if (rayIntersectGeometry(ray_starts[i], p, elem))
        counts[i]++;
    }

    if (counts[i] == 0)
      return SurfaceSide::OUTSIDE;
  }

  // --- (b) Immediate decisions ---
  // If both counts are odd or both are even, decide based on parity
  if ((counts[0] % 2) == (counts[1] % 2))
    return (counts[0] % 2 == 1) ? SurfaceSide::INSIDE : SurfaceSide::OUTSIDE;

  // --- (c) Check for other variation directions ---

  std::vector<Point> axis_dirs =
      (_dim == 3) ? std::vector<Point>{_second_variance_vector, _max_variance_vector}
                  : std::vector<Point>{_max_variance_vector};

  for (std::size_t idx = 0; idx < axis_dirs.size(); ++idx)
  {
    const auto & dir = axis_dirs[idx];
    std::array<Point, 2> ray_starts = {
        generateRayStart(p, false /*inverted*/, std::nullopt /*forced_xyz*/, idx),
        generateRayStart(p, true /*inverted*/, std::nullopt /*forced_xyz*/, idx)};

    for (int i = 0; i < 2; ++i)
    {
      int num = 0;
      for (const auto & elemID : collectCandidateElementIDs(p))
      {
        const auto & elem = _bd_elements[elemID].get();
        const auto ball = computeBoundingBall(elem);

        if (isOutsideRayBBox(ray_starts[i], dir, ball))
          continue;
        if (isOutsideBoundingRegion(ray_starts[i], dir, ball))
          continue;

        if (rayIntersectGeometry(ray_starts[i], p, elem))
          num++;
      }

      if (num == 0)
        return SurfaceSide::OUTSIDE;
    }
  }

  std::ostringstream oss;
  oss << p;
  mooseError("PointInPolyhedronCheck: No decision could be made for point " + oss.str());
}

bool
PointInPolyhedronCheck::rayIntersectGeometry(const Point & ray_start,
                                             const Point & ray_end,
                                             const SBMBndElementBase * elem) const
{
  LineSegment ray_segment(ray_start, ray_end);
  return elem->intersect(ray_segment);
}

Ball
PointInPolyhedronCheck::computeBoundingBall(const SBMBndElementBase * elem) const
{
  return elem->computeBoundingBall();
}

bool
PointInPolyhedronCheck::isOutsideBoundingBox(const Point & query_point) const
{
  return (_build_obb) ? !_obb_bounds.contains(query_point, _eps_on_surface)
                      : !_bounds.contains_point(query_point);
}

bool
PointInPolyhedronCheck::isOutsideRayBBox(const Point & orig,
                                         const Point & dir,
                                         const Ball & ball) const
{
  Point lb, ub;
  const auto & center = ball.center();
  const double radius = ball.radius();

  for (int i = 0; i < _dim; ++i)
  {
    lb(i) = std::min(orig(i), orig(i) + dir(i)) - radius;
    ub(i) = std::max(orig(i), orig(i) + dir(i)) + radius;
  }

  for (int i = 0; i < _dim; ++i)
  {
    if (center(i) < lb(i) || center(i) > ub(i))
      return true;
  }

  return false;
}

bool
PointInPolyhedronCheck::isOutsideBoundingRegion(const Point & orig,
                                                const Point & dir,
                                                const Ball & ball) const
{
  const auto & center = ball.center();
  const auto radius = ball.radius();

  const auto w = center - orig;

  double b = (w * dir) / (dir * dir);
  Point Pb = orig + b * dir;

  double distance_squared = 0.0;
  for (int i = 0; i < _dim; ++i)
    distance_squared += std::pow(Pb(i) - center(i), 2);

  return (distance_squared > radius * radius);
}

BoundingBox
PointInPolyhedronCheck::computeGlobalBoundingBox()
{
  _bounds_ready = true;
  const auto & first_elem = _bd_elements[0]->elem();
  BoundingBox bbox = first_elem.loose_bounding_box();

  for (const auto & bd_elem : _bd_elements)
    bbox.union_with(bd_elem->elem().loose_bounding_box());

  const Real eps = _eps_on_surface;
  Point min_pt = bbox.min();
  Point max_pt = bbox.max();

  for (unsigned int d = 0; d < 3; ++d)
  {
    min_pt(d) -= eps;
    max_pt(d) += eps;
  }

  return BoundingBox(min_pt, max_pt);
}

const Point
PointInPolyhedronCheck::generateRayStart(const Point & point,
                                         const bool inverted,
                                         const std::optional<Point> & forced_ray_direction_XYZ,
                                         const int number_to_larger_variance) const
{
  if (!_build_obb || forced_ray_direction_XYZ.has_value()) // Ray direction is X, Y, or Z
  {
    const auto & min = _bounds.min();
    const auto & max = _bounds.max();

    // Copy point to starting point
    Point starting_point = point;

    int ray_dir;
    for (int dim = 0; dim < _dim; ++dim)
      if (!_build_obb && _ray_direction(dim) == 1.0)
        ray_dir = dim;
      else if (forced_ray_direction_XYZ && forced_ray_direction_XYZ.value()(dim) == 1.0)
        ray_dir = dim;

    // Extend starting point outside bounding box in ray_dir

    const double ray_extent = max(ray_dir) - min(ray_dir);

    if (max(ray_dir) + min(ray_dir) < 2 * point(ray_dir))
      starting_point(ray_dir) = inverted ? max(ray_dir) + ray_extent : min(ray_dir) - ray_extent;
    else
      starting_point(ray_dir) = inverted ? min(ray_dir) - ray_extent : max(ray_dir) + ray_extent;

    return starting_point;
  }

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

  Point starting_point =
      projectPointToRayDirPlane(point, projection_plane_corner, number_to_larger_variance);
  return starting_point - (SAFE_FACTOR * last_axis_length * direction_multiplier) * _ray_direction;
}

///  Automatically set the ray direction if _ray_direction is not set previously.
void
PointInPolyhedronCheck::initializeRayDirection()
{
  if (MooseUtils::absoluteFuzzyEqual(_ray_direction(0), 1.0) or
      MooseUtils::absoluteFuzzyEqual(_ray_direction(1), 1.0) or
      MooseUtils::absoluteFuzzyEqual(_ray_direction(2), 1.0))
  {
    _ray_direction.unit();
    _bounds = computeGlobalBoundingBox();
  }
  else
  {
    _ray_direction = (_dim == 3) ? _min_variance_vector : _second_variance_vector;
    _build_obb = true;
  }
}

Point
PointInPolyhedronCheck::projectPointToRayDirPlane(const Point & point,
                                                  const Point & point_on_the_plane,
                                                  const int number_to_larger_variance) const
{

  Point ray_direction;
  if (number_to_larger_variance == 0)
    ray_direction = _ray_direction;
  else if (_dim - number_to_larger_variance == 0)
    ray_direction = _max_variance_vector;
  else if (_dim - number_to_larger_variance == 1 /* for 3D only*/)
    ray_direction = _second_variance_vector;

  // Compute the vector from the plane origin to the given point
  Point vector_to_point = point - point_on_the_plane;

  // Project the vector onto the ray direction
  Real projection_length = vector_to_point * _ray_direction;

  // Subtract the projection to get the component perpendicular to the ray direction
  Point perpendicular_component = vector_to_point - projection_length * _ray_direction;

  // Add the perpendicular component to the plane origin to get the projected point
  return point_on_the_plane + perpendicular_component;
}

void
PointInPolyhedronCheck::preparePCASVD()
{
  Point centroid_sum;

  std::vector<Point> nodal_points;
  for (const auto & elem : _bd_elements)
  {
    const auto & e = elem->elem();
    for (unsigned int i = 0; i < e.n_nodes(); ++i)
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
  for (unsigned int i = 0; i < N; ++i)
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

  // (e) normalized it to be safe
  _max_variance_vector.unit();
  _second_variance_vector.unit();
  _min_variance_vector.unit();

  mooseAssert(
      MooseUtils::absoluteFuzzyEqual(_max_variance_vector * _second_variance_vector, 0.0) &&
          MooseUtils::absoluteFuzzyEqual(_max_variance_vector * _min_variance_vector, 0.0) &&
          MooseUtils::absoluteFuzzyEqual(_second_variance_vector * _min_variance_vector, 0.0),
      "Principal directions are not orthogonal.");
}

void
PointInPolyhedronCheck::buildObbKdtreeAndMaxProjectedDiagonal(const Real expand_box_length)
{
  if (!_centroids.empty())
    mooseAssert(_centroids.size() >= 3, "Need at least three points.");

  // (a) Prepare KD‑tree data (optional) and track PCA‑space extents

  // Initialize the ray direction if not set
  initializeRayDirection();

  // Global min / max along the three PCA axes
  Real u_min = std::numeric_limits<Real>::max();
  Real u_max = std::numeric_limits<Real>::lowest();
  Real v_min = u_min, v_max = u_max;
  Real w_min = u_min, w_max = u_max;

  if (!_brute_force_looping_all_bndelements)
    _projected_centroids.resize(_num_elements);

  _max_projected_diag_length = 0.0;

  for (std::size_t i = 0; i < _num_elements; ++i)
  {
    // Per‑element data for optional KD‑tree
    if (!_brute_force_looping_all_bndelements)
    {
      const Point & pt =
          (!_centroids.empty() ? _centroids[i] : _bd_elements[i]->elem().vertex_average());

      _projected_centroids[i] = projectPointToRayDirPlane(pt, _plane_origin);

      if (_dim == 2) // flatten Z in 2‑D mode
        _projected_centroids[i](2) = 0.0;

      _max_projected_diag_length =
          std::max(_max_projected_diag_length,
                   _bd_elements[i]->getProjectedBoundingBoxDiagonal(_ray_direction));
    }

    if (_build_obb)
    {
      // Update PCA‑space bounding box
      const Elem & e = _bd_elements[i]->elem();

      for (unsigned int j = 0; j < e.n_nodes(); ++j)
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
      std::filesystem::path obb_path(_obb_file_name.c_str());
      _obb_bounds.writeVTK(obb_path);
    }

    if (_ray_file_name != "")
    {
      std::filesystem::path ray_path(_ray_file_name.c_str());
      _obb_bounds.writeRayAlongShortestAxis(ray_path);
    }
  }

  // (c) Finalise KD‑tree (if requested)
  if (!_brute_force_looping_all_bndelements)
    _kd_tree = std::make_unique<KDTree>(_projected_centroids, _leaf_max_size);
}

std::vector<unsigned int>
PointInPolyhedronCheck::collectCandidateElementIDs(const Point & query_point) const
{
  std::vector<unsigned int> elem_ids;

  // (a) Brute‑force: test every boundary element
  if (_brute_force_looping_all_bndelements)
  {
    elem_ids.resize(_num_elements);
    std::iota(elem_ids.begin(), elem_ids.end(), 0); // 0, 1, 2, ...
    return elem_ids;
  }

  // (b) KD‑tree radius search in projected PCA space
  Point proj = projectPointToRayDirPlane(query_point, _plane_origin);
  if (_dim == 2)
    proj(2) = 0.0; // flatten Z for 2‑D

  std::vector<nanoflann::ResultItem<std::size_t, Real>> matches;
  _kd_tree->radiusSearch(proj, _max_projected_diag_length, matches);

  elem_ids.reserve(matches.size());
  for (const auto & m : matches)
    elem_ids.push_back(static_cast<unsigned int>(m.first));

  return elem_ids;
}
