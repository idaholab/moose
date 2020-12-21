//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "StaticallyAllocatedSet.h"

// Local includes
#include "DebugRay.h"
#include "ElemExtrema.h"
#include "RayTracingCommon.h"
#include "NeighborInfo.h"

// libMesh includes
#include "libmesh/face.h"
#include "libmesh/cell_hex.h"
#include "libmesh/cell_prism.h"
#include "libmesh/cell_pyramid.h"
#include "libmesh/cell_tet.h"
#include "libmesh/remote_elem.h"

namespace libMesh
{
class Mesh;
class Edge;
class Cell;
}

namespace TraceRayTools
{

/// The element types that are traceable
extern const std::set<int> TRACEABLE_ELEMTYPES;
/// The element types that are traceable with adaptivity
extern const std::set<int> ADAPTIVITY_TRACEABLE_ELEMTYPES;

/// The standard tolerance to use in tracing
const Real TRACE_TOLERANCE = 1e-8;
/// Looser tolerance for use in error checking in difficult situations
const Real LOOSE_TRACE_TOLERANCE = 1e-5;

/// Value for an invalid integer
const int INVALID_INT = std::numeric_limits<int>::max();

/**
 * Enum for the possible vertices on a segment used in lineLineIntersect2D()
 */
enum SegmentVertices
{
  SEGMENT_VERTEX_0 = 0,
  SEGMENT_VERTEX_1 = 1,
  SEGMENT_VERTEX_NONE = 7
};

/**
 * Checks for the intersection of the line u0 -> u1 with the line v0 -> v1,
 * where u0 = start and u1 = start + direction * length.
 *
 * From: https://stackoverflow.com/a/565282
 *
 * @return Whether or not the lines intersect
 *
 * @param start The start point, u0
 * @param direction The direction used to define u1
 * @param length The length used to define u1
 * @param v0 The point v0
 * @param v1 The point v1
 * @param intersection_point To be filled with the point of intersection
 * @param intersection_distance To be filled with the distance of intersection
 * @param segment_vertex To be filled with the intersected vertex, if any
 */
bool lineLineIntersect2D(const Point & start,
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
);

/**
 * Checks whether or not a point is within a line segment
 * @param segment1 The first point on the segment
 * @param segment2 The second point on the segment
 * @param point The point
 * @param tolerance The tolerance to use
 * @return If point is within the segment defined by [segment1, segment2]
 */
bool isWithinSegment(const Point & segment1,
                     const Point & segment2,
                     const Point & point,
                     const Real tolerance = TRACE_TOLERANCE);

/**
 * Checks whether or not a point is within a line segment
 * @param segment1 The first point on the segment
 * @param segment2 The second point on the segment
 * @param segment_length The segment length (for optimization if it's already computed)
 * @param point The point
 * @param tolerance The tolerance to use
 * @return If point is within the segment defined by [segment1, segment2]
 */
bool isWithinSegment(const Point & segment1,
                     const Point & segment2,
                     const Real segment_length,
                     const Point & point,
                     const Real tolerance = TRACE_TOLERANCE);

/**
 * Rewrite of the find_point_neighbors function in libMesh, instead using a statically allocated
 * set: returns the active point neighbors at p within elem.
 *
 * @param elem The element
 * @param p The point
 * @param neighbor_set The set to fill the neighbors into
 * @param untested_set Set for internal use
 * @param next_untested_set Set for internal use
 * @param active_neighbor_children Temprorary vector for use in the search
 */
void findPointNeighbors(
    const Elem * const elem,
    const Point & point,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & neighbor_set,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & untested_set,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & next_untested_set,
    std::vector<const Elem *> active_neighbor_children,
    std::vector<NeighborInfo> & info);

void findNodeNeighbors(
    const Elem * const elem,
    const Node * const node,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & neighbor_set,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & untested_set,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & next_untested_set,
    std::vector<const Elem *> active_neighbor_children,
    std::vector<NeighborInfo> & info);

void findEdgeNeighbors(
    const Elem * const elem,
    const Node * const node1,
    const Node * const node2,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & neighbor_set,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & untested_set,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & next_untested_set,
    std::vector<const Elem *> active_neighbor_children,
    std::vector<NeighborInfo> & info);

/**
 * More generalized form of the find_point_neighbors function in libMesh. Instead uses a statically
 * allocated set and accepts a functor for the rejection/acceptance of an element.
 *
 * Returns the active neighbors that fit the criteria of keep_functor.
 *
 * @param elem The element
 * @param neighbor_set The set to fill the neighbors into
 * @param untested_set Set for internal use
 * @param next_untested_set Set for internal use
 * @param active_neighbor_children Vector to use in the search
 * @param keep_functor The functor that is used to accept or reject an element
 */
template <typename KeepFunctor>
void
findNeighbors(
    const Elem * const elem,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & neighbor_set,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & untested_set,
    MooseUtils::StaticallyAllocatedSet<const Elem *, MAX_POINT_NEIGHBORS> & next_untested_set,
    std::vector<const Elem *> active_neighbor_children,
    KeepFunctor & keep_functor)
{
  mooseAssert(elem->active(), "Inactive element");

  neighbor_set.clear();
  untested_set.clear();
  next_untested_set.clear();

  neighbor_set.insert(elem);
  untested_set.insert(elem);

  while (!untested_set.empty())
  {
    // Loop over all the elements in the patch that haven't already been tested
    for (const Elem * elem : untested_set)
      for (auto current_neighbor : elem->neighbor_ptr_range())
        if (current_neighbor &&
            current_neighbor != remote_elem) // we have a real neighbor on elem side
        {
          if (current_neighbor->active()) // ... if it is active
          {
            if (!neighbor_set.contains(current_neighbor) && keep_functor(current_neighbor))
            {
              next_untested_set.insert(current_neighbor);
              neighbor_set.insert(current_neighbor);
            }
          }
#ifdef LIBMESH_ENABLE_AMR
          else // ... the neighbor is *not* active,
          {    // ... so add *all* neighboring
               // active children that touch p
            current_neighbor->active_family_tree_by_neighbor(active_neighbor_children, elem);

            for (const Elem * current_child : active_neighbor_children)
              if (!neighbor_set.contains(current_child) && keep_functor(current_child))
              {
                next_untested_set.insert(current_child);
                neighbor_set.insert(current_child);
              }
          }
#endif // #ifdef LIBMESH_ENABLE_AMR
        }
    untested_set.swap(next_untested_set);
    next_untested_set.clear();
  }
}

template <typename T>
bool
findEdgeNeighborsWithinEdgeInternal(const Elem * const candidate,
                                    const Elem * const elem,
                                    const Node * const vertex1,
                                    const Node * const vertex2,
                                    const Real edge_length,
                                    std::vector<NeighborInfo> & info)
{
  // If we have the type already: use it so we can avoid all of the virtual
  // calls that we would be making on Elem
  mooseAssert(dynamic_cast<const T *>(candidate), "Incorrect elem type");
  const T * const T_candidate = static_cast<const T *>(candidate);

  // Local index of our two vertices of interest (if any)
  auto v1 = T_candidate->get_node_index(vertex1);
  auto v2 = T_candidate->get_node_index(vertex2);
  // Whether or not we have said vertices
  const bool has_v1 = v1 != invalid_uint;
  const bool has_v2 = v2 != invalid_uint;

  // If the candidate has both vertices, it shares the complete edge
  if (has_v1 && has_v2)
  {
    // Add the sides that contain said edge
    for (MooseIndex(T::num_edges) e = 0; e < T::num_edges; ++e)
      if (T_candidate->is_node_on_edge(v1, e) && T_candidate->is_node_on_edge(v2, e))
      {
        std::vector<unsigned short> sides(2);
        sides[0] = T::edge_sides_map[e][0];
        sides[1] = T::edge_sides_map[e][1];
        info.emplace_back(candidate, std::move(sides), 0, 1);
        return true;
      }

    mooseError("Failed to find a side that the vertices are on");
  }

  const auto n_vertices = T_candidate->n_vertices();

  // If we only have one of the vertices, we can still be contained within the edge if we have
  // another vertex that is within the edge
  if (has_v1 || has_v2)
  {
    // Local index of the vertex that the candidate and the target edge have in common
    const auto common_v = has_v1 ? v1 : v2;

    // See if another vertex that isn't the common node is contained
    MooseIndex(n_vertices) other_v;
    for (other_v = 0; other_v < n_vertices; ++other_v)
      if (other_v != common_v &&
          isWithinSegment(*vertex1, *vertex2, edge_length, T_candidate->point(other_v)))
        break;

    // If we have the common vertex and another vertex within the target edge, use the sides
    // that contain both of those vertices
    if (other_v != n_vertices)
    {
      for (MooseIndex(T::num_edges) e = 0; e < T::num_edges; ++e)
        if (T_candidate->is_node_on_edge(common_v, e) && T_candidate->is_node_on_edge(other_v, e))
        {
          std::vector<unsigned short> sides(2);
          sides[0] = T::edge_sides_map[e][0];
          sides[1] = T::edge_sides_map[e][1];

          info.emplace_back(
              candidate,
              std::move(sides),
              has_v1 ? 0 : (T_candidate->point(other_v) - (Point)*vertex1).norm() / edge_length,
              has_v1 ? (T_candidate->point(other_v) - (Point)*vertex1).norm() / edge_length : 1);
          return true;
        }
      mooseError("Failed to find a side that the vertices are on");
    }
    else if (T_candidate->level() < elem->level())
    {
      for (MooseIndex(T::num_edges) e = 0; e < T::num_edges; ++e)
        if (T_candidate->is_node_on_edge(common_v, e) &&
            isWithinSegment(T_candidate->point(T::edge_nodes_map[e][0]),
                            T_candidate->point(T::edge_nodes_map[e][1]),
                            has_v1 ? *vertex2 : *vertex1))
        {
          other_v = T::edge_nodes_map[e][0] == common_v ? T::edge_nodes_map[e][1]
                                                        : T::edge_nodes_map[e][0];
          std::vector<unsigned short> sides(2);
          sides[0] = T::edge_sides_map[e][0];
          sides[1] = T::edge_sides_map[e][1];

          info.emplace_back(
              candidate,
              std::move(sides),
              has_v1 ? 0 : (T_candidate->point(other_v) - (Point)*vertex1).norm() / edge_length,
              has_v1 ? (T_candidate->point(other_v) - (Point)*vertex1).norm() / edge_length : 1);
          return true;
        }

      return false;
    }
  }
  // When the candidate level is less, one of our vertices could be in a candidate's face and the
  // other could be within one of the candidate's edges
  else if (T_candidate->level() < elem->level())
  {
    auto v1_edge = RayTracingCommon::invalid_edge;
    auto v2_edge = RayTracingCommon::invalid_edge;

    // See if any of the edges contain one of the vertices
    for (MooseIndex(T::num_edges) e = 0; e < T::num_edges; ++e)
    {
      if (v1_edge == RayTracingCommon::invalid_edge &&
          isWithinSegment(T_candidate->point(T::edge_nodes_map[e][0]),
                          T_candidate->point(T::edge_nodes_map[e][1]),
                          *vertex1))
        v1_edge = e;
      if (v2_edge == RayTracingCommon::invalid_edge &&
          isWithinSegment(T_candidate->point(T::edge_nodes_map[e][0]),
                          T_candidate->point(T::edge_nodes_map[e][1]),
                          *vertex2))
        v2_edge = e;
    }

    const auto v1_within = v1_edge != RayTracingCommon::invalid_edge;
    const auto v2_within = v2_edge != RayTracingCommon::invalid_edge;

    if (v1_within && v2_within)
    {
      mooseAssert(v1_edge == v2_edge, "Vertices should be contained in same edge");

      std::vector<unsigned short> sides(2);
      sides[0] = T::edge_sides_map[v1_edge][0];
      sides[1] = T::edge_sides_map[v1_edge][1];
      info.emplace_back(candidate, std::move(sides), 0, 1);
      return true;
    }
    else if (v1_within || v2_within)
    {
      const auto in_edge = v1_within ? v1_edge : v2_edge;
      const Point & check_point = v1_within ? *vertex2 : *vertex1;

      std::vector<unsigned short> sides(1);
      Real lower_bound = 0;
      Real upper_bound = 1;

      if (candidate->side_ptr(T::edge_sides_map[in_edge][0])->contains_point(check_point))
        sides[0] = T::edge_sides_map[in_edge][0];
      else if (candidate->side_ptr(T::edge_sides_map[in_edge][1])->contains_point(check_point))
        sides[0] = T::edge_sides_map[in_edge][1];
      else
      {
        const Real point_bound = v1_within ? 0 : 1;
        lower_bound = point_bound;
        upper_bound = point_bound;
        sides[0] = T::edge_sides_map[in_edge][0];
        sides.push_back(T::edge_sides_map[in_edge][1]);
      }

      info.emplace_back(candidate, std::move(sides), lower_bound, upper_bound);
      return true;
    }

    return false;
  }
  // If we have neither of the vertices, and the candidate's edge can't fully contain [vertex1,
  // vertex2], check for other vertices that are contained within the edge
  else
  {
    for (v1 = 0; v1 < n_vertices; ++v1)
      if (isWithinSegment(*vertex1, *vertex2, edge_length, candidate->point(v1)))
      {
        for (v2 = v1 + 1; v2 < n_vertices; ++v2)
          if (isWithinSegment(*vertex1, *vertex2, edge_length, candidate->point(v2)))
            break;
        break;
      }

    // No vertices are contained within the edge
    if (v1 == n_vertices)
      return false;

    // Only one vertex contained: add the sides associated with that vertex
    if (v2 >= n_vertices)
    {
      std::vector<unsigned short> sides;
      sides.reserve(2);

      for (MooseIndex(T::num_sides) s = 0; s < T::num_sides; ++s)
        if (T_candidate->is_node_on_side(v1, s))
          sides.push_back(s);

      if (sides.empty())
        mooseError("Failed to find sides that the vertex is on");

      // Bounds are the same because we only touch at a point
      const auto bound = (T_candidate->point(v1) - (Point)*vertex1).norm() / edge_length;
      info.emplace_back(candidate, std::move(sides), bound, bound);
      return true;
    }
    // Two vertices contained: find the sides that contain both vertices
    else
    {
      for (MooseIndex(T::num_edges) e = 0; e < T::num_edges; ++e)
        if (T_candidate->is_node_on_edge(v1, e) && T_candidate->is_node_on_edge(v2, e))
        {
          auto lower_bound = (T_candidate->point(v1) - (Point)*vertex1).norm() / edge_length;
          auto upper_bound = (T_candidate->point(v2) - (Point)*vertex1).norm() / edge_length;
          if (lower_bound > upper_bound)
          {
            const auto temp = lower_bound;
            lower_bound = upper_bound;
            upper_bound = temp;
          }

          std::vector<unsigned short> sides(2);
          sides[0] = T::edge_sides_map[e][0];
          sides[1] = T::edge_sides_map[e][1];
          info.emplace_back(candidate, std::move(sides), lower_bound, upper_bound);
          return true;
        }

      mooseError("Failed to find sides that the vertices are on");
    }
  }

  return false;
}

/**
 * Find the child of an elem that contains a point on a specified side of \p elem.
 * @param elem The parent element
 * @param point The point that is in elem
 * @param side The side that point is on in elem
 * @return The child that contains the point
 */
const Elem *
childContainingPointOnSide(const Elem * elem, const Point & point, const unsigned short side);

/**
 * Get the active neighbor on side of elem that contains point.
 *
 * @param elem The element
 * @param side The side of elem that point is on
 * @param point The point on the side you want the neighbor for
 * @return The active neighbor that contains the point
 */
const Elem * getActiveNeighbor(const Elem * elem, const unsigned short side, const Point & point);

/**
 * Checks for the intersection of a ray and a triangular face
 *
 * Adapted from:
 * webserver2.tecgraf.puc-rio.br/~mgattass/cg/trbRR/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
 *
 * @param start Start point of the ray
 * @param direction Direction of the ray
 * @param elem The elem that contains the triangular face (used to get the vertex points)
 * @param v0 Vertex index on elem that is the zeroth vertex of the triangular face
 * @param v1 Vertex index on elem that is the first vertex of the triangular face
 * @param v2 Vertex index on elem that is the second vertex of the triangular face
 * @param intersection_distance If intersected, storage for the intersection distance
 * @param intersected_extrema If a vertex/edge is intersected, will be filled with the intersection
 * @return Whether or not the ray intersected the triangular face
 */
bool intersectTriangle(const Point & start,
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
);

/**
 * Checks for the intersection of a ray and a quadrilateral, numbered as such:
 *
 * v01    v11
 * o--------o
 * |        |
 * |        |
 * o--------o
 * v00    v10
 *
 * Uses intersectTriangle() to check the possible intersection with the two triangles that make
 * up the quad
 *
 * @param start Start point of the ray
 * @param direction Direction of the ray
 * @param elem The elem that contains the quad face (used to get the vertex points)
 * @param v00 Vertex index on elem that represents v00 in the method description
 * @param v10 Vertex index on elem that represents v10 in the method description
 * @param v11 Vertex index on elem that represents v11 in the method description
 * @param v01 Vertex index on elem that represents v01 in the method description
 * @param intersection_distance If intersected, storage for the intersection distance
 * @param intersected_extrema If a vertex/edge is intersected, will be filled with the intersection
 * @return Whether or not the ray intersected the quadrilateral
 */
bool intersectQuad(const Point & start,
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
);

/**
 * SFINAE typed object for checking the intersection of a line segment with the face of a Face
 * typed element, i.e., the intersection with the edge of a 2D element.
 * @param elem The element to check
 * @param start_point Start point of the segment
 * @param direction Direction of the segment
 * @param side The side of elem to check
 * @param max_length A length that is longer than the length of the line segment
 * @param intersection_point If intersected, storage for the intersection point
 * @param intersection_distance If intersected, storage for the intersection distance
 * @param intersected_extrema If a vertex/edge is intersected, will be filled with the intersection
 * @return Whether or not the line segment intersected with the face
 */
template <typename T>
typename std::enable_if<std::is_base_of<Face, T>::value, bool>::type
sideIntersectedByLine(const Elem * elem,
                      const Point & start_point,
                      const Point & direction,
                      const unsigned short side,
                      const Real max_length,
                      Point & intersection_point,
                      Real & intersection_distance,
                      ElemExtrema & intersected_extrema,
                      const Real /* hmax */
#ifdef DEBUG_RAY_INTERSECTIONS
                      ,
                      const bool debug
#endif
)
{
  mooseAssert(intersection_point == RayTracingCommon::invalid_point, "Point should be invalid");
  mooseAssert(intersected_extrema.isInvalid(), "Should be invalid");

  SegmentVertices segment_vertex = SEGMENT_VERTEX_NONE;

  const bool intersected = lineLineIntersect2D(start_point,
                                               direction,
                                               max_length,
                                               elem->point(T::side_nodes_map[side][0]),
                                               elem->point(T::side_nodes_map[side][1]),
                                               intersection_point,
                                               intersection_distance,
                                               segment_vertex
#ifdef DEBUG_RAY_INTERSECTIONS
                                               ,
                                               debug
#endif
  );

  if (segment_vertex != SEGMENT_VERTEX_NONE)
  {
    intersected_extrema.setVertex(T::side_nodes_map[side][segment_vertex]);
    mooseAssert(intersected_extrema.vertexPoint(elem).absolute_fuzzy_equals(intersection_point,
                                                                            TRACE_TOLERANCE),
                "Doesn't intersect vertex");
  }

  return intersected;
}

/**
 * SFINAE typed object for checking the intersection of a line segment with the face of a Hex
 * typed element
 * @param elem The element to check
 * @param start_point Start point of the segment
 * @param direction Direction of the segment
 * @param side The side of elem to check
 * @param intersection_point If intersected, storage for the intersection point
 * @param intersection_distance If intersected, storage for the intersection distance
 * @param intersected_extrema If a vertex/edge is intersected, will be filled with the intersection
 * @return Whether or not the line segment intersected with the face
 */
template <typename T>
typename std::enable_if<std::is_base_of<Hex, T>::value, bool>::type
sideIntersectedByLine(const Elem * elem,
                      const Point & start_point,
                      const Point & direction,
                      const unsigned short side,
                      const Real,
                      Point & intersection_point,
                      Real & intersection_distance,
                      ElemExtrema & intersected_extrema,
                      const Real hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                      ,
                      const bool debug
#endif
)
{
  mooseAssert(elem->first_order_equivalent_type(elem->type()) == HEX8, "Not a Hex");
  mooseAssert(intersection_point == RayTracingCommon::invalid_point, "Point should be invalid");
  mooseAssert(intersected_extrema.isInvalid(), "Should be invalid");

  const bool intersected = intersectQuad(start_point,
                                         direction,
                                         elem,
                                         T::side_nodes_map[side][3],
                                         T::side_nodes_map[side][2],
                                         T::side_nodes_map[side][1],
                                         T::side_nodes_map[side][0],
                                         intersection_distance,
                                         intersected_extrema,
                                         hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                                         ,
                                         debug
#endif
  );

  if (intersected)
    intersection_point = start_point + intersection_distance * direction;

  return intersected;
}

/**
 * SFINAE typed object for checking the intersection of a line segment with the face of a Tet
 * typed element
 * @param elem The element to check
 * @param start_point Start point of the segment
 * @param direction Direction of the segment
 * @param side The side of elem to check
 * @param max_length A length that is longer than the length of the line segment
 * @param intersection_point If intersected, storage for the intersection point
 * @param intersection_distance If intersected, storage for the intersection distance
 * @param intersected_vertex If a vertex is intersected, storage for said intersection
 * @param intersected_extrema If a vertex/edge is intersected, will be filled with the intersection
 * @return Whether or not the line segment intersected with the face
 */
template <typename T>
typename std::enable_if<std::is_base_of<Tet, T>::value, bool>::type
sideIntersectedByLine(const Elem * elem,
                      const Point & start_point,
                      const Point & direction,
                      const unsigned short side,
                      const Real /* max_length */,
                      Point & intersection_point,
                      Real & intersection_distance,
                      ElemExtrema & intersected_extrema,
                      const Real hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                      ,
                      const bool debug
#endif
)
{
  mooseAssert(elem->first_order_equivalent_type(elem->type()) == TET4, "Not a Tet");
  mooseAssert(intersection_point == RayTracingCommon::invalid_point, "Point should be invalid");
  mooseAssert(intersected_extrema.isInvalid(), "Should be invalid");

  const bool intersected = intersectTriangle(start_point,
                                             direction,
                                             elem,
                                             T::side_nodes_map[side][2],
                                             T::side_nodes_map[side][1],
                                             T::side_nodes_map[side][0],
                                             intersection_distance,
                                             intersected_extrema,
                                             hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                                             ,
                                             debug
#endif
  );

  if (intersected)
    intersection_point = start_point + intersection_distance * direction;

  return intersected;
}

/**
 * SFINAE typed object for checking the intersection of a line segment with the face of a Pyramid
 * typed element
 * @param elem The element to check
 * @param start_point Start point of the segment
 * @param direction Direction of the segment
 * @param side The side of elem to check
 * @param max_length A length that is longer than the length of the line segment
 * @param intersection_point If intersected, storage for the intersection point
 * @param intersection_distance If intersected, storage for the intersection distance
 * @param intersected_vertex If a vertex is intersected, storage for said intersection
 * @param intersected_extrema If a vertex/edge is intersected, will be filled with the intersection
 * @return Whether or not the line segment intersected with the face
 */
template <typename T>
typename std::enable_if<std::is_base_of<Pyramid, T>::value, bool>::type
sideIntersectedByLine(const Elem * elem,
                      const Point & start_point,
                      const Point & direction,
                      const unsigned short side,
                      const Real /* max_length */,
                      Point & intersection_point,
                      Real & intersection_distance,
                      ElemExtrema & intersected_extrema,
                      const Real hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                      ,
                      const bool debug
#endif
)
{
  mooseAssert(elem->first_order_equivalent_type(elem->type()) == PYRAMID5, "Not a Pyramid");
  mooseAssert(intersection_point == RayTracingCommon::invalid_point, "Point should be invalid");
  mooseAssert(intersected_extrema.isInvalid(), "Should be invalid");

  const bool intersected = side < 4 ? intersectTriangle(start_point,
                                                        direction,
                                                        elem,
                                                        T::side_nodes_map[side][2],
                                                        T::side_nodes_map[side][1],
                                                        T::side_nodes_map[side][0],
                                                        intersection_distance,
                                                        intersected_extrema,
                                                        hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                                                        ,
                                                        debug
#endif
                                                        )
                                    : intersectQuad(start_point,
                                                    direction,
                                                    elem,
                                                    T::side_nodes_map[side][3],
                                                    T::side_nodes_map[side][2],
                                                    T::side_nodes_map[side][1],
                                                    T::side_nodes_map[side][0],
                                                    intersection_distance,
                                                    intersected_extrema,
                                                    hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                                                    ,
                                                    debug
#endif
                                      );

  if (intersected)
    intersection_point = start_point + intersection_distance * direction;

  return intersected;
}

/**
 * SFINAE typed object for checking the intersection of a line segment with the face of a Prism
 * typed element
 * @param elem The element to check
 * @param start_point Start point of the segment
 * @param direction Direction of the segment
 * @param side The side of elem to check
 * @param max_length A length that is longer than the length of the line segment
 * @param intersection_point If intersected, storage for the intersection point
 * @param intersection_distance If intersected, storage for the intersection distance
 * @param intersected_vertex If a vertex is intersected, storage for said intersection
 * @param intersected_extrema If a vertex/edge is intersected, will be filled with the intersection
 * @return Whether or not the line segment intersected with the face
 */
template <typename T>
typename std::enable_if<std::is_base_of<Prism, T>::value, bool>::type
sideIntersectedByLine(const Elem * elem,
                      const Point & start_point,
                      const Point & direction,
                      const unsigned short side,
                      const Real /* max_length */,
                      Point & intersection_point,
                      Real & intersection_distance,
                      ElemExtrema & intersected_extrema,
                      const Real hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                      ,
                      const bool debug
#endif
)
{
  mooseAssert(elem->first_order_equivalent_type(elem->type()) == PRISM6, "Not a Prism");
  mooseAssert(intersection_point == RayTracingCommon::invalid_point, "Point should be invalid");
  mooseAssert(intersected_extrema.isInvalid(), "Should be invalid");

  const bool intersected = (side == 0 || side == 4) ? intersectTriangle(start_point,
                                                                        direction,
                                                                        elem,
                                                                        T::side_nodes_map[side][2],
                                                                        T::side_nodes_map[side][1],
                                                                        T::side_nodes_map[side][0],
                                                                        intersection_distance,
                                                                        intersected_extrema,
                                                                        hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                                                                        ,
                                                                        debug
#endif
                                                                        )
                                                    : intersectQuad(start_point,
                                                                    direction,
                                                                    elem,
                                                                    T::side_nodes_map[side][3],
                                                                    T::side_nodes_map[side][2],
                                                                    T::side_nodes_map[side][1],
                                                                    T::side_nodes_map[side][0],
                                                                    intersection_distance,
                                                                    intersected_extrema,
                                                                    hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                                                                    ,
                                                                    debug
#endif
                                                      );

  if (intersected)
    intersection_point = start_point + intersection_distance * direction;

  return intersected;
}

/**
 * @return Whether not the element is traceable
 */
bool isTraceableElem(const Elem * elem);
/**
 * @return Whether or not the element is traceable with adapativity
 */
bool isAdaptivityTraceableElem(const Elem * elem);

/**
 * Determines if a point is at a vertex of an element.
 * @param elem The element
 * @param point The point
 * @return The local vertex ID the point is at, if any (invalid_point otherwise)
 */
unsigned short atVertex(const Elem * elem, const Point & point);

/**
 * Determines if a point is at a vertex on the side of en element.
 * @param elem The element
 * @param point The point
 * @param side The side
 * @return The local vertex ID the point is at, if any (invalid_point otherwise)
 */
unsigned short atVertexOnSide(const Elem * elem, const Point & point, const unsigned short side);

/**
 * Returns the number of nodes on a side for an Elem that is not a Pyramid or Prism.
 */
template <typename T>
inline typename std::enable_if<!std::is_base_of<Pyramid, T>::value &&
                                   !std::is_base_of<Prism, T>::value,
                               unsigned short>::type
nodesPerSide(const unsigned short)
{
  return T::nodes_per_side;
}

/**
 * Returns the number of nodes on a side on a Pyramid elem.
 */
template <typename T>
inline typename std::enable_if<std::is_base_of<Pyramid, T>::value, unsigned short>::type
nodesPerSide(const unsigned short side)
{
  return T::nodes_per_side - (side != 4);
}

/**
 * Returns the number of nodes on a side on a Prism elem.
 */
template <typename T>
inline typename std::enable_if<std::is_base_of<Prism, T>::value, unsigned short>::type
nodesPerSide(const unsigned short side)
{
  return T::nodes_per_side - (side == 0 || side == 4);
}

/**
 * Determines if a point is at a vertex on the side of an element.
 *
 * SFINAE here makes this method available for only 2D/3D elem types (not edges)
 *
 * The template argument should be the first order type of the elem - this is used to
 * access the vertex mappings for said element without calling any virtuals
 *
 * @param elem The element
 * @param point The point
 * @param side The side
 * @return The local vertex ID the point is at, if any (invalid_point otherwise)
 */
template <typename T>
typename std::enable_if<!std::is_base_of<Edge, T>::value, unsigned short>::type
atVertexOnSideTempl(const Elem * elem, const Point & point, const unsigned short side);

/**
 * Determines if a point is at a vertex on the side of an element.
 *
 * SFINAE here makes this method available for only 1D elem types (edges)
 *
 * The template argument should be the first order type of the elem - this is used to
 * access the vertex mappings for said element without calling any virtuals
 *
 * @param elem The element
 * @param point The point
 * @param side The side
 * @return The local vertex ID the point is at, if any (invalid_point otherwise)
 */
template <typename T>
typename std::enable_if<std::is_base_of<Edge, T>::value, unsigned short>::type
atVertexOnSideTempl(const Elem * elem, const Point & point, const unsigned short side);

/**
 * Determines if a point is within edge on an element.
 *
 * The template argument should be a derived element type that corresponds to the elem->type(),
 * and is used to obtain edge/node mapping without using virtuals.
 *
 * @param elem The element
 * @param point The point
 * @param extrema To be filled with the local vertex IDs that contain the edge the point is
 * within, if any
 * @param tolerance The tolerance to use
 * @return If the point is within an edge of the element
 */
template <typename T>
bool withinEdgeTempl(const Elem * elem,
                     const Point & point,
                     ElemExtrema & extrema,
                     const Real tolerance = TRACE_TOLERANCE);

/**
 * Determines if a point is within an edge on an element.
 * @param elem The element
 * @param point The point
 * @param extrema To be filled with the local vertex IDs that contain the edge the point is
 * within, if any
 * @param tolerance The tolerance to use
 * @return If the point is within an edge of the element
 */
bool withinEdge(const Elem * elem,
                const Point & point,
                ElemExtrema & extrema,
                const Real tolerance = TRACE_TOLERANCE);

/**
 * Determines if a point is within an edge on the side of an element.
 * @param elem The element
 * @param point The point
 * @param side The side
 * @param extrema To be filled with the local vertex IDs that contain the edge the point is
 * within, if any
 * @return If the point is within an edge on the side of the element
 */
bool withinEdgeOnSide(const Elem * const elem,
                      const Point & point,
                      const unsigned short side,
                      ElemExtrema & extrema);

/**
 * Determines if a point is within an Elem's extrema (at vertex/within edge) on a side
 * @param elem The element
 * @param point The point
 * @param side The side
 * @param dim The element dimension
 * @param extrema To be filled with the extrema if any are found
 * @return If the point is at a vertex/within an edge on the side
 */
bool withinExtremaOnSide(const Elem * const elem,
                         const Point & point,
                         const unsigned short side,
                         const unsigned int dim,
                         ElemExtrema & extrema);

/**
 * Determines if a point is within an edge on the side of an element.
 *
 * SFINAE here makes this method available for only 3D elem types (Cell)
 *
 * The template argument should be the first order type of the elem - this is used to
 * access the vertex mappings for said element without calling any virtuals
 *
 * @param elem The element
 * @param point The point
 * @param side The side
 * @param extrema To be filled with the local vertex IDs that contain the edge the point is
 * within, if any
 * @return If the point is within an edge on the side of the element
 */
template <typename T>
typename std::enable_if<std::is_base_of<Cell, T>::value, bool>::type withinEdgeOnSideTempl(
    const Elem * const elem, const Point & point, const unsigned short side, ElemExtrema & extrema);

/**
 * Determines if a point is within an edge on the side of an element.
 *
 * SFINAE here makes this method available for only 1D/2D elem types (not Cell), which do not have
 * edges, therefore this function errors.
 */
template <typename T>
typename std::enable_if<!std::is_base_of<Cell, T>::value, bool>::type
withinEdgeOnSideTempl(const Elem * const, const Point &, const unsigned short, ElemExtrema &)
{
  mooseError("Should not call withinEdgeOnSideTempl() with a non-Cell derived Elem");
}

/**
 * Whether or not \p point is on the boundary (min/max) of \p bbox.
 *
 * Checks \p dim dimensions.
 */
bool onBoundingBoxBoundary(const BoundingBox & bbox,
                           const Point & point,
                           const unsigned int dim,
                           const Real tolerance);

}
