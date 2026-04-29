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
#if defined(LIBMESH_HAVE_TRIANGLE) || defined(LIBMESH_HAVE_POLY2TRI)
#include "libmesh/replicated_mesh.h"
#include "libmesh/mesh_triangle_interface.h"
#include "libmesh/poly2tri_triangulator.h"
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <numeric>
#include <optional>
#include <set>
#include <unordered_map>

using namespace libMesh;

namespace
{

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

  std::vector<const Node *> sorted_nodes;
  sorted_nodes.reserve(triangulation_mesh.n_nodes());
  for (const auto & node : triangulation_mesh.node_ptr_range())
    sorted_nodes.push_back(node);
  std::sort(sorted_nodes.begin(),
            sorted_nodes.end(),
            [](const Node * const left, const Node * const right)
            { return left->id() < right->id(); });

  for (const auto * const node : sorted_nodes)
    if (!node_id_to_local_index.count(node->id()))
    {
      const Point node_point((*node)(0), (*node)(1), 0);
      unsigned int matched_index = libMesh::invalid_uint;
      Real best_distance = std::numeric_limits<Real>::max();

      for (const auto i : index_range(poly_nodes))
      {
        const Real distance = (node_point - poly_nodes[i]).norm();
        if (distance <= length_tol && distance < best_distance)
        {
          matched_index = i;
          best_distance = distance;
        }
      }

      if (matched_index == libMesh::invalid_uint)
      {
        matched_index = static_cast<unsigned int>(poly_nodes.size());
        poly_nodes.push_back(node_point);
      }

      node_id_to_local_index.emplace(node->id(), matched_index);
    }

  std::vector<std::array<unsigned int, 3>> triangles;
  triangles.reserve(triangulation_mesh.n_elem());

  std::vector<const Elem *> sorted_elems;
  sorted_elems.reserve(triangulation_mesh.n_elem());
  for (const auto & elem : triangulation_mesh.active_element_ptr_range())
    sorted_elems.push_back(elem);
  std::sort(sorted_elems.begin(),
            sorted_elems.end(),
            [](const Elem * const left, const Elem * const right)
            { return left->id() < right->id(); });

  for (const auto * const elem : sorted_elems)
  {
    if (elem->type() != TRI3)
      mooseError("The delaunay mortar triangulation backend produced a non-TRI3 element: ",
                 static_cast<int>(elem->type()));

    std::array<unsigned int, 3> local_triangle;
    for (const auto i : make_range(3u))
      local_triangle[i] = node_id_to_local_index.at(elem->node_id(i));

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

MortarSegmentHelper::MortarSegmentHelper(const std::vector<Point> secondary_nodes,
                                         const Point & center,
                                         const Point & normal,
                                         const MortarSegmentTriangulationMode triangulation_mode,
                                         const bool triangulate_triangles)
  : _center(center),
    _normal(normal),
    _debug(false),
    _triangulation_mode(triangulation_mode),
    _triangulate_triangles(triangulate_triangles)
{
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
  std::vector<Point> primary_poly = projectPrimaryPoly(primary_nodes);

  if (isDisjoint(primary_poly))
  {
    primary_poly.clear();
    return primary_poly;
  }

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
  // Clip primary elem against secondary elem
  std::vector<Point> clipped_poly = clipPoly(primary_nodes);
  if (clipped_poly.size() < 3)
    return;

  if (_debug)
    for (auto pt : clipped_poly)
      if (!isInsideSecondary(pt))
        mooseError("Clipped polygon not inside linearized secondary element");

  // Compute area of clipped polygon, update remaining area fraction
  _remaining_area_fraction -= area(clipped_poly) / _secondary_area;

  // Triangulate clip polygon. tri_map indices are local to clipped_poly (starting at 0); we
  // shift them into the global node numbering after appending the polygon nodes below.
  std::vector<std::vector<unsigned int>> tri_map;
  triangulatePoly(clipped_poly, tri_map);
  if (tri_map.empty())
    return;

  // Transform clipped poly back to (linearized) 3d and append to list
  const auto offset = static_cast<unsigned int>(nodes.size());
  for (auto pt : clipped_poly)
    nodes.emplace_back((pt(0) * _u) + (pt(1) * _v) + _center);

  for (const auto & tri : tri_map)
  {
    std::vector<unsigned int> shifted_tri;
    shifted_tri.reserve(tri.size());
    for (const auto local_index : tri)
      shifted_tri.push_back(offset + local_index);
    elem_to_nodes.push_back(std::move(shifted_tri));
  }
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
