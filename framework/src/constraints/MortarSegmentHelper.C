//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "MortarSegmentHelper.h"
#include "MooseError.h"

#include "libmesh/int_range.h"
#include "libmesh/utility.h"
#if defined(LIBMESH_HAVE_TRIANGLE) || defined(LIBMESH_HAVE_POLY2TRI)
#include "libmesh/replicated_mesh.h"
#include "libmesh/mesh_triangle_interface.h"
#include "libmesh/poly2tri_triangulator.h"
#endif

#include <Eigen/Dense>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>

using namespace libMesh;

namespace
{

constexpr Real mortar_reference_mapping_tolerance = 1e-8;
constexpr unsigned int quad_newton_max_iterations = 25;
constexpr unsigned int quad_newton_max_backtracks = 12;

bool
isFinitePoint(const Point & point)
{
  for (const auto component : make_range(LIBMESH_DIM))
    if (!std::isfinite(point(component)))
      return false;

  return true;
}

Real
norm2D(const Point & point)
{
  return std::hypot(point(0), point(1));
}

std::array<Real, 4>
bilinearQuadShape(const Real xi, const Real eta)
{
  return {{0.25 * (1. - xi) * (1. - eta),
           0.25 * (1. + xi) * (1. - eta),
           0.25 * (1. + xi) * (1. + eta),
           0.25 * (1. - xi) * (1. + eta)}};
}

struct BilinearQuadEvaluation
{
  Point mapped_point;
  Point dxdxi;
  Point dxdeta;
};

BilinearQuadEvaluation
evaluateBilinearQuad(const std::array<Point, 4> & poly, const Real xi, const Real eta)
{
  const auto phi = bilinearQuadShape(xi, eta);
  const std::array<Real, 4> dphi_dxi = {
      {-0.25 * (1. - eta), 0.25 * (1. - eta), 0.25 * (1. + eta), -0.25 * (1. + eta)}};
  const std::array<Real, 4> dphi_deta = {
      {-0.25 * (1. - xi), -0.25 * (1. + xi), 0.25 * (1. + xi), 0.25 * (1. - xi)}};

  BilinearQuadEvaluation evaluation;
  for (const auto i : index_range(poly))
  {
    evaluation.mapped_point += phi[i] * poly[i];
    evaluation.dxdxi += dphi_dxi[i] * poly[i];
    evaluation.dxdeta += dphi_deta[i] * poly[i];
  }

  return evaluation;
}

bool
solve2x2(const Point & column_zero,
         const Point & column_one,
         const Point & rhs,
         Real & solution_zero,
         Real & solution_one)
{
  Eigen::Matrix<Real, 2, 2> jacobian;
  jacobian << column_zero(0), column_one(0), column_zero(1), column_one(1);
  const Eigen::JacobiSVD<Eigen::Matrix<Real, 2, 2>> svd(jacobian,
                                                        Eigen::ComputeFullU | Eigen::ComputeFullV);
  const auto & singular_values = svd.singularValues();

  if (!singular_values.allFinite() || singular_values(0) <= 0. ||
      singular_values(1) <= mortar_reference_mapping_tolerance * singular_values(0))
    return false;

  const Eigen::Matrix<Real, 2, 1> solution = svd.solve(Eigen::Matrix<Real, 2, 1>(rhs(0), rhs(1)));
  solution_zero = solution(0);
  solution_one = solution(1);

  return std::isfinite(solution_zero) && std::isfinite(solution_one);
}

// Signed-area test for the 2D triangle (a, b, c). Returns twice the signed area:
// positive if a->b->c is counter-clockwise, negative if clockwise, zero if
// collinear. Used as the building block for orientation, point-in-triangle, and
// circumcircle predicates.
Real
orient2dHelper(const Point & a, const Point & b, const Point & c)
{
  return (b(0) - a(0)) * (c(1) - a(1)) - (b(1) - a(1)) * (c(0) - a(0));
}

Real
triangleAreaHelper(const Point & a, const Point & b, const Point & c)
{
  return 0.5 * std::abs(orient2dHelper(a, b, c));
}

// Canonical key for an undirected edge: the two endpoint indices sorted so that
// (a, b) and (b, a) hash and compare equal. Used to dedupe / look up edges in
// triangle-adjacency maps.
std::array<unsigned int, 2>
canonicalEdgeHelper(const unsigned int a, const unsigned int b)
{
  return {{std::min(a, b), std::max(a, b)}};
}

// Reorder the three vertex indices (a, b, c) so the resulting triangle is wound
// counter-clockwise (CCW) in the 2D plane spanned by \p nodes. Many of the
// triangulation paths (orientation tests, area accumulation, ear-clipping
// validity checks) assume CCW input, so we normalize before emitting triangles.
std::array<unsigned int, 3>
makeCCWTriangleHelper(const std::vector<Point> & nodes,
                      const unsigned int a,
                      const unsigned int b,
                      const unsigned int c)
{
  if (orient2dHelper(nodes[a], nodes[b], nodes[c]) >= 0)
    return {{a, b, c}};
  return {{a, c, b}};
}

bool
pointInCircumcircleHelper(const Point & a, const Point & b, const Point & c, const Point & p)
{
  const auto ax = a(0) - p(0);
  const auto ay = a(1) - p(1);
  const auto bx = b(0) - p(0);
  const auto by = b(1) - p(1);
  const auto cx = c(0) - p(0);
  const auto cy = c(1) - p(1);
  const Real det = (ax * ax + ay * ay) * (bx * cy - by * cx) -
                   (bx * bx + by * by) * (ax * cy - ay * cx) +
                   (cx * cx + cy * cy) * (ax * by - ay * bx);
  const Real orientation = orient2dHelper(a, b, c);
  return orientation >= 0 ? det > TOLERANCE : det < -TOLERANCE;
}

void
performLocalDelaunayFlips(const std::vector<Point> & poly_nodes,
                          const std::set<std::array<unsigned int, 2>> & constrained_edges,
                          std::vector<std::array<unsigned int, 3>> & triangles)
{
  bool flipped = true;
  while (flipped)
  {
    flipped = false;

    std::map<std::array<unsigned int, 2>, std::vector<unsigned int>> edge_to_triangles;
    for (const auto tri_index : index_range(triangles))
    {
      const auto & tri = triangles[tri_index];
      edge_to_triangles[canonicalEdgeHelper(tri[0], tri[1])].push_back(tri_index);
      edge_to_triangles[canonicalEdgeHelper(tri[1], tri[2])].push_back(tri_index);
      edge_to_triangles[canonicalEdgeHelper(tri[2], tri[0])].push_back(tri_index);
    }

    for (const auto & [edge, owning_triangles] : edge_to_triangles)
    {
      if (owning_triangles.size() != 2 || constrained_edges.count(edge))
        continue;

      const auto first_tri_index = owning_triangles[0];
      const auto second_tri_index = owning_triangles[1];
      const auto & first_triangle = triangles[first_tri_index];
      const auto & second_triangle = triangles[second_tri_index];

      const auto a = edge[0];
      const auto b = edge[1];
      const auto first_opposite =
          *std::find_if(first_triangle.begin(),
                        first_triangle.end(),
                        [a, b](const unsigned int vertex) { return vertex != a && vertex != b; });
      const auto second_opposite =
          *std::find_if(second_triangle.begin(),
                        second_triangle.end(),
                        [a, b](const unsigned int vertex) { return vertex != a && vertex != b; });

      if (first_opposite == second_opposite)
        continue;

      const auto side_a =
          orient2dHelper(poly_nodes[first_opposite], poly_nodes[second_opposite], poly_nodes[a]);
      const auto side_b =
          orient2dHelper(poly_nodes[first_opposite], poly_nodes[second_opposite], poly_nodes[b]);
      if (side_a * side_b >= -TOLERANCE)
        continue;

      if (!pointInCircumcircleHelper(poly_nodes[first_triangle[0]],
                                     poly_nodes[first_triangle[1]],
                                     poly_nodes[first_triangle[2]],
                                     poly_nodes[second_opposite]))
        continue;

      triangles[first_tri_index] =
          makeCCWTriangleHelper(poly_nodes, first_opposite, second_opposite, b);
      triangles[second_tri_index] =
          makeCCWTriangleHelper(poly_nodes, second_opposite, first_opposite, a);
      flipped = true;
      break;
    }
  }
}

#if defined(LIBMESH_HAVE_TRIANGLE) || defined(LIBMESH_HAVE_POLY2TRI)
void
triangulateConstrainedDelaunayPolygon(std::vector<Point> & poly_nodes,
                                      const Real area_tol,
                                      const Real length_tol,
                                      std::vector<std::vector<unsigned int>> & tri_map)
{
  Parallel::Communicator comm_self;
  ReplicatedMesh triangulation_mesh(comm_self, 2);
  std::unordered_map<dof_id_type, unsigned int> node_id_to_local_index;
  node_id_to_local_index.reserve(poly_nodes.size());

  for (const auto i : index_range(poly_nodes))
    triangulation_mesh.add_point(poly_nodes[i], i);

  triangulation_mesh.set_mesh_dimension(2);

#ifdef LIBMESH_HAVE_TRIANGLE
  TriangleInterface triangulator(triangulation_mesh);
#else
  Poly2TriTriangulator triangulator(triangulation_mesh);
  triangulator.set_refine_boundary_allowed(false);
#endif

  triangulator.triangulation_type() = TriangulatorInterface::PSLG;
  triangulator.elem_type() = TRI3;
  triangulator.set_interpolate_boundary_points(0);
  triangulator.set_verify_hole_boundaries(false);
  triangulator.desired_area() = 0;
  triangulator.minimum_angle() = 0;
  triangulator.smooth_after_generating() = false;
  triangulator.quiet() = true;
  triangulator.segments.reserve(poly_nodes.size());
  for (const auto i : index_range(poly_nodes))
    triangulator.segments.emplace_back(i, (i + 1) % poly_nodes.size());

  triangulator.triangulate();

  // node_ptr_range() and active_element_ptr_range() iterate in id order on this
  // serial ReplicatedMesh, so no explicit sort is needed.
  for (const auto * const node : triangulation_mesh.node_ptr_range())
    if (!node_id_to_local_index.count(node->id()))
    {
      // Node inherits from Point and the triangulator operates on a 2D plane, so
      // the libMesh node already lives at z = 0 and we can use it directly.
      unsigned int matched_index = libMesh::invalid_uint;
      Real best_distance = std::numeric_limits<Real>::max();

      for (const auto i : index_range(poly_nodes))
      {
        const Real distance = (*node - poly_nodes[i]).norm();
        if (distance <= length_tol && distance < best_distance)
        {
          matched_index = i;
          best_distance = distance;
        }
      }

      if (matched_index == libMesh::invalid_uint)
      {
        matched_index = cast_int<unsigned int>(poly_nodes.size());
        poly_nodes.push_back(*node);
      }

      node_id_to_local_index.emplace(node->id(), matched_index);
    }

  std::vector<std::array<unsigned int, 3>> triangles;
  triangles.reserve(triangulation_mesh.n_elem());

  for (const auto * const elem : triangulation_mesh.active_element_ptr_range())
  {
    mooseAssert(elem->type() == TRI3,
                "The delaunay mortar triangulation backend produced a non-TRI3 element: "
                    << static_cast<int>(elem->type()));

    std::array<unsigned int, 3> local_triangle;
    for (const auto i : index_range(local_triangle))
      local_triangle[i] = libmesh_map_find(node_id_to_local_index, elem->node_id(i));

    const Real orientation = orient2dHelper(poly_nodes[local_triangle[0]],
                                            poly_nodes[local_triangle[1]],
                                            poly_nodes[local_triangle[2]]);
    if (std::abs(orientation) <= 2. * area_tol)
      continue;

    if (orientation < 0)
      std::swap(local_triangle[1], local_triangle[2]);

    triangles.push_back(local_triangle);
  }

  std::set<std::array<unsigned int, 2>> constrained_edges;
  for (const auto i : index_range(poly_nodes))
    constrained_edges.insert(canonicalEdgeHelper(i, (i + 1) % poly_nodes.size()));

  performLocalDelaunayFlips(poly_nodes, constrained_edges, triangles);

  std::set<std::array<unsigned int, 3>> seen_triangles;
  for (auto local_triangle : triangles)
  {
    auto canonical_triangle = local_triangle;
    std::sort(canonical_triangle.begin(), canonical_triangle.end());
    if (!seen_triangles.insert(canonical_triangle).second)
      continue;

    tri_map.push_back({local_triangle[0], local_triangle[1], local_triangle[2]});
  }
}
#endif

} // namespace

MortarSegmentHelper::MortarSegmentHelper(std::vector<Point> secondary_nodes,
                                         const Point & center,
                                         const Point & normal,
                                         const MortarSegmentTriangulationMode triangulation_mode,
                                         const bool triangulate_triangles)
  : MortarSegmentHelper(
        std::move(secondary_nodes), {}, center, normal, triangulation_mode, triangulate_triangles)
{
}

MortarSegmentHelper::MortarSegmentHelper(std::vector<Point> secondary_nodes,
                                         std::vector<Point> secondary_reference_points,
                                         const Point & center,
                                         const Point & normal,
                                         const MortarSegmentTriangulationMode triangulation_mode,
                                         const bool triangulate_triangles)
  : _center(center),
    _normal(normal),
    _debug(false),
    _triangulation_mode(triangulation_mode),
    _triangulate_triangles(triangulate_triangles),
    _secondary_reference_points(std::move(secondary_reference_points))
{
  mooseAssert(_secondary_reference_points.empty() ||
                  secondary_nodes.size() == _secondary_reference_points.size(),
              "Each projected secondary node needs one parent reference point.");

  _secondary_poly.clear();
  _secondary_poly.reserve(secondary_nodes.size());

  // Get orientation of secondary poly
  const Point e1 = secondary_nodes[0] - secondary_nodes[1];
  const Point e2 = secondary_nodes[2] - secondary_nodes[1];
  const Real orient = e2.cross(e1) * _normal;

  // u and v define the tangent plane of the element (at center)
  // Note we embed orientation into our transformation to make 2D poly always
  // positively oriented
  _u = _normal.cross(secondary_nodes[0] - center).unit();
  _v = (orient > 0) ? _normal.cross(_u).unit() : _u.cross(_normal).unit();

  // Transform problem to 2D plane spanned by u and v
  for (const auto & node : secondary_nodes)
  {
    Point pt = node - _center;
    _secondary_poly.emplace_back(pt * _u, pt * _v, 0);
  }

  // Initialize area of secondary polygon
  _remaining_area_fraction = 1.0;
  _secondary_area = area(_secondary_poly);

  // Tolerance for quantities with area dimensions
  _area_tol = _tolerance * _secondary_area;

  // Tolerance for quantites with length dimensions
  _length_tol = _tolerance * std::sqrt(_secondary_area);
}

Point
MortarSegmentHelper::getIntersection(
    const Point & p1, const Point & p2, const Point & q1, const Point & q2, Real & s) const
{
  const Point dp = p2 - p1;
  const Point dq = q2 - q1;
  const Real cp1q1 = p1(0) * q1(1) - p1(1) * q1(0);
  const Real cp1q2 = p1(0) * q2(1) - p1(1) * q2(0);
  const Real cq1q2 = q1(0) * q2(1) - q1(1) * q2(0);
  const Real alpha = 1. / (dp(0) * dq(1) - dp(1) * dq(0));
  s = -alpha * (cp1q2 - cp1q1 - cq1q2);

  // Intersection should be between p1 and p2, if it's not (due to poor conditioning), simply
  // move it to one of the end points
  s = s > 1 ? 1. : s;
  s = s < 0 ? 0. : s;
  return p1 + s * dp;
}

bool
MortarSegmentHelper::isInsideSecondary(const Point & pt) const
{
  for (auto i : index_range(_secondary_poly))
  {
    const Point & q1 = _secondary_poly[i];
    const Point & q2 = _secondary_poly[(i + 1) % _secondary_poly.size()];

    const Point e1 = q2 - q1;
    const Point e2 = pt - q1;

    // If point corresponds to one of the secondary vertices, skip
    if (e2.norm() < _tolerance)
      return true;

    const bool inside = (e1(0) * e2(1) - e1(1) * e2(0)) < _area_tol;
    if (!inside)
      return false;
  }
  return true;
}

bool
MortarSegmentHelper::isDisjoint(const std::vector<Point> & poly) const
{
  for (auto i : index_range(_secondary_poly))
  {
    // Get edge to check
    const Point & q1 = _secondary_poly[i];
    const Point & q2 = _secondary_poly[(i + 1) % _secondary_poly.size()];
    const Point edg = q2 - q1;
    const Real cp = q2(0) * q1(1) - q2(1) * q1(0);

    // If more optimization needed, could store these values for later
    // Check if point is to the left of (or on) clip_edge
    auto is_inside = [&edg, cp](Point & pt, Real tol)
    { return pt(0) * edg(1) - pt(1) * edg(0) + cp < -tol; };

    bool all_outside = true;
    for (auto pt : poly)
      if (is_inside(pt, _area_tol))
        all_outside = false;

    if (all_outside)
      return true;
  }
  return false;
}

std::vector<Point>
MortarSegmentHelper::projectPrimaryPoly(const std::vector<Point> & primary_nodes) const
{
  // Check orientation of primary_poly
  const Point e1 = primary_nodes[0] - primary_nodes[1];
  const Point e2 = primary_nodes[2] - primary_nodes[1];

  // Note we use u x v here instead of normal because it may be flipped if secondary elem was
  // negatively oriented
  const Real orient = e2.cross(e1) * _u.cross(_v);

  // Get primary_poly (primary is clipping poly). If negatively oriented, reverse
  std::vector<Point> primary_poly;
  const int n_verts = primary_nodes.size();
  primary_poly.reserve(primary_nodes.size());
  for (auto n : index_range(primary_nodes))
  {
    Point pt = (orient > 0) ? primary_nodes[n] - _center : primary_nodes[n_verts - 1 - n] - _center;
    primary_poly.emplace_back(pt * _u, pt * _v, 0.);
  }

  return primary_poly;
}

std::vector<Point>
MortarSegmentHelper::clipPoly(const std::vector<Point> & primary_nodes) const
{
  return clipProjectedPoly(projectPrimaryPoly(primary_nodes));
}

std::vector<Point>
MortarSegmentHelper::clipProjectedPoly(const std::vector<Point> & primary_poly) const
{
  if (isDisjoint(primary_poly))
    return {};

  // Initialize clipped poly with secondary poly (secondary is target poly)
  std::vector<Point> clipped_poly = _secondary_poly;

  // Loop through clipping edges
  for (auto i : index_range(primary_poly))
  {
    // If clipped poly trivial, return
    if (clipped_poly.size() < 3)
    {
      clipped_poly.clear();
      return clipped_poly;
    }

    // Set input poly to current clipped poly
    std::vector<Point> input_poly(clipped_poly);
    clipped_poly.clear();

    // Get clipping edge
    const Point & clip_pt1 = primary_poly[i];
    const Point & clip_pt2 = primary_poly[(i + 1) % primary_poly.size()];
    const Point edg = clip_pt2 - clip_pt1;
    const Real cp = clip_pt2(0) * clip_pt1(1) - clip_pt2(1) * clip_pt1(0);

    // Check if point is to the left of (or on) clip_edge
    /*
     * Note that use of tolerance here is to avoid degenerate case when lines are
     * essentially on top of each other (common when meshes match across interface)
     * since finding intersection is ill-conditioned in this case.
     */
    auto is_inside = [&edg, cp](const Point & pt, Real tol)
    { return pt(0) * edg(1) - pt(1) * edg(0) + cp < tol; };

    // Loop through edges of target polygon (with previous clippings already included)
    for (auto j : index_range(input_poly))
    {
      // Get target edge
      const Point curr_pt = input_poly[(j + 1) % input_poly.size()];
      const Point prev_pt = input_poly[j];

      // TODO: Don't need to calculate both each loop
      const bool is_current_inside = is_inside(curr_pt, _area_tol);
      const bool is_previous_inside = is_inside(prev_pt, _area_tol);

      if (is_current_inside)
      {
        if (!is_previous_inside)
        {
          Real s;
          Point intersect = getIntersection(prev_pt, curr_pt, clip_pt1, clip_pt2, s);

          /*
           * s is the fraction of distance along clip poly edge that intersection lies
           * It is used here to avoid degenerate polygon cases. For example, consider a
           * case like:
           *          o
           *          |    (inside)
           *    ------|------
           *          |    (outside)
           * when the distance is small (< 1e-7) we don't want to to add both the point
           * and intersection. Also note that when distance on the scale of 1e-7,
           * area on scale of 1e-14 so is insignificant if this results in dropping
           * a tri (for example if next edge crosses again)
           */
          if (s < (1 - _tolerance))
            clipped_poly.push_back(intersect);
        }
        clipped_poly.push_back(curr_pt);
      }
      else if (is_previous_inside)
      {
        Real s;
        Point intersect = getIntersection(prev_pt, curr_pt, clip_pt1, clip_pt2, s);
        if (s > _tolerance)
          clipped_poly.push_back(intersect);
      }
    }
  }

  // Make sure final clipped poly is not trivial
  if (clipped_poly.size() < 3)
  {
    clipped_poly.clear();
    return clipped_poly;
  }

  // Clean up result by removing any duplicate nodes
  std::vector<Point> cleaned_poly;
  cleaned_poly.push_back(clipped_poly.back());
  for (auto i : make_range(clipped_poly.size() - 1))
  {
    const Point prev_pt = cleaned_poly.back();
    const Point curr_pt = clipped_poly[i];

    // If points are sufficiently distanced, add to output
    if ((curr_pt - prev_pt).norm() > _length_tol)
      cleaned_poly.push_back(curr_pt);
  }

  mooseAssert(
      cleaned_poly.size() <= 8,
      "Our distributed mesh numbering scheme assumes that we have at most 8 nodes resulting from "
      "clipping the projection of the primary sub-element onto the secondary sub-element");
  return cleaned_poly;
}

void
MortarSegmentHelper::triangulatePoly(std::vector<Point> & poly_nodes,
                                     std::vector<std::vector<unsigned int>> & tri_map) const
{
  // tri_map is populated with triangle indices that are local to poly_nodes (starting at 0).
  // Callers are responsible for shifting these indices into a global node numbering.
  const auto polygon_centroid = [](const std::vector<Point> & polygon_nodes)
  {
    Point centroid(0);
    Real double_area = 0;
    for (const auto i : index_range(polygon_nodes))
    {
      const auto & a = polygon_nodes[i];
      const auto & b = polygon_nodes[(i + 1) % polygon_nodes.size()];
      const Real cross = a(0) * b(1) - b(0) * a(1);
      double_area += cross;
      centroid(0) += (a(0) + b(0)) * cross;
      centroid(1) += (a(1) + b(1)) * cross;
    }

    if (std::abs(double_area) <= TOLERANCE)
    {
      for (const auto & node : polygon_nodes)
        centroid += node;
      centroid /= polygon_nodes.size();
      return centroid;
    }

    centroid /= (3. * double_area);
    centroid(2) = 0;
    return centroid;
  };

  const auto append_triangle = [this, &poly_nodes, &tri_map](
                                   const unsigned int a, const unsigned int b, const unsigned int c)
  {
    if (triangleAreaHelper(poly_nodes[a], poly_nodes[b], poly_nodes[c]) <= _area_tol)
      return false;

    if (orient2dHelper(poly_nodes[a], poly_nodes[b], poly_nodes[c]) >= 0)
      tri_map.push_back({a, b, c});
    else
      tri_map.push_back({a, c, b});

    return true;
  };

  const auto point_in_triangle =
      [this](const Point & p, const Point & a, const Point & b, const Point & c)
  {
    const Real ab = orient2dHelper(a, b, p);
    const Real bc = orient2dHelper(b, c, p);
    const Real ca = orient2dHelper(c, a, p);
    return ab >= -_area_tol && bc >= -_area_tol && ca >= -_area_tol;
  };

  const auto min_triangle_angle = [](const Point & a, const Point & b, const Point & c)
  {
    const auto clamp_cos = [](Real value) { return std::max(-1., std::min(1., value)); };
    const auto angle_at =
        [&clamp_cos](const Point & vertex, const Point & point_one, const Point & point_two)
    {
      const Point edge_one = point_one - vertex;
      const Point edge_two = point_two - vertex;
      const Real denom = edge_one.norm() * edge_two.norm();
      if (denom <= TOLERANCE)
        return 0.;
      return std::acos(clamp_cos((edge_one * edge_two) / denom));
    };

    return std::min({angle_at(a, b, c), angle_at(b, c, a), angle_at(c, a, b)});
  };

  const auto canonicalize_polygon = [this, &poly_nodes]()
  {
    if (poly_nodes.size() < 3)
      return;

    if (area(poly_nodes) < 0)
      std::reverse(poly_nodes.begin(), poly_nodes.end());

    bool changed = true;
    while (changed && poly_nodes.size() > 3)
    {
      changed = false;
      for (const auto i : index_range(poly_nodes))
      {
        const auto prev = (i + poly_nodes.size() - 1) % poly_nodes.size();
        const auto next = (i + 1) % poly_nodes.size();
        if ((poly_nodes[i] - poly_nodes[prev]).norm() <= _length_tol ||
            (poly_nodes[next] - poly_nodes[i]).norm() <= _length_tol ||
            triangleAreaHelper(poly_nodes[prev], poly_nodes[i], poly_nodes[next]) <= _area_tol)
        {
          poly_nodes.erase(poly_nodes.begin() + i);
          changed = true;
          break;
        }
      }
    }

    if (poly_nodes.size() >= 3 && area(poly_nodes) < 0)
      std::reverse(poly_nodes.begin(), poly_nodes.end());
  };

  const auto triangulate_with_ear_clipping =
      [this, &poly_nodes, &point_in_triangle, &min_triangle_angle](
          const bool perform_delaunay_flips)
  {
    std::vector<std::array<unsigned int, 3>> triangles;
    if (poly_nodes.size() < 3)
      return triangles;

    if (poly_nodes.size() == 3)
    {
      triangles.push_back(makeCCWTriangleHelper(poly_nodes, 0, 1, 2));
      return triangles;
    }

    std::vector<unsigned int> remaining_vertices(poly_nodes.size());
    std::iota(remaining_vertices.begin(), remaining_vertices.end(), 0);

    while (remaining_vertices.size() > 3)
    {
      std::optional<std::size_t> best_position;
      Real best_score = -std::numeric_limits<Real>::max();
      Real best_area = -std::numeric_limits<Real>::max();

      for (const auto position : index_range(remaining_vertices))
      {
        const auto prev_position =
            (position + remaining_vertices.size() - 1) % remaining_vertices.size();
        const auto next_position = (position + 1) % remaining_vertices.size();
        const auto prev = remaining_vertices[prev_position];
        const auto curr = remaining_vertices[position];
        const auto next = remaining_vertices[next_position];

        if (orient2dHelper(poly_nodes[prev], poly_nodes[curr], poly_nodes[next]) <= _area_tol)
          continue;

        bool contains_other_vertex = false;
        for (const auto other : remaining_vertices)
        {
          if (other == prev || other == curr || other == next)
            continue;

          if (point_in_triangle(
                  poly_nodes[other], poly_nodes[prev], poly_nodes[curr], poly_nodes[next]))
          {
            contains_other_vertex = true;
            break;
          }
        }

        if (contains_other_vertex)
          continue;

        const Real candidate_score =
            min_triangle_angle(poly_nodes[prev], poly_nodes[curr], poly_nodes[next]);
        const Real candidate_area =
            triangleAreaHelper(poly_nodes[prev], poly_nodes[curr], poly_nodes[next]);
        if (!best_position || candidate_score > best_score + TOLERANCE ||
            (std::abs(candidate_score - best_score) <= TOLERANCE &&
             candidate_area > best_area + _area_tol))
        {
          best_position = position;
          best_score = candidate_score;
          best_area = candidate_area;
        }
      }

      if (!best_position)
      {
        std::vector<std::array<unsigned int, 3>> best_fan;
        Real best_fan_score = -std::numeric_limits<Real>::max();
        Real best_fan_area = -std::numeric_limits<Real>::max();

        for (const auto root_position : index_range(remaining_vertices))
        {
          std::vector<std::array<unsigned int, 3>> candidate_fan;
          Real candidate_score = std::numeric_limits<Real>::max();
          Real candidate_area = std::numeric_limits<Real>::max();
          bool valid_fan = true;
          const auto root = remaining_vertices[root_position];

          for (unsigned int step = 1; step + 1 < remaining_vertices.size(); ++step)
          {
            const auto next_position = (root_position + step) % remaining_vertices.size();
            const auto following_position = (root_position + step + 1) % remaining_vertices.size();
            const auto vertex_one = remaining_vertices[next_position];
            const auto vertex_two = remaining_vertices[following_position];

            if (orient2dHelper(poly_nodes[root], poly_nodes[vertex_one], poly_nodes[vertex_two]) <=
                _area_tol)
            {
              valid_fan = false;
              break;
            }

            candidate_fan.push_back(
                makeCCWTriangleHelper(poly_nodes, root, vertex_one, vertex_two));
            candidate_score =
                std::min(candidate_score,
                         min_triangle_angle(
                             poly_nodes[root], poly_nodes[vertex_one], poly_nodes[vertex_two]));
            candidate_area =
                std::min(candidate_area,
                         triangleAreaHelper(
                             poly_nodes[root], poly_nodes[vertex_one], poly_nodes[vertex_two]));
          }

          if (!valid_fan || candidate_fan.empty())
            continue;

          if (candidate_score > best_fan_score + TOLERANCE ||
              (std::abs(candidate_score - best_fan_score) <= TOLERANCE &&
               candidate_area > best_fan_area + _area_tol))
          {
            best_fan = std::move(candidate_fan);
            best_fan_score = candidate_score;
            best_fan_area = candidate_area;
          }
        }

        if (best_fan.empty())
          for (unsigned int i = 1; i + 1 < remaining_vertices.size(); ++i)
            best_fan.push_back(makeCCWTriangleHelper(poly_nodes,
                                                     remaining_vertices[0],
                                                     remaining_vertices[i],
                                                     remaining_vertices[i + 1]));

        triangles.insert(triangles.end(), best_fan.begin(), best_fan.end());
        break;
      }

      const auto prev_position =
          (*best_position + remaining_vertices.size() - 1) % remaining_vertices.size();
      const auto next_position = (*best_position + 1) % remaining_vertices.size();
      triangles.push_back(makeCCWTriangleHelper(poly_nodes,
                                                remaining_vertices[prev_position],
                                                remaining_vertices[*best_position],
                                                remaining_vertices[next_position]));
      remaining_vertices.erase(remaining_vertices.begin() + *best_position);
    }

    if (remaining_vertices.size() == 3)
      triangles.push_back(makeCCWTriangleHelper(
          poly_nodes, remaining_vertices[0], remaining_vertices[1], remaining_vertices[2]));

    if (!perform_delaunay_flips)
      return triangles;

    std::set<std::array<unsigned int, 2>> boundary_edges;
    for (const auto i : index_range(poly_nodes))
      boundary_edges.insert(canonicalEdgeHelper(i, (i + 1) % poly_nodes.size()));

    performLocalDelaunayFlips(poly_nodes, boundary_edges, triangles);
    return triangles;
  };

  const auto is_convex_polygon = [this](const std::vector<Point> & polygon_nodes)
  {
    if (polygon_nodes.size() <= 3)
      return true;

    for (const auto i : index_range(polygon_nodes))
    {
      const auto prev = (i + polygon_nodes.size() - 1) % polygon_nodes.size();
      const auto next = (i + 1) % polygon_nodes.size();
      if (orient2dHelper(polygon_nodes[prev], polygon_nodes[i], polygon_nodes[next]) <= _area_tol)
        return false;
    }

    return true;
  };

  // Fewer than 3 nodes can't be triangulated
  if (poly_nodes.size() < 3)
    mooseError("Can't triangulate poly with fewer than 3 nodes");

  // Legacy centroid path: when the default triangulation (centroid) is selected
  // and triangle re-tessellation is not requested, reproduce the legacy
  // algorithm byte-for-byte so existing mortar baselines remain valid.
  // Uses the arithmetic mean of the vertices (not the area-weighted centroid),
  // emits one triangle per polygon edge without degeneracy filtering, and skips
  // the canonicalization pass which would drop near-degenerate vertices and
  // perturb integration weights in downstream test baselines.
  if (_triangulation_mode == MortarSegmentTriangulationMode::Centroid && !_triangulate_triangles)
  {
    if (poly_nodes.size() == 3)
    {
      tri_map.push_back({0, 1, 2});
      return;
    }

    const unsigned int n_verts = poly_nodes.size();
    Point poly_center;
    for (const auto & node : poly_nodes)
      poly_center += node;
    poly_center /= n_verts;

    for (const auto i : make_range(n_verts))
      tri_map.push_back({i, (i + 1) % n_verts, n_verts});

    poly_nodes.push_back(poly_center);
    return;
  }

  canonicalize_polygon();
  if (poly_nodes.size() < 3)
    return;

  if (poly_nodes.size() == 3 && !_triangulate_triangles)
  {
    append_triangle(0, 1, 2);
    return;
  }

  const bool force_triangle_centroid_split = _triangulate_triangles && poly_nodes.size() == 3;

  if (_triangulation_mode == MortarSegmentTriangulationMode::Vertex &&
      !force_triangle_centroid_split)
  {
    const unsigned int n_verts = poly_nodes.size();
    for (unsigned int i = 1; i + 1 < n_verts; ++i)
      append_triangle(0, i, i + 1);
    return;
  }

  if (_triangulation_mode == MortarSegmentTriangulationMode::Delaunay &&
      !force_triangle_centroid_split)
  {
#if defined(LIBMESH_HAVE_TRIANGLE) || defined(LIBMESH_HAVE_POLY2TRI)
    triangulateConstrainedDelaunayPolygon(poly_nodes, _area_tol, _length_tol, tri_map);
    return;
#else
    mooseError("The 'delaunay' mortar triangulation mode requires libMesh TriangleInterface or "
               "Poly2Tri support.");
#endif
  }

  if (_triangulation_mode == MortarSegmentTriangulationMode::EarClipping &&
      !force_triangle_centroid_split)
  {
    for (const auto & triangle : triangulate_with_ear_clipping(true))
      append_triangle(triangle[0], triangle[1], triangle[2]);
    return;
  }

  if (!force_triangle_centroid_split && !is_convex_polygon(poly_nodes))
  {
    for (const auto & triangle : triangulate_with_ear_clipping(true))
      append_triangle(triangle[0], triangle[1], triangle[2]);
    return;
  }

  const unsigned int n_verts = poly_nodes.size();
  const Point poly_center = polygon_centroid(poly_nodes);

  bool added_triangle = false;
  for (const auto i : make_range(n_verts))
    if (triangleAreaHelper(poly_nodes[i], poly_nodes[(i + 1) % n_verts], poly_center) > _area_tol)
    {
      tri_map.push_back({i, (i + 1) % n_verts, n_verts});
      added_triangle = true;
    }

  if (added_triangle)
    poly_nodes.push_back(poly_center);
}

void
MortarSegmentHelper::getMortarSegments(const std::vector<Point> & primary_nodes,
                                       std::vector<Point> & nodes,
                                       std::vector<std::vector<unsigned int>> & elem_to_nodes)
{
  getMortarSegmentsImpl(primary_nodes, nodes, elem_to_nodes, nullptr);
}

void
MortarSegmentHelper::getMortarSegments(
    const std::vector<Point> & primary_nodes,
    const std::vector<Point> & primary_reference_points,
    std::vector<Point> & nodes,
    std::vector<std::vector<unsigned int>> & elem_to_nodes,
    std::vector<std::array<Point, 3>> & elem_to_secondary_reference_points,
    std::vector<std::array<Point, 3>> & elem_to_primary_reference_points,
    const Real minimum_segment_area)
{
  ReferenceMappingData reference_mapping{primary_reference_points,
                                         elem_to_secondary_reference_points,
                                         elem_to_primary_reference_points,
                                         minimum_segment_area};
  getMortarSegmentsImpl(primary_nodes, nodes, elem_to_nodes, &reference_mapping);
}

void
MortarSegmentHelper::getMortarSegmentsImpl(const std::vector<Point> & primary_nodes,
                                           std::vector<Point> & nodes,
                                           std::vector<std::vector<unsigned int>> & elem_to_nodes,
                                           ReferenceMappingData * const reference_mapping)
{
  std::vector<Point> primary_poly;
  std::vector<Point> primary_poly_reference_points;

  if (reference_mapping)
  {
    if (primary_nodes.size() != reference_mapping->primary_reference_points.size())
      mooseError("Reference-interpolation mortar segment generation requires one primary "
                 "reference point per primary sub-element node.");
    if (_secondary_poly.size() != _secondary_reference_points.size())
      mooseError("Reference-interpolation mortar segment generation requires one secondary "
                 "reference point per secondary sub-element node.");
    if (reference_mapping->elem_to_secondary_reference_points.size() != elem_to_nodes.size() ||
        reference_mapping->elem_to_primary_reference_points.size() != elem_to_nodes.size())
      mooseError("Reference-interpolation mortar segment outputs must be aligned before appending "
                 "new segments.");

    // Keep reference points in the projected polygon's orientation.
    const Point e1 = primary_nodes[0] - primary_nodes[1];
    const Point e2 = primary_nodes[2] - primary_nodes[1];
    const Real orient = e2.cross(e1) * _u.cross(_v);
    const auto n_verts = primary_nodes.size();

    primary_poly = projectPrimaryPoly(primary_nodes);
    primary_poly_reference_points.reserve(reference_mapping->primary_reference_points.size());
    for (const auto n : index_range(primary_nodes))
    {
      const auto primary_node_index = (orient > 0) ? n : n_verts - 1 - n;
      primary_poly_reference_points.push_back(
          reference_mapping->primary_reference_points[primary_node_index]);
    }
  }

  // Reference mode adds coordinate maps without changing clipping or triangulation.
  std::vector<Point> clipped_poly =
      reference_mapping ? clipProjectedPoly(primary_poly) : clipPoly(primary_nodes);
  if (clipped_poly.size() < 3)
    return;

  if (_debug)
    for (const auto & point : clipped_poly)
      if (!isInsideSecondary(point))
        mooseError("Clipped polygon not inside linearized secondary element");

  _remaining_area_fraction -= area(clipped_poly) / _secondary_area;

  std::vector<std::vector<unsigned int>> tri_map;
  triangulatePoly(clipped_poly, tri_map);
  if (reference_mapping && reference_mapping->minimum_segment_area > 0.)
    tri_map.erase(
        std::remove_if(tri_map.begin(),
                       tri_map.end(),
                       [&clipped_poly, reference_mapping](const std::vector<unsigned int> & tri)
                       {
                         mooseAssert(tri.size() == 3,
                                     "Mortar segment triangulation should only produce TRI3 maps.");
                         return triangleAreaHelper(clipped_poly[tri[0]],
                                                   clipped_poly[tri[1]],
                                                   clipped_poly[tri[2]]) <
                                reference_mapping->minimum_segment_area;
                       }),
        tri_map.end());
  if (tri_map.empty())
    return;

  std::vector<Point> secondary_node_reference_points;
  std::vector<Point> primary_node_reference_points;
  if (reference_mapping)
  {
    secondary_node_reference_points.reserve(clipped_poly.size());
    primary_node_reference_points.reserve(clipped_poly.size());

    const auto recover_reference_point = [this](const Point & projected_point,
                                                const std::vector<Point> & poly,
                                                const std::vector<Point> & reference_points,
                                                const char * const parent_name,
                                                const std::size_t node_index)
    {
      std::string failure_reason;
      const auto reference_point =
          referencePoint(projected_point, poly, reference_points, &failure_reason);
      if (!reference_point)
        mooseError("Unable to recover the ",
                   parent_name,
                   " parent reference point for retained 3D mortar overlap vertex ",
                   node_index,
                   " at projected point ",
                   projected_point,
                   ". Reason: ",
                   failure_reason,
                   ". Reference interpolation does not fall back to normal projection.");

      return *reference_point;
    };

    for (const auto node_index : index_range(clipped_poly))
    {
      const auto & point = clipped_poly[node_index];
      secondary_node_reference_points.push_back(recover_reference_point(
          point, _secondary_poly, _secondary_reference_points, "secondary", node_index));
      primary_node_reference_points.push_back(recover_reference_point(
          point, primary_poly, primary_poly_reference_points, "primary", node_index));
    }
  }

  const auto offset = cast_int<unsigned int>(nodes.size());
  for (const auto & point : clipped_poly)
    nodes.emplace_back((point(0) * _u) + (point(1) * _v) + _center);

  for (const auto & tri : tri_map)
  {
    std::vector<unsigned int> shifted_tri;
    shifted_tri.reserve(tri.size());
    for (const auto local_index : tri)
      shifted_tri.push_back(offset + local_index);
    elem_to_nodes.push_back(std::move(shifted_tri));

    if (reference_mapping)
    {
      mooseAssert(tri.size() == 3, "Mortar segment triangulation should only produce TRI3 maps.");
      std::array<Point, 3> elem_secondary_reference_points;
      std::array<Point, 3> elem_primary_reference_points;
      for (const auto n : index_range(tri))
      {
        const auto local_node = tri[n];
        elem_secondary_reference_points[n] = secondary_node_reference_points[local_node];
        elem_primary_reference_points[n] = primary_node_reference_points[local_node];
      }

      reference_mapping->elem_to_secondary_reference_points.push_back(
          elem_secondary_reference_points);
      reference_mapping->elem_to_primary_reference_points.push_back(elem_primary_reference_points);
    }
  }
}

std::optional<Point>
MortarSegmentHelper::referencePoint(const Point & point,
                                    const std::vector<Point> & poly,
                                    const std::vector<Point> & reference_points,
                                    std::string * const failure_reason) const
{
  mooseAssert(poly.size() == reference_points.size(),
              "Projected point and reference point containers should be the same size.");

  if (failure_reason)
    failure_reason->clear();

  const auto fail = [failure_reason](const std::string & reason) -> std::optional<Point>
  {
    if (failure_reason)
      *failure_reason = reason;
    return std::nullopt;
  };

  if (!isFinitePoint(point))
    return fail("the projected target point contains a non-finite coordinate");

  for (const auto i : index_range(poly))
  {
    if (!isFinitePoint(poly[i]))
      return fail("projected polygon vertex " + std::to_string(i) +
                  " contains a non-finite coordinate");
    if (!isFinitePoint(reference_points[i]))
      return fail("parent reference vertex " + std::to_string(i) +
                  " contains a non-finite coordinate");
  }

  if (poly.size() != 3 && poly.size() != 4)
    return fail("reference point recovery only supports triangular and quadrilateral mortar "
                "sub-elements, but received " +
                std::to_string(poly.size()) + " vertices");

  Real minimum_edge_length = std::numeric_limits<Real>::max();
  for (const auto i : index_range(poly))
    minimum_edge_length =
        std::min(minimum_edge_length, norm2D(poly[(i + 1) % poly.size()] - poly[i]));
  if (!std::isfinite(minimum_edge_length) ||
      minimum_edge_length <= 100. * std::numeric_limits<Real>::epsilon())
    return fail("the projected polygon has a zero-length edge");

  if (poly.size() == 3)
  {
    const Real local_scale = std::max(norm2D(poly[1] - poly[0]), norm2D(poly[2] - poly[0]));
    const Real singular_tolerance = 100. * std::numeric_limits<Real>::epsilon();
    if (!std::isfinite(local_scale) || local_scale <= singular_tolerance)
      return fail("triangle inverse map is singular because its local length scale is zero");

    const Point v1 = (poly[1] - poly[0]) / local_scale;
    const Point v2 = (poly[2] - poly[0]) / local_scale;
    const Point rhs = (point - poly[0]) / local_scale;
    const Real snapping_tolerance = std::max(mortar_reference_mapping_tolerance,
                                             _area_tol / (minimum_edge_length * local_scale));

    std::array<Real, 3> weights;
    if (!solve2x2(v1, v2, rhs, weights[1], weights[2]))
      return fail("triangle inverse map has a singular or ill-conditioned Jacobian");
    weights[0] = 1. - weights[1] - weights[2];

    for (const auto weight : weights)
      if (!std::isfinite(weight))
        return fail("triangle inverse map produced a non-finite barycentric coordinate");

    const Real recovery_residual = norm2D(weights[1] * v1 + weights[2] * v2 - rhs);
    if (!std::isfinite(recovery_residual) || recovery_residual > mortar_reference_mapping_tolerance)
    {
      std::ostringstream reason;
      reason << "triangle inverse map reconstruction has normalized residual " << recovery_residual
             << ", exceeding " << mortar_reference_mapping_tolerance;
      return fail(reason.str());
    }

    for (auto & weight : weights)
      weight = std::clamp(weight, 0., 1.);

    const Real weight_sum = std::accumulate(weights.begin(), weights.end(), 0.);
    if (!std::isfinite(weight_sum) || weight_sum <= singular_tolerance)
      return fail("clamped triangle barycentric coordinates have a zero or non-finite sum");

    for (auto & weight : weights)
      weight /= weight_sum;

    // Enforce partition of unity after clamping roundoff-sized violations.
    const auto corrected_weight =
        std::distance(weights.begin(), std::max_element(weights.begin(), weights.end()));
    weights[corrected_weight] = 1.;
    for (const auto i : index_range(weights))
      if (i != static_cast<unsigned int>(corrected_weight))
        weights[corrected_weight] -= weights[i];

    const Real snapped_residual = norm2D(weights[1] * v1 + weights[2] * v2 - rhs);
    if (!std::isfinite(snapped_residual) || snapped_residual > snapping_tolerance)
    {
      std::ostringstream reason;
      reason << "snapped triangle inverse map has normalized residual " << snapped_residual
             << ", exceeding the clipping-consistent tolerance " << snapping_tolerance;
      return fail(reason.str());
    }

    Point reference_point;
    for (const auto i : index_range(weights))
      reference_point += weights[i] * reference_points[i];

    if (!isFinitePoint(reference_point))
      return fail("triangle interpolation produced a non-finite parent reference point");

    return reference_point;
  }

  if (poly.size() == 4)
  {
    // Consistent corner Jacobians make this bilinear map one-to-one; extra seeds aid convergence.
    Point local_origin;
    for (const auto & vertex : poly)
      local_origin += vertex;
    local_origin /= poly.size();

    Real local_scale = 0.;
    for (const auto & vertex : poly)
      local_scale = std::max(local_scale, norm2D(vertex - local_origin));

    const Real singular_tolerance = 100. * std::numeric_limits<Real>::epsilon();
    if (!std::isfinite(local_scale) || local_scale <= singular_tolerance)
      return fail("quadrilateral inverse map is singular because its local length scale is zero");
    const Real snapping_tolerance = std::max(mortar_reference_mapping_tolerance,
                                             _area_tol / (minimum_edge_length * local_scale));

    std::array<Point, 4> normalized_poly;
    for (const auto i : index_range(normalized_poly))
      normalized_poly[i] = (poly[i] - local_origin) / local_scale;
    const Point normalized_point = (point - local_origin) / local_scale;

    const std::array<Point, 4> reference_corners = {
        {Point(-1., -1., 0.), Point(1., -1., 0.), Point(1., 1., 0.), Point(-1., 1., 0.)}};

    int jacobian_sign = 0;
    Real first_determinant = 0.;
    for (const auto corner : index_range(reference_corners))
    {
      const auto evaluation = evaluateBilinearQuad(
          normalized_poly, reference_corners[corner](0), reference_corners[corner](1));
      const Real determinant =
          evaluation.dxdxi(0) * evaluation.dxdeta(1) - evaluation.dxdxi(1) * evaluation.dxdeta(0);
      const Real determinant_scale = norm2D(evaluation.dxdxi) * norm2D(evaluation.dxdeta);

      if (!isFinitePoint(evaluation.mapped_point) || !isFinitePoint(evaluation.dxdxi) ||
          !isFinitePoint(evaluation.dxdeta) || !std::isfinite(determinant) ||
          !std::isfinite(determinant_scale))
      {
        std::ostringstream reason;
        reason << "quadrilateral corner " << corner << " has a non-finite Jacobian";
        return fail(reason.str());
      }

      if (determinant_scale <= singular_tolerance ||
          std::abs(determinant) <= mortar_reference_mapping_tolerance * determinant_scale)
      {
        std::ostringstream reason;
        reason << "quadrilateral corner " << corner << " has a singular Jacobian (determinant "
               << determinant << ")";
        return fail(reason.str());
      }

      const int current_sign = determinant > 0. ? 1 : -1;
      if (corner == 0)
      {
        jacobian_sign = current_sign;
        first_determinant = determinant;
      }
      else if (current_sign != jacobian_sign)
      {
        std::ostringstream reason;
        reason << "quadrilateral corner Jacobians have inconsistent signs (corner 0 determinant "
               << first_determinant << ", corner " << corner << " determinant " << determinant
               << "); the map is concave, folded, or self-intersecting";
        return fail(reason.str());
      }
    }

    const std::array<Point, 5> newton_seeds = {{Point(0., 0., 0.),
                                                Point(-1., -1., 0.),
                                                Point(1., -1., 0.),
                                                Point(1., 1., 0.),
                                                Point(-1., 1., 0.)}};

    const auto recover_root = [&normalized_poly, &normalized_point](
                                  Point coordinates, std::string & reason) -> std::optional<Point>
    {
      for (const auto iteration : make_range(quad_newton_max_iterations))
      {
        const auto evaluation =
            evaluateBilinearQuad(normalized_poly, coordinates(0), coordinates(1));
        const Point residual = evaluation.mapped_point - normalized_point;
        const Real residual_norm = norm2D(residual);

        if (!std::isfinite(residual_norm) || !isFinitePoint(evaluation.dxdxi) ||
            !isFinitePoint(evaluation.dxdeta))
        {
          reason = "Newton iteration produced a non-finite residual or Jacobian";
          return std::nullopt;
        }

        Real delta_xi;
        Real delta_eta;
        const Point rhs(-residual(0), -residual(1), 0.);
        if (!solve2x2(evaluation.dxdxi, evaluation.dxdeta, rhs, delta_xi, delta_eta))
        {
          std::ostringstream stream;
          stream << "Newton iteration " << iteration << " encountered a singular Jacobian";
          reason = stream.str();
          return std::nullopt;
        }

        const Real full_update_norm = std::hypot(delta_xi, delta_eta);
        if (!std::isfinite(full_update_norm))
        {
          reason = "Newton iteration produced a non-finite reference-space update";
          return std::nullopt;
        }

        if (residual_norm <= mortar_reference_mapping_tolerance &&
            full_update_norm <= mortar_reference_mapping_tolerance)
          return coordinates;

        Real step_length = 1.;
        bool accepted_step = false;
        for (unsigned int backtracks = 0; backtracks <= quad_newton_max_backtracks; ++backtracks)
        {
          const Point trial_coordinates(coordinates(0) + step_length * delta_xi,
                                        coordinates(1) + step_length * delta_eta,
                                        0.);
          const auto trial_evaluation =
              evaluateBilinearQuad(normalized_poly, trial_coordinates(0), trial_coordinates(1));
          const Real trial_residual_norm = norm2D(trial_evaluation.mapped_point - normalized_point);

          if (std::isfinite(trial_residual_norm) && trial_residual_norm < residual_norm)
          {
            coordinates = trial_coordinates;
            accepted_step = true;
            break;
          }

          if (backtracks < quad_newton_max_backtracks)
            step_length *= 0.5;
        }

        if (!accepted_step)
        {
          std::ostringstream stream;
          stream << "Newton iteration " << iteration << " failed to reduce the normalized residual "
                 << "after " << quad_newton_max_backtracks << " half-step backtracks";
          reason = stream.str();
          return std::nullopt;
        }
      }

      std::ostringstream stream;
      stream << "Newton iteration did not converge in " << quad_newton_max_iterations
             << " iterations";
      reason = stream.str();
      return std::nullopt;
    };

    std::string last_seed_failure;

    for (const auto seed_index : index_range(newton_seeds))
    {
      std::string seed_reason;
      const auto recovered_root = recover_root(newton_seeds[seed_index], seed_reason);
      if (!recovered_root)
      {
        last_seed_failure = "seed " + std::to_string(seed_index) + ": " + seed_reason;
        continue;
      }

      Point root(
          std::clamp((*recovered_root)(0), -1., 1.), std::clamp((*recovered_root)(1), -1., 1.), 0.);
      const Real snapped_residual_norm = norm2D(
          evaluateBilinearQuad(normalized_poly, root(0), root(1)).mapped_point - normalized_point);
      if (!std::isfinite(snapped_residual_norm) || snapped_residual_norm > snapping_tolerance)
      {
        std::ostringstream stream;
        stream << "seed " << seed_index << " produced a boundary-snapped root with normalized "
               << "residual " << snapped_residual_norm;
        last_seed_failure = stream.str();
        continue;
      }

      const auto phi = bilinearQuadShape(root(0), root(1));
      Point reference_point;
      for (const auto i : index_range(phi))
        reference_point += phi[i] * reference_points[i];

      if (!isFinitePoint(reference_point))
      {
        last_seed_failure = "quadrilateral interpolation produced a non-finite reference point";
        continue;
      }

      return reference_point;
    }

    return fail("quadrilateral inverse map found no in-domain root from the center and corner "
                "seeds; last outcome: " +
                last_seed_failure);
  }

  mooseError("Unreachable mortar reference-point recovery branch.");
}

Real
MortarSegmentHelper::area(const std::vector<Point> & nodes) const
{
  Real poly_area = 0;
  for (auto i : index_range(nodes))
    poly_area += nodes[i](0) * nodes[(i + 1) % nodes.size()](1) -
                 nodes[i](1) * nodes[(i + 1) % nodes.size()](0);
  poly_area *= 0.5;
  return poly_area;
}
