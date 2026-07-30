//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseMeshXYMeshCuttingUtils.h"

#include "MooseMeshUtils.h"
#include "MooseMeshElementConversionUtils.h"
#include "MooseException.h"
#include "MooseTypes.h"

#include "libmesh/boundary_info.h"
#include "libmesh/face_c0polygon.h"
#include "libmesh/elem.h"
#include "libmesh/enum_order.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/node.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace MooseMeshXYMeshCuttingUtils
{

namespace
{
// Signed area of a closed polygon (twice the signed area, sign indicates orientation)
Real
signedAreaTwice(const std::vector<Point> & polyline)
{
  Real s = 0.0;
  const std::size_t n = polyline.size();
  for (std::size_t i = 0; i < n; ++i)
  {
    const Point & a = polyline[i];
    const Point & b = polyline[(i + 1) % n];
    s += a(0) * b(1) - b(0) * a(1);
  }
  return s;
}

// Distance from point p to a segment ab; if t is non-null, returns the parameter along ab
// (clamped to [0, 1]) corresponding to the closest point.
Real
distancePointSegment2D(const Point & p, const Point & a, const Point & b, Real * t_out = nullptr)
{
  const Real abx = b(0) - a(0);
  const Real aby = b(1) - a(1);
  const Real ab2 = abx * abx + aby * aby;
  Real t = 0.0;
  if (ab2 > 0.0)
    t = ((p(0) - a(0)) * abx + (p(1) - a(1)) * aby) / ab2;
  t = std::max(0.0, std::min(1.0, t));
  const Real cx = a(0) + t * abx;
  const Real cy = a(1) + t * aby;
  if (t_out)
    *t_out = t;
  return std::hypot(p(0) - cx, p(1) - cy);
}

// 2D segment-segment intersection on the XY plane.
// Returns true if proper or end-touching intersection exists within tol; writes t and s
// (parameters along p1->p2 and q1->q2 respectively) and the intersection point.
bool
segmentSegmentIntersect2D(const Point & p1,
                          const Point & p2,
                          const Point & q1,
                          const Point & q2,
                          Real tol,
                          Real & t,
                          Real & s,
                          Point & out)
{
  const Real rx = p2(0) - p1(0);
  const Real ry = p2(1) - p1(1);
  const Real sx = q2(0) - q1(0);
  const Real sy = q2(1) - q1(1);
  const Real denom = rx * sy - ry * sx;

  // Use a relative tolerance for the denominator based on the segment lengths
  const Real r_len = std::hypot(rx, ry);
  const Real s_len = std::hypot(sx, sy);
  const Real denom_tol = std::max(libMesh::TOLERANCE, tol) * std::max(r_len, s_len);

  if (std::abs(denom) <= denom_tol * std::max(r_len, s_len))
    return false; // Parallel or near-parallel (collinear handled by endpoint snap upstream)

  const Real qx_px = q1(0) - p1(0);
  const Real qy_py = q1(1) - p1(1);
  t = (qx_px * sy - qy_py * sx) / denom;
  s = (qx_px * ry - qy_py * rx) / denom;

  // Allow small over-shoot at endpoints
  const Real param_tol = std::max(libMesh::TOLERANCE, tol / std::max(r_len, libMesh::TOLERANCE));
  if (t < -param_tol || t > 1.0 + param_tol)
    return false;
  if (s < -param_tol || s > 1.0 + param_tol)
    return false;
  t = std::max(0.0, std::min(1.0, t));
  s = std::max(0.0, std::min(1.0, s));
  out = Point(p1(0) + t * rx, p1(1) + t * ry, 0.0);
  return true;
}
} // anonymous namespace

std::vector<Point>
extractClosedOuterPolyline(const libMesh::MeshBase & cutter)
{
  // Build the set of boundary side endpoints. Each side contributes a (from_node, to_node) pair
  // in CCW orientation relative to its parent element (which means with the element to the
  // LEFT of the directed edge). The outer boundary of the cutter region therefore traces CCW
  // around the cutter, if the cutter elements are themselves CCW.
  const libMesh::BoundaryInfo & boundary_info = cutter.get_boundary_info();
  const auto side_list = boundary_info.build_side_list();

  std::unordered_map<dof_id_type, dof_id_type> next_node;
  std::unordered_set<dof_id_type> all_nodes;
  for (const auto & t : side_list)
  {
    const dof_id_type elem_id = std::get<0>(t);
    const unsigned short side_id = std::get<1>(t);
    const Elem * elem = cutter.elem_ptr(elem_id);
    if (!elem)
      continue;
    if (elem->dim() != 2)
      continue;
    // Determine the two endpoint nodes of this side, in the order that puts the element on the
    // LEFT (i.e. side index k means edge from node k to node k+1 in standard 2D libMesh ordering)
    const auto sn = elem->nodes_on_side(side_id);
    if (sn.size() < 2)
      continue;
    const Node & n0 = *elem->node_ptr(sn[0]);
    const Node & n1 = *elem->node_ptr(sn[1]);
    const dof_id_type a = n0.id();
    const dof_id_type b = n1.id();
    if (next_node.count(a))
      throw MooseException(
          "Cutter outer boundary has a node with two outgoing edges: the boundary is not a "
          "simple closed loop.");
    next_node.emplace(a, b);
    all_nodes.insert(a);
    all_nodes.insert(b);
  }

  if (next_node.empty())
    throw MooseException("Cutter mesh has no boundary sides.");

  // Walk one loop from an arbitrary starting node
  const dof_id_type start = next_node.begin()->first;
  std::vector<dof_id_type> loop_ids;
  loop_ids.reserve(next_node.size());
  std::unordered_set<dof_id_type> visited;
  dof_id_type cur = start;
  while (true)
  {
    if (visited.count(cur))
    {
      if (cur != start)
        throw MooseException("Cutter outer boundary is not a simple closed loop.");
      break;
    }
    visited.insert(cur);
    loop_ids.push_back(cur);
    auto it = next_node.find(cur);
    if (it == next_node.end())
      throw MooseException("Cutter outer boundary is not a simple closed loop.");
    cur = it->second;
  }
  if (loop_ids.size() != next_node.size())
    throw MooseException(
        "Cutter outer boundary has multiple connected components; v1 supports a single simple "
        "closed loop only.");

  // Convert to points
  std::vector<Point> polyline;
  polyline.reserve(loop_ids.size());
  for (const auto nid : loop_ids)
    polyline.push_back(*cutter.node_ptr(nid));

  // Re-orient to CCW (positive signed area)
  if (signedAreaTwice(polyline) < 0.0)
    std::reverse(polyline.begin(), polyline.end());

  return polyline;
}

short
classifyPointVsPolyline(const Point & p,
                        libMesh::PointLocatorBase & cutter_locator,
                        const std::vector<Point> & polyline,
                        Real on_tol)
{
  // First check on-polyline within tol
  const std::size_t n = polyline.size();
  for (std::size_t i = 0; i < n; ++i)
  {
    const Point & a = polyline[i];
    const Point & b = polyline[(i + 1) % n];
    if (distancePointSegment2D(p, a, b) <= on_tol)
      return 0;
  }

  // Use PointLocator to determine inside vs outside
  const Elem * found = cutter_locator(p);
  return found ? +1 : -1;
}

std::vector<EdgePolylineHit>
intersectEdgeWithPolyline(const Point & a,
                          const Point & b,
                          const std::vector<Point> & polyline,
                          Real tol)
{
  std::vector<EdgePolylineHit> hits;
  const std::size_t n = polyline.size();
  for (std::size_t i = 0; i < n; ++i)
  {
    const Point & p1 = polyline[i];
    const Point & p2 = polyline[(i + 1) % n];
    Real t, s;
    Point out;
    if (segmentSegmentIntersect2D(a, b, p1, p2, tol, t, s, out))
      hits.push_back({t, i, out});
  }
  // Dedup near-coincident hits (within tol in t)
  std::sort(hits.begin(),
            hits.end(),
            [](const EdgePolylineHit & x, const EdgePolylineHit & y) { return x.t < y.t; });
  std::vector<EdgePolylineHit> dedup;
  const Real edge_len = std::hypot(b(0) - a(0), b(1) - a(1));
  const Real param_tol = edge_len > 0.0 ? std::max(libMesh::TOLERANCE, tol) / edge_len : 0.0;
  for (const auto & h : hits)
  {
    if (!dedup.empty() && std::abs(h.t - dedup.back().t) <= param_tol)
      continue;
    dedup.push_back(h);
  }
  return dedup;
}

std::size_t
snapNodesToPolyline(libMesh::ReplicatedMesh & primary,
                    const std::vector<Point> & polyline,
                    Real snap_tol,
                    bool only_interior)
{
  if (snap_tol <= 0.0)
    return 0;

  // Identify boundary nodes (for interior-only filter)
  std::set<dof_id_type> boundary_node_ids;
  if (only_interior)
  {
    const auto bnodes = libMesh::MeshTools::find_boundary_nodes(primary);
    boundary_node_ids.insert(bnodes.begin(), bnodes.end());
  }

  // Polyline AABB for an early-out filter
  Real xmin = polyline.front()(0), xmax = polyline.front()(0);
  Real ymin = polyline.front()(1), ymax = polyline.front()(1);
  for (const auto & p : polyline)
  {
    xmin = std::min(xmin, p(0));
    xmax = std::max(xmax, p(0));
    ymin = std::min(ymin, p(1));
    ymax = std::max(ymax, p(1));
  }
  xmin -= snap_tol;
  xmax += snap_tol;
  ymin -= snap_tol;
  ymax += snap_tol;

  std::size_t n_snapped = 0;
  const std::size_t n_seg = polyline.size();

  for (auto node_it = primary.nodes_begin(); node_it != primary.nodes_end(); ++node_it)
  {
    Node * node = *node_it;
    if (only_interior && boundary_node_ids.count(node->id()))
      continue;
    const Real x = (*node)(0);
    const Real y = (*node)(1);
    if (x < xmin || x > xmax || y < ymin || y > ymax)
      continue;

    // Find nearest polyline vertex
    Real best_d = std::numeric_limits<Real>::infinity();
    Point best_p;
    for (std::size_t i = 0; i < n_seg; ++i)
    {
      const Real dx = polyline[i](0) - x;
      const Real dy = polyline[i](1) - y;
      const Real d = std::hypot(dx, dy);
      if (d < best_d)
      {
        best_d = d;
        best_p = polyline[i];
      }
    }
    // Find nearest segment projection
    for (std::size_t i = 0; i < n_seg; ++i)
    {
      const Point & a = polyline[i];
      const Point & b = polyline[(i + 1) % n_seg];
      Real t;
      const Real d = distancePointSegment2D(*node, a, b, &t);
      if (d < best_d)
      {
        best_d = d;
        best_p = Point(a(0) + t * (b(0) - a(0)), a(1) + t * (b(1) - a(1)), 0.0);
      }
    }

    if (best_d <= snap_tol)
    {
      (*node)(0) = best_p(0);
      (*node)(1) = best_p(1);
      ++n_snapped;
    }
  }
  return n_snapped;
}

void
meshCutterRemoverCutElemPoly(libMesh::ReplicatedMesh & primary,
                             const std::vector<Point> & polyline,
                             libMesh::PointLocatorBase & cutter_locator,
                             CutMode mode,
                             subdomain_id_type outside_shift,
                             subdomain_id_type inside_shift,
                             const SubdomainName & outside_suffix,
                             const SubdomainName & inside_suffix,
                             subdomain_id_type block_id_to_remove,
                             boundary_id_type new_boundary_id,
                             Real tol)
{
  libMesh::BoundaryInfo & boundary_info = primary.get_boundary_info();
  const auto bdry_side_list = boundary_info.build_side_list();

  // Classify a primary vertex relative to the cutter region.
  // Returns: -1 outside cutter, 0 on polyline (within tol), +1 inside cutter.
  auto classify = [&cutter_locator, &polyline, tol](const Point & p) -> short
  { return classifyPointVsPolyline(p, cutter_locator, polyline, tol); };

  // Whether a class is on the retained side; retain_kind 0 = retain outside, 1 = retain inside.
  auto is_retained = [](short cls, int retain_kind) -> bool
  {
    if (cls == 0)
      return true;
    return retain_kind == 0 ? cls < 0 : cls > 0;
  };

  const bool do_inside = (mode == CutMode::REMOVE_OUTSIDE) || (mode == CutMode::KEEP_BOTH);
  const bool do_outside = (mode == CutMode::REMOVE_INSIDE) || (mode == CutMode::KEEP_BOTH);

  // First pass: classify each active element, tag fully-removed ones for deletion, relabel
  // fully-retained ones (KEEP_BOTH only).
  std::set<subdomain_id_type> original_subdomain_ids;
  std::vector<dof_id_type> cross_elems;
  for (auto elem_it = primary.active_elements_begin(); elem_it != primary.active_elements_end();
       ++elem_it)
  {
    Elem * elem = *elem_it;
    original_subdomain_ids.insert(elem->subdomain_id());

    const auto n_v = elem->n_vertices();
    unsigned int n_in = 0, n_out = 0;
    for (const auto i : make_range(n_v))
    {
      if (!MooseUtils::absoluteFuzzyEqual(elem->point(i)(2), 0.0))
        throw MooseException(
            "Primary mesh contains a non-XY-plane element; only 2D XY meshes are supported.");
      const short c = classify(elem->point(i));
      if (c > 0)
        ++n_in;
      else if (c < 0)
        ++n_out;
    }

    if (n_in == 0 && n_out == 0)
      continue; // All vertices on the polyline; treat as fully retained.

    if (n_in == 0)
    {
      // Fully outside cutter
      if (mode == CutMode::REMOVE_OUTSIDE)
        elem->subdomain_id() = block_id_to_remove;
      else if (mode == CutMode::KEEP_BOTH)
        elem->subdomain_id() += outside_shift;
      continue;
    }
    if (n_out == 0)
    {
      // Fully inside cutter
      if (mode == CutMode::REMOVE_INSIDE)
        elem->subdomain_id() = block_id_to_remove;
      else if (mode == CutMode::KEEP_BOTH)
        elem->subdomain_id() += inside_shift;
      continue;
    }
    if (elem->default_order() != libMesh::Order::FIRST)
      throw MooseException("Only first order elements are supported for cutting.");
    cross_elems.push_back(elem->id());
  }

  // Dedup intersection nodes across primary elements sharing a cut edge.
  using XKey = std::tuple<dof_id_type, dof_id_type, std::size_t>;
  std::map<XKey, Node *> cross_nodes;
  auto get_cross_node = [&primary, &cross_nodes](
                            Node * a, Node * b, std::size_t seg, const Point & p) -> Node *
  {
    const XKey key{std::min(a->id(), b->id()), std::max(a->id(), b->id()), seg};
    auto it = cross_nodes.find(key);
    if (it != cross_nodes.end())
      return it->second;
    Node * n = primary.add_point(p);
    cross_nodes[key] = n;
    return n;
  };

  // Dedup interior cutter polyline vertex nodes (one Node per cutter polyline vertex).
  std::map<std::size_t, Node *> interior_vert_nodes;
  auto get_interior_node = [&primary, &interior_vert_nodes](
                               std::size_t polyline_idx, const Point & p) -> Node *
  {
    auto it = interior_vert_nodes.find(polyline_idx);
    if (it != interior_vert_nodes.end())
      return it->second;
    Node * n = primary.add_point(p);
    interior_vert_nodes[polyline_idx] = n;
    return n;
  };

  // Test whether a point is strictly interior to a 2D element (not within tol of any vertex
  // or edge).
  auto pointStrictlyInsideElem = [](const Point & v, const Elem & elem, Real ptol) -> bool
  {
    if (!elem.contains_point(v, ptol))
      return false;
    const auto n_v = elem.n_vertices();
    for (const auto i : make_range(n_v))
    {
      if ((elem.point(i) - v).norm() <= ptol)
        return false;
      const Point & a = elem.point(i);
      const Point & b = elem.point((i + 1) % n_v);
      Real t;
      const Real d = distancePointSegment2D(v, a, b, &t);
      if (d <= ptol)
        return false;
    }
    return true;
  };

  // For each crossed element, build retained polygon(s) using a Sutherland-Hodgman walk. v1
  // restriction: at most one intersection per primary edge, and exactly two intersections per
  // element on two different edges (the cut chord plus any interior cutter polyline vertices
  // are inserted between the entry and exit crosses).
  for (const auto elem_id : cross_elems)
  {
    Elem * orig_elem = primary.elem_ptr(elem_id);
    const auto n_v = orig_elem->n_vertices();

    std::vector<std::vector<EdgePolylineHit>> edge_hits(n_v);
    unsigned int total_hits = 0;
    unsigned int edges_with_hits = 0;
    for (const auto i : make_range(n_v))
    {
      const auto j = (i + 1) % n_v;
      edge_hits[i] = intersectEdgeWithPolyline(
          orig_elem->point(i), orig_elem->point(j), polyline, tol);
      total_hits += edge_hits[i].size();
      if (!edge_hits[i].empty())
        ++edges_with_hits;
    }

    std::vector<short> cls(n_v);
    for (const auto i : make_range(n_v))
      cls[i] = classify(orig_elem->point(i));

    if (total_hits == 0)
      continue; // Mixed classification but no edge crossings; treat as fully retained.

    if (total_hits != 2 || edges_with_hits != 2)
      throw MooseException(
          "XYCutMeshByMeshGenerator: primary element " + std::to_string(elem_id) +
          " produced " + std::to_string(total_hits) + " intersections on " +
          std::to_string(edges_with_hits) +
          " edges, but only exactly 2 intersections on 2 different edges are supported. This "
          "indicates the cutter polyline winds within a single primary element (multi-component "
          "retained region).");

    // For each of the 2 hits, compute polyline_pos and classify as entry/exit based on the
    // cutter polyline direction relative to the primary edge tangent (CCW elem has interior
    // on LEFT of edge tangent, so polyline crossing with cutter-interior on LEFT-of-polyline
    // gives a consistent entry/exit determination).
    const std::size_t n_poly = polyline.size();
    struct HitInfo
    {
      unsigned int edge_idx;
      Real polyline_pos; // seg + s in [0, n_poly)
      bool is_entry;
    };
    std::vector<HitInfo> hit_info;
    hit_info.reserve(2);
    for (const auto i : make_range(n_v))
    {
      if (edge_hits[i].empty())
        continue;
      const auto & h = edge_hits[i].front();
      const Point & seg_a = polyline[h.seg];
      const Point & seg_b = polyline[(h.seg + 1) % n_poly];
      const Real segx = seg_b(0) - seg_a(0);
      const Real segy = seg_b(1) - seg_a(1);
      const Real seg_len2 = segx * segx + segy * segy;
      Real s = 0.0;
      if (seg_len2 > 0.0)
        s = ((h.p(0) - seg_a(0)) * segx + (h.p(1) - seg_a(1)) * segy) / seg_len2;
      s = std::max(0.0, std::min(1.0, s));
      const Real polyline_pos = static_cast<Real>(h.seg) + s;
      // Cross product of edge tangent and polyline direction: positive means polyline goes
      // INTO element interior (which is on LEFT of CCW edge tangent).
      const auto j = (i + 1) % n_v;
      const Real ex = orig_elem->point(j)(0) - orig_elem->point(i)(0);
      const Real ey = orig_elem->point(j)(1) - orig_elem->point(i)(1);
      const Real cz = ex * segy - ey * segx;
      hit_info.push_back({i, polyline_pos, cz > 0.0});
    }

    // Validate exactly one entry and one exit
    int n_entries = 0;
    for (const auto & hi : hit_info)
      if (hi.is_entry)
        ++n_entries;
    if (n_entries != 1 || hit_info.size() != 2)
      throw MooseException(
          "XYCutMeshByMeshGenerator: primary element " + std::to_string(elem_id) +
          " has inconsistent cutter-polyline entry/exit topology (multi-component retained "
          "region).");

    // Find interior cutter polyline vertices strictly inside this primary element.
    // Each such vertex must be inserted on the cut edge between the entry and exit crosses.
    std::vector<std::pair<std::size_t, Point>> interior_verts; // (polyline_idx, point)
    for (std::size_t k = 0; k < n_poly; ++k)
    {
      if (pointStrictlyInsideElem(polyline[k], *orig_elem, tol))
        interior_verts.emplace_back(k, polyline[k]);
    }

    // Build the interior path in polyline order: from the entry hit's polyline_pos to the exit
    // hit's polyline_pos (going forward along the polyline, wrapping if needed). Forward order is
    // used when retained = inside cutter (REMOVE_OUTSIDE); reversed when retained = outside cutter
    // (REMOVE_INSIDE).
    Real entry_pos = 0.0, exit_pos = 0.0;
    for (const auto & hi : hit_info)
    {
      if (hi.is_entry)
        entry_pos = hi.polyline_pos;
      else
        exit_pos = hi.polyline_pos;
    }
    std::vector<std::size_t> interior_path_forward; // polyline indices ordered entry -> exit
    {
      // Sort interior verts by polyline_idx
      std::sort(interior_verts.begin(),
                interior_verts.end(),
                [](const auto & a, const auto & b) { return a.first < b.first; });
      // Collect verts whose polyline_idx lies strictly between entry_pos and exit_pos in the
      // forward direction along the polyline (handling wrap-around).
      auto in_range_forward = [entry_pos, exit_pos](Real pos) -> bool
      {
        if (entry_pos < exit_pos)
          return pos > entry_pos && pos < exit_pos;
        // Wraps around: entry_pos > exit_pos
        return pos > entry_pos || pos < exit_pos;
      };
      if (entry_pos <= exit_pos)
      {
        for (const auto & [idx, p] : interior_verts)
          if (in_range_forward(static_cast<Real>(idx)))
            interior_path_forward.push_back(idx);
      }
      else
      {
        // Wrap: emit indices > entry_pos first (in ascending), then indices < exit_pos.
        for (const auto & [idx, p] : interior_verts)
          if (static_cast<Real>(idx) > entry_pos)
            interior_path_forward.push_back(idx);
        for (const auto & [idx, p] : interior_verts)
          if (static_cast<Real>(idx) < exit_pos)
            interior_path_forward.push_back(idx);
      }
    }
    // Reverse for retain_outside (REMOVE_INSIDE)
    std::vector<std::size_t> interior_path_reverse(interior_path_forward.rbegin(),
                                                   interior_path_forward.rend());

    // Map from polyline_idx -> point (for looking up Point objects)
    std::map<std::size_t, Point> polyline_idx_to_point;
    for (const auto & [idx, p] : interior_verts)
      polyline_idx_to_point[idx] = p;

    std::vector<std::vector<boundary_id_type>> elem_side_list(n_v);
    {
      auto range = std::equal_range(bdry_side_list.begin(),
                                    bdry_side_list.end(),
                                    elem_id,
                                    MooseMeshElementConversionUtils::BCTupleKeyComp());
      for (auto i = range.first; i != range.second; ++i)
        elem_side_list[std::get<1>(*i)].push_back(std::get<2>(*i));
    }

    auto build_polygon = [&](int retain_kind) -> std::pair<std::vector<Node *>, std::vector<int>>
    {
      std::vector<Node *> out;
      std::vector<int> origin;
      auto emit = [&out, &origin](Node * n, int side)
      {
        if (!out.empty() && out.back() == n)
        {
          origin.back() = side;
          return;
        }
        out.push_back(n);
        origin.push_back(side);
      };

      // Interior path order along the cut: forward (entry -> exit) for retain_inside, reverse
      // (exit -> entry) for retain_outside. Inserted after the "cross with origin=-1" emit point.
      const auto & interior_path =
          retain_kind == 1 ? interior_path_forward : interior_path_reverse;

      for (const auto i : make_range(n_v))
      {
        const auto j = (i + 1) % n_v;
        Node * vi = orig_elem->node_ptr(i);
        Node * vj = orig_elem->node_ptr(j);
        const bool i_ret = is_retained(cls[i], retain_kind);
        const bool j_ret = is_retained(cls[j], retain_kind);

        if (i_ret && j_ret)
        {
          emit(vi, static_cast<int>(i));
        }
        else if (i_ret && !j_ret)
        {
          emit(vi, static_cast<int>(i));
          if (edge_hits[i].empty())
            throw MooseException(
                "XYCutMeshByMeshGenerator: classification disagrees with edge intersection "
                "count; degenerate geometry. Try snap_tol or mesh refinement.");
          const auto & h = edge_hits[i].front();
          Node * cross = (cls[i] == 0)
                             ? vi
                             : (cls[j] == 0 ? vj : get_cross_node(vi, vj, h.seg, h.p));
          emit(cross, -1);
          // Insert interior cutter polyline vertices on the cut edge.
          for (const auto idx : interior_path)
            emit(get_interior_node(idx, polyline_idx_to_point.at(idx)), -1);
        }
        else if (!i_ret && j_ret)
        {
          if (edge_hits[i].empty())
            throw MooseException(
                "XYCutMeshByMeshGenerator: classification disagrees with edge intersection "
                "count; degenerate geometry. Try snap_tol or mesh refinement.");
          const auto & h = edge_hits[i].front();
          Node * cross = (cls[j] == 0)
                             ? vj
                             : (cls[i] == 0 ? vi : get_cross_node(vi, vj, h.seg, h.p));
          emit(cross, static_cast<int>(i));
        }
      }
      return {std::move(out), std::move(origin)};
    };

    auto add_single_polygon = [&](const std::vector<Node *> & out,
                                  const std::vector<int> & origin,
                                  subdomain_id_type new_sid)
    {
      if (out.size() < 3)
        throw MooseException("XYCutMeshByMeshGenerator: a clipped polygon has fewer than 3 "
                             "vertices; degenerate geometry.");
      auto polygon = std::make_unique<libMesh::C0Polygon>(out.size());
      for (const auto k : index_range(out))
        polygon->set_node(k, out[k]);
      polygon->subdomain_id() = new_sid;
      Elem * new_elem = primary.add_elem(std::move(polygon));
      for (const auto k : index_range(out))
      {
        const int side = origin[k];
        if (side == -1)
          boundary_info.add_side(new_elem, k, new_boundary_id);
        else if (side >= 0)
          for (const auto & bid : elem_side_list[side])
            boundary_info.add_side(new_elem, k, bid);
        // side == -2: internal diagonal from a split, no boundary id added.
      }
    };

    // Detect reflex vertices (CCW polygon). Returns indices of reflex vertices.
    auto find_reflex_vertices = [](const std::vector<Node *> & out) -> std::vector<int>
    {
      std::vector<int> reflex;
      const int n = static_cast<int>(out.size());
      for (int i = 0; i < n; ++i)
      {
        const Point & a = *out[(i + n - 1) % n];
        const Point & b = *out[i];
        const Point & c = *out[(i + 1) % n];
        const Real cz = (b(0) - a(0)) * (c(1) - b(1)) - (b(1) - a(1)) * (c(0) - b(0));
        if (cz < -libMesh::TOLERANCE * libMesh::TOLERANCE)
          reflex.push_back(i);
      }
      return reflex;
    };

    // Check if a 2D segment p-q lies inside the polygon (strictly, not crossing any non-incident
    // edge of the polygon).
    auto diagonal_inside_polygon =
        [](const std::vector<Node *> & out, int from_idx, int to_idx, Real ptol) -> bool
    {
      const int n = static_cast<int>(out.size());
      const Point & p = *out[from_idx];
      const Point & q = *out[to_idx];
      // Check intersection with each non-incident polygon edge.
      for (int i = 0; i < n; ++i)
      {
        const int j = (i + 1) % n;
        if (i == from_idx || j == from_idx || i == to_idx || j == to_idx)
          continue;
        const Point & a = *out[i];
        const Point & b = *out[j];
        Real t, s;
        Point hit_pt;
        if (segmentSegmentIntersect2D(p, q, a, b, ptol, t, s, hit_pt))
          if (t > ptol && t < 1.0 - ptol && s > ptol && s < 1.0 - ptol)
            return false;
      }
      // Check midpoint of diagonal is inside polygon (point-in-polygon via ray casting).
      const Point mid((p(0) + q(0)) * 0.5, (p(1) + q(1)) * 0.5, 0.0);
      bool inside = false;
      for (int i = 0, j = n - 1; i < n; j = i++)
      {
        const Point & vi = *out[i];
        const Point & vj = *out[j];
        if (((vi(1) > mid(1)) != (vj(1) > mid(1))) &&
            (mid(0) <
             (vj(0) - vi(0)) * (mid(1) - vi(1)) / (vj(1) - vi(1) + 1e-30) + vi(0)))
          inside = !inside;
      }
      return inside;
    };

    // Split a polygon (possibly non-convex) into one or more convex polygons. Recursive: each
    // resulting polygon is convex. Returns pairs of (out, origin).
    using PolyPair = std::pair<std::vector<Node *>, std::vector<int>>;
    std::function<std::vector<PolyPair>(std::vector<Node *>, std::vector<int>)> split_to_convex;
    split_to_convex = [&](std::vector<Node *> out_, std::vector<int> origin_) -> std::vector<PolyPair>
    {
      auto reflex = find_reflex_vertices(out_);
      if (reflex.empty())
        return {{std::move(out_), std::move(origin_)}};

      const int n = static_cast<int>(out_.size());
      const int r = reflex.front(); // process the first reflex vertex
      // Try non-adjacent vertices as the diagonal target, starting from the "opposite" vertex
      // and fanning outward.
      int chosen = -1;
      for (int offset = n / 2; offset > 1; --offset)
      {
        for (int sign : {+1, -1})
        {
          const int cand = ((r + sign * offset) % n + n) % n;
          if (cand == r || cand == (r + 1) % n || cand == (r + n - 1) % n)
            continue;
          if (diagonal_inside_polygon(out_, r, cand, tol))
          {
            chosen = cand;
            break;
          }
        }
        if (chosen >= 0)
          break;
      }
      if (chosen < 0)
        throw MooseException(
            "XYCutMeshByMeshGenerator: cannot split a non-convex retained polygon into convex "
            "pieces (no valid diagonal found). Refine the primary mesh or simplify the cutter.");

      // Build two halves: from r to chosen (CCW), and from chosen to r (CCW).
      // Closing edge of each half is the diagonal (origin = -2).
      std::vector<Node *> p1_out, p2_out;
      std::vector<int> p1_origin, p2_origin;
      int i = r;
      while (true)
      {
        p1_out.push_back(out_[i]);
        if (i == chosen)
        {
          p1_origin.push_back(-2);
          break;
        }
        p1_origin.push_back(origin_[i]);
        i = (i + 1) % n;
      }
      i = chosen;
      while (true)
      {
        p2_out.push_back(out_[i]);
        if (i == r)
        {
          p2_origin.push_back(-2);
          break;
        }
        p2_origin.push_back(origin_[i]);
        i = (i + 1) % n;
      }

      // Recurse on each half (each may still be non-convex if multi-reflex original).
      auto r1 = split_to_convex(std::move(p1_out), std::move(p1_origin));
      auto r2 = split_to_convex(std::move(p2_out), std::move(p2_origin));
      r1.insert(r1.end(),
                std::make_move_iterator(r2.begin()),
                std::make_move_iterator(r2.end()));
      return r1;
    };

    auto add_polygon = [&](std::vector<Node *> & out,
                           std::vector<int> & origin,
                           subdomain_id_type new_sid)
    {
      auto pieces = split_to_convex(out, origin);
      for (auto & [piece_out, piece_origin] : pieces)
        add_single_polygon(piece_out, piece_origin, new_sid);
    };

    if (do_outside)
    {
      auto [out, origin] = build_polygon(0);
      add_polygon(out, origin, orig_elem->subdomain_id() + outside_shift);
    }
    if (do_inside)
    {
      auto [out, origin] = build_polygon(1);
      add_polygon(out, origin, orig_elem->subdomain_id() + inside_shift);
    }

    orig_elem->subdomain_id() = block_id_to_remove;
  }

  // Apply subdomain name suffixes for each populated shifted subdomain
  auto apply_suffix = [&](subdomain_id_type shift, const SubdomainName & suffix)
  {
    if (suffix.empty())
      return;
    for (const auto & sub_id : original_subdomain_ids)
    {
      const subdomain_id_type new_sid = sub_id + shift;
      if (!MooseMeshUtils::hasSubdomainID(primary, new_sid))
        continue;
      const SubdomainName base = primary.subdomain_name(sub_id).empty()
                                     ? std::to_string(sub_id)
                                     : primary.subdomain_name(sub_id);
      const SubdomainName new_name = base + '_' + suffix;
      if (MooseMeshUtils::hasSubdomainName(primary, new_name))
        throw MooseException("The new subdomain name already exists in the mesh.");
      primary.subdomain_name(new_sid) = new_name;
    }
  };

  if (do_outside)
    apply_suffix(outside_shift, outside_suffix);
  if (do_inside)
    apply_suffix(inside_shift, inside_suffix);

  // Delete elements marked for removal
  for (auto elem_it = primary.active_subdomain_elements_begin(block_id_to_remove);
       elem_it != primary.active_subdomain_elements_end(block_id_to_remove);
       ++elem_it)
    primary.delete_elem(*elem_it);
  primary.contract();
}

} // namespace MooseMeshXYMeshCuttingUtils
