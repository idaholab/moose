//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TraceRayTools.h"

// MOOSE includes
#include "MooseTypes.h"
#include "MooseUtils.h"

// libMesh includes
#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex20.h"
#include "libmesh/cell_hex27.h"
#include "libmesh/cell_prism6.h"
#include "libmesh/cell_prism15.h"
#include "libmesh/cell_prism18.h"
#include "libmesh/cell_pyramid5.h"
#include "libmesh/cell_pyramid13.h"
#include "libmesh/cell_pyramid14.h"
#include "libmesh/cell_tet4.h"
#include "libmesh/cell_tet10.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/enum_to_string.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_tri3.h"
#include "libmesh/mesh.h"
#include "libmesh/remote_elem.h"
#include "libmesh/tensor_value.h"

namespace TraceRayTools
{

const std::set<int> TRACEABLE_ELEMTYPES = {HEX8,
                                           HEX20,
                                           HEX27,
                                           QUAD4,
                                           QUAD8,
                                           QUAD9,
                                           TET4,
                                           TET10,
                                           TRI3,
                                           TRI6,
                                           EDGE2,
                                           EDGE3,
                                           EDGE4,
                                           PYRAMID5,
                                           PYRAMID13,
                                           PYRAMID14,
                                           PRISM6,
                                           PRISM15,
                                           PRISM18};
const std::set<int> ADAPTIVITY_TRACEABLE_ELEMTYPES = {QUAD4, HEX8, TRI3, TET4, EDGE2};

bool
lineLineIntersect2D(const Point & start,
                    const Point & direction,
                    const Real length,
                    const Point & v0,
                    const Point & v1,
                    Point & intersection_point,
                    Real & intersection_distance,
                    SegmentVertices & segment_vertex
#ifdef DEBUG_RAY_INTERSECTIONS
                    ,
                    const bool debug
#endif
)

{
  // TODO: consider using hmax scaling here
  mooseAssert(segment_vertex == SEGMENT_VERTEX_NONE, "Vertex should be none");
  debugRaySimple("Called lineLineIntersect2D()");
  debugRaySimple("  start = ", start);
  debugRaySimple("  direction = ", direction);
  debugRaySimple("  length = ", length);
  debugRaySimple("  v0 = ", v0);
  debugRaySimple("  v1 = ", v1);

  const auto r = direction * length;
  const auto s = v1 - v0;

  const auto rxs = r(0) * s(1) - r(1) * s(0);
  debugRaySimple("  rxs = ", rxs);

  // Lines are parallel or colinear
  if (std::abs(rxs) < TRACE_TOLERANCE)
    return false;

  const auto v0mu0 = v0 - start;

  const auto t = (v0mu0(0) * s(1) - v0mu0(1) * s(0)) / rxs;
  debugRaySimple("  t = ", t);
  if (0 >= t + TRACE_TOLERANCE || t - TRACE_TOLERANCE > 1.0)
  {
    debugRaySimple("lineLineIntersect2D did not intersect: t out of range");
    return false;
  }

  const auto u = (v0mu0(0) * r(1) - v0mu0(1) * r(0)) / rxs;
  debugRaySimple("  u = ", u);
  if (0 < u + TRACE_TOLERANCE && u - TRACE_TOLERANCE <= 1.0)
  {
    intersection_point = start + r * t;
    intersection_distance = t * length;

    if (u < TRACE_TOLERANCE)
      segment_vertex = SEGMENT_VERTEX_0;
    else if (u > 1.0 - TRACE_TOLERANCE)
      segment_vertex = SEGMENT_VERTEX_1;

    debugRaySimple("lineLineIntersect2D intersected with:");
    debugRaySimple("  intersection_distance = ", intersection_point);
    debugRaySimple("  intersection_distance = ", intersection_distance);
    debugRaySimple("  segment_vertex = ", Utility::enum_to_string(segment_vertex));

    return true;
  }

  // Not parallel, but don't intersect
  debugRaySimple("lineLineIntersect2d() did not intersect: u out of range");
  return false;
}

void
findPointNeighbors(
    const Elem * const elem,
    const Point & point,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & neighbor_set,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & untested_set,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & next_untested_set,
    std::vector<const Elem *> active_neighbor_children,
    std::vector<NeighborInfo> & info)
{
  mooseAssert(elem->contains_point(point), "Doesn't contain point");

  info.clear();

  // Helper for avoiding extraneous allocation when building side elements
  std::unique_ptr<const Elem> side_helper;

  auto contains_point = [&point, &info, &side_helper](const Elem * const candidate)
  {
    if (candidate->contains_point(point))
    {
      std::vector<unsigned short> sides;
      for (const auto s : candidate->side_index_range())
      {
        candidate->build_side_ptr(side_helper, s);
        if (side_helper->contains_point(point))
          sides.push_back(s);
      }

      if (!sides.empty())
      {
        info.emplace_back(candidate, std::move(sides));
        return true;
      }
    }

    return false;
  };

  // Fill info for the element that was passed in
  contains_point(elem);

  findNeighbors(elem,
                neighbor_set,
                untested_set,
                next_untested_set,
                active_neighbor_children,
                contains_point);

#ifndef NDEBUG
  // In non-opt modes, verify that we found all of the correct neighbors
  // using the more expensive libMesh routine
  std::set<const Elem *> point_neighbors;
  elem->find_point_neighbors(point, point_neighbors);
  for (const auto & point_neighbor : point_neighbors)
    if (!neighbor_set.contains(point_neighbor))
      mooseError("Missed a point neighbor");
#endif
}

void
findNodeNeighbors(
    const Elem * const elem,
    const Node * const node,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & neighbor_set,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & untested_set,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & next_untested_set,
    std::vector<const Elem *> active_neighbor_children,
    std::vector<NeighborInfo> & info)
{
  mooseAssert(elem->get_node_index(node) != libMesh::invalid_uint, "Doesn't contain node");

  info.clear();

  // Helper for avoiding extraneous allocations when building side elements
  std::unique_ptr<const Elem> side_helper;

  auto contains_node = [&node, &elem, &info, &side_helper](const Elem * const candidate)
  {
    // Candidate has this node and it is a vertex - add sides that contain said node
    const auto n = candidate->get_node_index(node);
    if (n != invalid_uint && candidate->is_vertex(n))
    {
      std::vector<unsigned short> sides;
      for (const auto s : candidate->side_index_range())
        if (candidate->is_node_on_side(n, s))
          sides.push_back(s);

      if (sides.empty())
        mooseError("Failed to find a side containing node");

      info.emplace_back(candidate, std::move(sides));
      return true;
    }
    // In the case of a less refined candidate, the node can be a hanging node. The candidate
    // will only ever have the hanging node if it is less refined.
    if (candidate->level() < elem->level() && candidate->contains_point(*node))
    {
      std::vector<unsigned short> sides;
      for (const auto s : candidate->side_index_range())
      {
        candidate->build_side_ptr(side_helper, s);
        if (side_helper->contains_point(*node))
          sides.push_back(s);
      }

      if (!sides.empty())
      {
        info.emplace_back(candidate, std::move(sides));
        return true;
      }
    }

    return false;
  };

  // Fill info for the element that was passed in
  contains_node(elem);

  findNeighbors(
      elem, neighbor_set, untested_set, next_untested_set, active_neighbor_children, contains_node);

#ifndef NDEBUG
  // In non-opt modes, verify that we found all of the correct neighbors
  // using the more expensive libMesh routine
  std::set<const Elem *> point_neighbors;
  elem->find_point_neighbors(*node, point_neighbors);
  for (const auto & point_neighbor : point_neighbors)
    for (const auto & neighbor_node : point_neighbor->node_ref_range())
      if (node == &neighbor_node && !neighbor_set.contains(point_neighbor))
        mooseError("Missed a node neighbor");
#endif
}

void
findEdgeNeighbors(
    const Elem * const elem,
    const Node * const node1,
    const Node * const node2,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & neighbor_set,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & untested_set,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & next_untested_set,
    std::vector<const Elem *> active_neighbor_children,
    std::vector<NeighborInfo> & info)
{
  mooseAssert(elem->get_node_index(node1) != libMesh::invalid_uint, "Doesn't contain node");
  mooseAssert(elem->get_node_index(node2) != libMesh::invalid_uint, "Doesn't contain node");

  info.clear();

  // The length for this edge used for checking if a point is contained within said edge
  const Real edge_length = ((Point)*node1 - (Point)*node2).norm();

  // Lambda that returns whether or not a candidate element contains an edge that is within the
  // target edge defined by node1 and node2. Also fills "info" if a match is found
  auto within_edge = [&elem, &node1, &node2, &edge_length, &info](const Elem * const candidate)
  {
    switch (candidate->type())
    {
      case HEX8:
        return findEdgeNeighborsWithinEdgeInternal<Hex8>(
            candidate, elem, node1, node2, edge_length, info);
      case TET4:
        return findEdgeNeighborsWithinEdgeInternal<Tet4>(
            candidate, elem, node1, node2, edge_length, info);
      case PYRAMID5:
        return findEdgeNeighborsWithinEdgeInternal<Pyramid5>(
            candidate, elem, node1, node2, edge_length, info);
      case PRISM6:
        return findEdgeNeighborsWithinEdgeInternal<Prism6>(
            candidate, elem, node1, node2, edge_length, info);
      case HEX20:
        return findEdgeNeighborsWithinEdgeInternal<Hex20>(
            candidate, elem, node1, node2, edge_length, info);
      case HEX27:
        return findEdgeNeighborsWithinEdgeInternal<Hex27>(
            candidate, elem, node1, node2, edge_length, info);
      case TET10:
        return findEdgeNeighborsWithinEdgeInternal<Tet10>(
            candidate, elem, node1, node2, edge_length, info);
      case PYRAMID13:
        return findEdgeNeighborsWithinEdgeInternal<Pyramid13>(
            candidate, elem, node1, node2, edge_length, info);
      case PYRAMID14:
        return findEdgeNeighborsWithinEdgeInternal<Pyramid14>(
            candidate, elem, node1, node2, edge_length, info);
      case PRISM15:
        return findEdgeNeighborsWithinEdgeInternal<Prism15>(
            candidate, elem, node1, node2, edge_length, info);
      case PRISM18:
        return findEdgeNeighborsWithinEdgeInternal<Prism18>(
            candidate, elem, node1, node2, edge_length, info);
      default:
        mooseError("Element type ",
                   Utility::enum_to_string(candidate->type()),
                   " not supported in TraceRayTools::findEdgeNeighbors()");
    }
  };

  // Fill info for the element that was passed in
  within_edge(elem);

  findNeighbors(
      elem, neighbor_set, untested_set, next_untested_set, active_neighbor_children, within_edge);
}

const Elem *
childContainingPointOnSide(const Elem * elem, const Point & point, const unsigned short side)
{
  mooseAssert(!elem->active(), "Should be inactive");
  mooseAssert(elem->side_ptr(side)->contains_point(point), "Side should contain point");

  for (unsigned int c = 0; c < elem->n_children(); ++c)
  {
    if (!elem->is_child_on_side(c, side))
      continue;

    const auto child = elem->child_ptr(c);
    // Experience shows that we need to loosen this tolerance just a little
    // The default is libMesh::TOLERANCE = 1e-6
    if (child->close_to_point(point, 5.e-5))
    {
      if (child->active())
        return child;
      else
        return childContainingPointOnSide(child, point, side);
    }
  }

  mooseError("Failed to find child containing point on side");
}

const Elem *
getActiveNeighbor(const Elem * elem, const unsigned short side, const Point & point)
{
  const auto neighbor = elem->neighbor_ptr(side);
  if (!neighbor || neighbor->active())
    return neighbor;

  // There is adaptivity... need to find the active child that contains the point
  const auto neighbor_side = neighbor->which_neighbor_am_i(elem);
  return childContainingPointOnSide(neighbor, point, neighbor_side);
}

bool
intersectTriangle(const Point & start,
                  const Point & direction,
                  const Elem * const elem,
                  const unsigned short v0,
                  const unsigned short v1,
                  const unsigned short v2,
                  Real & intersection_distance,
                  ElemExtrema & intersected_extrema,
                  const Real hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                  ,
                  const bool debug
#endif
)
{
  debugRaySimple("intersectTriangle() called:");
  debugRaySimple("  start = ", start);
  debugRaySimple("  direction = ", direction);
  debugRaySimple("  elem->id() = ", elem->id());
  debugRaySimple("  v0 = ", v0, " at ", elem->point(v0));
  debugRaySimple("  v1 = ", v1, " at ", elem->point(v1));
  debugRaySimple("  v2 = ", v2, " at ", elem->point(v2));
  debugRaySimple("  hmax = ", hmax);
  mooseAssert(elem->is_vertex(v0), "Not a vertex");
  mooseAssert(elem->is_vertex(v1), "Not a vertex");
  mooseAssert(elem->is_vertex(v2), "Not a vertex");

  // We are scaling the whole element (start, v0, v1, v2) by 1 / hmax as an alternative to scaling
  // the tolerance by hmax. If an intersection is found, the resulting intersection distance is
  // then scaled by hmax to reverse the original scaling.
  const auto inv_hmax = 1.0 / hmax;

  const auto & v0_point = elem->point(v0);

  const auto edge1 = (elem->point(v1) - v0_point) * inv_hmax;
  const auto edge2 = (elem->point(v2) - v0_point) * inv_hmax;

  const auto pvec = direction.cross(edge2);

  auto det = edge1 * pvec;
  debugRaySimple("  det = ", det);
  if (det < TRACE_TOLERANCE)
  {
    debugRaySimple("intersectTriangle() did not intersect: det < tol");
    return false;
  }

  const auto tvec = (start - v0_point) * inv_hmax;
  const auto u = tvec * pvec;
  debugRaySimple("  u = ", u);
  debugRaySimple("  u / det = ", u / det);
  if (u < -TRACE_TOLERANCE || u > det + TRACE_TOLERANCE)
  {
    debugRaySimple("intersectTriangle() did not intersect: u out of range");
    return false;
  }

  const auto qvec = tvec.cross(edge1);
  const auto v = direction * qvec;
  debugRaySimple("  v = ", v);
  debugRaySimple("  v / det = ", v / det);
  debugRaySimple("  (u + v) / det = ", (u + v) / det);
  if (v < -TRACE_TOLERANCE || u + v > det + TRACE_TOLERANCE)
  {
    debugRaySimple("intersectTriangle() did not intersect: v out of range");
    return false;
  }

  const auto possible_distance = (edge2 * qvec) / det;
  debugRaySimple("  possible_distance = ", possible_distance);
  if (possible_distance <= TRACE_TOLERANCE)
  {
    debugRaySimple("intersectTriangle() did not intersect: distance too small");
    return false;
  }

  // Recall that the element was scaled by (1 / hmax), reverse this scaling by
  // scaling the intersection distance by hmax
  intersection_distance = possible_distance * hmax;

  // Here, u and v aren't truly u and v. The actual u and v are obtained with:
  // u = u / det and v = v / det -> move det to the RHS to avoid division
  if (u < TRACE_TOLERANCE * det)
  {
    if (v < TRACE_TOLERANCE * det) // u = 0, v = 0
      intersected_extrema.setVertex(v0);
    else if (v > (1.0 - TRACE_TOLERANCE) * det) // u = 0, v = 1
      intersected_extrema.setVertex(v2);
    else // u = 0
      intersected_extrema.setEdge(v0, v2);
  }
  else if (v < TRACE_TOLERANCE * det)
  {
    if (u > (1.0 - TRACE_TOLERANCE) * det) // u = 1, v = 0
      intersected_extrema.setVertex(v1);
    else // v = 0
      intersected_extrema.setEdge(v0, v1);
  }
  else if ((u + v > (1.0 - TRACE_TOLERANCE) * det)) // u + v = 1
    intersected_extrema.setEdge(v1, v2);

  debugRaySimple("intersectTriangle() intersected with:");
  debugRaySimple("  intersection_distance = ", intersection_distance);
  debugRaySimple("  intersected_extrema = ", intersected_extrema);

  return true;
}

bool
intersectQuad(const Point & start,
              const Point & direction,
              const Elem * const elem,
              const unsigned short v00,
              const unsigned short v10,
              const unsigned short v11,
              const unsigned short v01,
              Real & intersection_distance,
              ElemExtrema & intersected_extrema,
              const Real hmax
#ifdef DEBUG_RAY_INTERSECTIONS
              ,
              const bool debug
#endif
)
{
  mooseAssert(intersected_extrema.isInvalid(), "Should be invalid");
  debugRaySimple("intersectQuad() called:");
  debugRaySimple("  start = ", start);
  debugRaySimple("  direction = ", direction);
  debugRaySimple("  elem->id() = ", elem->id());
  debugRaySimple("  v00 = ", v00, " at ", elem->point(v00));
  debugRaySimple("  v10 = ", v10, " at ", elem->point(v10));
  debugRaySimple("  v11 = ", v11, " at ", elem->point(v11));
  debugRaySimple("  v01 = ", v01, " at ", elem->point(v01));

  // NOTE discovered by @GiudGiud: In the case that you have
  // a glancing intersection (the direction is within the plane
  // of the face), you could possibly miss a further intersection
  // on the second triangle. In reality, this should not be
  // a problem because we check all sides of the element, so
  // we would find a further intersection in the future.

  // First check the triangle contained by v00, v10, v11
  bool intersects = intersectTriangle(start,
                                      direction,
                                      elem,
                                      v00,
                                      v10,
                                      v11,
                                      intersection_distance,
                                      intersected_extrema,
                                      hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                                      ,
                                      debug
#endif
  );
  // If no intersection, check the triangle contained by v11, v01, v00
  if (!intersects)
    intersects = intersectTriangle(start,
                                   direction,
                                   elem,
                                   v11,
                                   v01,
                                   v00,
                                   intersection_distance,
                                   intersected_extrema,
                                   hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                                   ,
                                   debug
#endif
    );

  // Because we split the quad into two triangles, we could intersect the edge v00 - v11. However,
  // that really isn't an edge - it's a diagonal across the quad. If we intersect this edge, be sure
  // to invalidate it
  if (intersects && intersected_extrema.atEdge(v00, v11))
    intersected_extrema.invalidate();

  return intersects;
}

bool
isTraceableElem(const Elem * elem)
{
  return TRACEABLE_ELEMTYPES.count(elem->type());
}

bool
isAdaptivityTraceableElem(const Elem * elem)
{
  return ADAPTIVITY_TRACEABLE_ELEMTYPES.count(elem->type());
}

unsigned short
atVertex(const Elem * elem, const Point & point)
{
  for (unsigned int v = 0; v < elem->n_vertices(); ++v)
    if (elem->point(v).absolute_fuzzy_equals(point, TRACE_TOLERANCE))
      return v;

  return RayTracingCommon::invalid_vertex;
}

template <typename T>
bool
withinEdgeTempl(const Elem * elem,
                const Point & point,
                ElemExtrema & extrema,
                const Real tolerance /* = TRACE_TOLERANCE */)
{
  mooseAssert(extrema.isInvalid(), "Should be invalid");

  for (int e = 0; e < T::num_edges; ++e)
    if (isWithinSegment(elem->point(T::edge_nodes_map[e][0]),
                        elem->point(T::edge_nodes_map[e][1]),
                        point,
                        tolerance))
    {
      extrema.setEdge(T::edge_nodes_map[e][0], T::edge_nodes_map[e][1]);
      return true;
    }

  return false;
}

bool
withinEdge(const Elem * elem,
           const Point & point,
           ElemExtrema & extrema,
           const Real tolerance /* = TRACE_TOLERANCE */)
{
  switch (elem->type())
  {
    case HEX8:
      return withinEdgeTempl<Hex8>(elem, point, extrema, tolerance);
    case TET4:
      return withinEdgeTempl<Tet4>(elem, point, extrema, tolerance);
    case PYRAMID5:
      return withinEdgeTempl<Pyramid5>(elem, point, extrema, tolerance);
    case PRISM6:
      return withinEdgeTempl<Prism6>(elem, point, extrema, tolerance);
    case HEX20:
    case HEX27:
      return withinEdgeTempl<Hex8>(elem, point, extrema, tolerance);
    case TET10:
      return withinEdgeTempl<Tet4>(elem, point, extrema, tolerance);
    case PYRAMID13:
    case PYRAMID14:
      return withinEdgeTempl<Pyramid5>(elem, point, extrema, tolerance);
    case PRISM15:
    case PRISM18:
      return withinEdgeTempl<Prism6>(elem, point, extrema, tolerance);
    default:
      mooseError("Element type ",
                 Utility::enum_to_string(elem->type()),
                 " not supported in TraceRayTools::withinEdge()");
  }

  return false;
}

unsigned short
atVertexOnSide(const Elem * elem, const Point & point, const unsigned short side)
{
  switch (elem->type())
  {
    case HEX8:
      return atVertexOnSideTempl<Hex8>(elem, point, side);
    case TET4:
      return atVertexOnSideTempl<Tet4>(elem, point, side);
    case PYRAMID5:
      return atVertexOnSideTempl<Pyramid5>(elem, point, side);
    case PRISM6:
      return atVertexOnSideTempl<Prism6>(elem, point, side);
    case QUAD4:
      return atVertexOnSideTempl<Quad4>(elem, point, side);
    case TRI3:
      return atVertexOnSideTempl<Tri3>(elem, point, side);
    case HEX20:
    case HEX27:
      return atVertexOnSideTempl<Hex8>(elem, point, side);
    case QUAD8:
    case QUAD9:
      return atVertexOnSideTempl<Quad4>(elem, point, side);
    case TRI6:
      return atVertexOnSideTempl<Tri3>(elem, point, side);
    case TET10:
      return atVertexOnSideTempl<Tet4>(elem, point, side);
    case PYRAMID13:
    case PYRAMID14:
      return atVertexOnSideTempl<Pyramid5>(elem, point, side);
    case PRISM15:
    case PRISM18:
      return atVertexOnSideTempl<Prism6>(elem, point, side);
    case EDGE2:
    case EDGE3:
    case EDGE4:
      return atVertexOnSideTempl<Edge2>(elem, point, side);
    default:
      mooseError("Element type ",
                 Utility::enum_to_string(elem->type()),
                 " not supported in TraceRayTools::atVertexOnSide()");
  }

  return RayTracingCommon::invalid_vertex;
}

template <typename T>
typename std::enable_if<!std::is_base_of<Edge, T>::value, unsigned short>::type
atVertexOnSideTempl(const Elem * elem, const Point & point, const unsigned short side)
{
  mooseAssert(side < elem->n_sides(), "Invalid side");
  mooseAssert(elem->side_ptr(side)->close_to_point(point, LOOSE_TRACE_TOLERANCE),
              "Side does not contain point");

  for (int i = 0; i < nodesPerSide<T>(side); ++i)
    if (elem->point(T::side_nodes_map[side][i]).absolute_fuzzy_equals(point, TRACE_TOLERANCE))
      return T::side_nodes_map[side][i];

  return RayTracingCommon::invalid_vertex;
}

template <typename T>
typename std::enable_if<std::is_base_of<Edge, T>::value, unsigned short>::type
atVertexOnSideTempl(const Elem * elem, const Point & point, const unsigned short side)
{
  mooseAssert(side < elem->n_sides(), "Invalid side");
  mooseAssert(elem->side_ptr(side)->close_to_point(point, LOOSE_TRACE_TOLERANCE),
              "Side does not contain point");

  if (elem->point(side).absolute_fuzzy_equals(point, TRACE_TOLERANCE))
    return side;

  return RayTracingCommon::invalid_vertex;
}

bool
withinEdgeOnSide(const Elem * const elem,
                 const Point & point,
                 const unsigned short side,
                 ElemExtrema & extrema)
{
  switch (elem->type())
  {
    case HEX8:
      return withinEdgeOnSideTempl<Hex8>(elem, point, side, extrema);
    case TET4:
      return withinEdgeOnSideTempl<Tet4>(elem, point, side, extrema);
    case PYRAMID5:
      return withinEdgeOnSideTempl<Pyramid5>(elem, point, side, extrema);
    case PRISM6:
      return withinEdgeOnSideTempl<Prism6>(elem, point, side, extrema);
    case HEX20:
    case HEX27:
      return withinEdgeOnSideTempl<Hex8>(elem, point, side, extrema);
    case TET10:
      return withinEdgeOnSideTempl<Tet4>(elem, point, side, extrema);
    case PYRAMID13:
    case PYRAMID14:
      return withinEdgeOnSideTempl<Pyramid5>(elem, point, side, extrema);
    case PRISM15:
    case PRISM18:
      return withinEdgeOnSideTempl<Prism6>(elem, point, side, extrema);
    default:
      mooseError("Element type ",
                 Utility::enum_to_string(elem->type()),
                 " not supported in TraceRayTools::withinEdgeOnSide()");
  }

  return false;
}

template <typename T>
typename std::enable_if<std::is_base_of<Cell, T>::value, bool>::type
withinEdgeOnSideTempl(const Elem * const elem,
                      const Point & point,
                      const unsigned short side,
                      ElemExtrema & extrema)
{
  mooseAssert(side < elem->n_sides(), "Invalid side");
  mooseAssert(elem->side_ptr(side)->close_to_point(point, LOOSE_TRACE_TOLERANCE),
              "Side does not contain point");
  mooseAssert(extrema.isInvalid(), "Should be invalid");

  int last_n = T::side_nodes_map[side][nodesPerSide<T>(side) - 1];

  for (int side_v = 0; side_v < nodesPerSide<T>(side); ++side_v)
    if (isWithinSegment(elem->point(last_n), elem->point(T::side_nodes_map[side][side_v]), point))
    {
      extrema.setEdge(last_n, T::side_nodes_map[side][side_v]);
      mooseAssert(extrema.buildEdge(elem)->close_to_point(point, LOOSE_TRACE_TOLERANCE),
                  "Edge doesn't contain point");
      return true;
    }
    else
      last_n = T::side_nodes_map[side][side_v];

  return false;
}

bool
withinExtremaOnSide(const Elem * const elem,
                    const Point & point,
                    const unsigned short side,
                    const unsigned int dim,
                    ElemExtrema & extrema)
{
  mooseAssert(extrema.isInvalid(), "Extrema should be invalid");
  mooseAssert(dim == elem->dim(), "Incorrect dim");

  extrema.first = atVertexOnSide(elem, point, side);
  if (extrema.atVertex())
    return true;
  if (dim == 3 && withinEdgeOnSide(elem, point, side, extrema))
    return true;

  return false;
}

bool
isWithinSegment(const Point & segment1,
                const Point & segment2,
                const Point & point,
                const Real tolerance /* = TRACE_TOLERANCE */)
{
  mooseAssert(!segment1.absolute_fuzzy_equals(segment2, TRACE_TOLERANCE), "Same endpoints");

  const auto segment_length = (segment1 - segment2).norm();
  return isWithinSegment(segment1, segment2, segment_length, point, tolerance);
}

bool
isWithinSegment(const Point & segment1,
                const Point & segment2,
                const Real segment_length,
                const Point & point,
                const Real tolerance /* = TRACE_TOLERANCE */)
{
  mooseAssert(!segment1.absolute_fuzzy_equals(segment2, TRACE_TOLERANCE), "Same endpoints");
  mooseAssert(MooseUtils::absoluteFuzzyEqual((segment1 - segment2).norm(), segment_length),
              "Invalid segment length");

  const auto diff1 = point - segment1;
  const auto diff2 = point - segment2;

  if (diff1 * diff2 > tolerance * segment_length)
    return false;

  return std::abs(diff1.norm() + diff2.norm() - segment_length) < tolerance * segment_length;
}

bool
onBoundingBoxBoundary(const BoundingBox & bbox,
                      const Point & point,
                      const unsigned int dim,
                      const Real tolerance)
{
  for (unsigned int d = 0; d < dim; ++d)
    if (MooseUtils::absoluteFuzzyEqual(point(d), bbox.min()(d), tolerance) ||
        MooseUtils::absoluteFuzzyEqual(point(d), bbox.max()(d), tolerance))
      return true;

  return false;
}
}
