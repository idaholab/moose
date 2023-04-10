//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TraceRay.h"

// Local includes
#include "Ray.h"
#include "RayBoundaryConditionBase.h"
#include "RayKernelBase.h"
#include "RayTracingStudy.h"
#include "TraceRayTools.h"

// libMesh includes
#include "libmesh/cell_tet4.h"
#include "libmesh/cell_tet10.h"
#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex20.h"
#include "libmesh/cell_hex27.h"
#include "libmesh/cell_prism6.h"
#include "libmesh/cell_prism15.h"
#include "libmesh/cell_prism18.h"
#include "libmesh/cell_pyramid5.h"
#include "libmesh/cell_pyramid13.h"
#include "libmesh/cell_pyramid14.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"
#include "libmesh/edge_edge4.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad8.h"
#include "libmesh/face_quad9.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_tri6.h"
#include "libmesh/enum_to_string.h"
#include "libmesh/mesh.h"

using namespace TraceRayTools;

TraceRay::TraceRay(RayTracingStudy & study, const THREAD_ID tid)
  : _study(study),
    _mesh(study.getSubProblem().mesh()),
    _dim(_mesh.dimension()),
    _boundary_info(_mesh.getMesh().get_boundary_info()),
    _pid(_study.comm().rank()),
    _tid(tid),
    _backface_culling(false),
    _current_normals(nullptr),
    _results(FAILED_TRACES + 1)
{
}

void
TraceRay::preExecute()
{
  _current_subdomain_id = Elem::invalid_subdomain_id;
  _current_elem_type = INVALID_ELEM;

  // Zero out all results
  for (auto & val : _results)
    val = 0;

  _has_ray_kernels = _study.hasRayKernels(_tid);
  _is_rectangular_domain = _study.isRectangularDomain();
}

void
TraceRay::meshChanged()
{
  // Invalidate the vertex and edge neighbor caches
  _vertex_neighbors.clear();
  _edge_neighbors.clear();
}

TraceRay::ExitsElemResult
TraceRay::exitsElem(const Elem * elem,
                    const ElemType elem_type,
                    const unsigned short incoming_side,
                    Point & intersection_point,
                    unsigned short & intersected_side,
                    ElemExtrema & intersected_extrema,
                    Real & intersection_distance,
                    const Point * normals)
{
  debugRay("Called exitsElem()");

  traceAssert(elem_type == elem->type(), "elem_type incorrect");
  traceAssert(intersection_point == RayTracingCommon::invalid_point, "Point should be invalid");
  traceAssert(intersected_side == RayTracingCommon::invalid_side, "Side should be invalid");
  traceAssert(intersected_extrema.isInvalid(), "Extrema should be invalid");
  traceAssert(intersection_distance == RayTracingCommon::invalid_distance,
              "Distance should be invalid");
  if (_study.verifyRays() && incoming_side != RayTracingCommon::invalid_side &&
      !_study.sideIsNonPlanar(elem, incoming_side))
    traceAssert(_study.sideIsIncoming(elem, incoming_side, (*_current_ray)->direction(), _tid),
                "Incoming side is non-entrant");

  bool intersected;
  switch (elem_type)
  {
    case HEX8:
      intersected = exitsElem<Hex8, Hex8>(elem,
                                          incoming_side,
                                          intersection_point,
                                          intersected_side,
                                          intersected_extrema,
                                          intersection_distance,
                                          normals);
      break;
    case TET4:
      intersected = exitsElem<Tet4, Tet4>(elem,
                                          incoming_side,
                                          intersection_point,
                                          intersected_side,
                                          intersected_extrema,
                                          intersection_distance,
                                          normals);
      break;
    case PYRAMID5:
      intersected = exitsElem<Pyramid5, Pyramid5>(elem,
                                                  incoming_side,
                                                  intersection_point,
                                                  intersected_side,
                                                  intersected_extrema,
                                                  intersection_distance,
                                                  normals);
      break;
    case PRISM6:
      intersected = exitsElem<Prism6, Prism6>(elem,
                                              incoming_side,
                                              intersection_point,
                                              intersected_side,
                                              intersected_extrema,
                                              intersection_distance,
                                              normals);
      break;
    case QUAD4:
      intersected = exitsElem<Quad4, Quad4>(elem,
                                            incoming_side,
                                            intersection_point,
                                            intersected_side,
                                            intersected_extrema,
                                            intersection_distance,
                                            normals);
      break;
    case TRI3:
      intersected = exitsElem<Tri3, Tri3>(elem,
                                          incoming_side,
                                          intersection_point,
                                          intersected_side,
                                          intersected_extrema,
                                          intersection_distance,
                                          normals);
      break;
    case HEX20:
      intersected = exitsElem<Hex20, Hex8>(elem,
                                           incoming_side,
                                           intersection_point,
                                           intersected_side,
                                           intersected_extrema,
                                           intersection_distance,
                                           normals);
      break;
    case HEX27:
      intersected = exitsElem<Hex27, Hex8>(elem,
                                           incoming_side,
                                           intersection_point,
                                           intersected_side,
                                           intersected_extrema,
                                           intersection_distance,
                                           normals);
      break;
    case QUAD8:
      intersected = exitsElem<Quad8, Quad4>(elem,
                                            incoming_side,
                                            intersection_point,
                                            intersected_side,
                                            intersected_extrema,
                                            intersection_distance,
                                            normals);
      break;
    case QUAD9:
      intersected = exitsElem<Quad9, Quad4>(elem,
                                            incoming_side,
                                            intersection_point,
                                            intersected_side,
                                            intersected_extrema,
                                            intersection_distance,
                                            normals);
      break;
    case TRI6:
      intersected = exitsElem<Tri6, Tri3>(elem,
                                          incoming_side,
                                          intersection_point,
                                          intersected_side,
                                          intersected_extrema,
                                          intersection_distance,
                                          normals);
      break;
    case TET10:
      intersected = exitsElem<Tet10, Tet4>(elem,
                                           incoming_side,
                                           intersection_point,
                                           intersected_side,
                                           intersected_extrema,
                                           intersection_distance,
                                           normals);
      break;
    case PYRAMID13:
      intersected = exitsElem<Pyramid13, Pyramid5>(elem,
                                                   incoming_side,
                                                   intersection_point,
                                                   intersected_side,
                                                   intersected_extrema,
                                                   intersection_distance,
                                                   normals);
      break;
    case PYRAMID14:
      intersected = exitsElem<Pyramid14, Pyramid5>(elem,
                                                   incoming_side,
                                                   intersection_point,
                                                   intersected_side,
                                                   intersected_extrema,
                                                   intersection_distance,
                                                   normals);
      break;
    case PRISM15:
      intersected = exitsElem<Prism15, Prism6>(elem,
                                               incoming_side,
                                               intersection_point,
                                               intersected_side,
                                               intersected_extrema,
                                               intersection_distance,
                                               normals);
      break;
    case PRISM18:
      intersected = exitsElem<Prism18, Prism6>(elem,
                                               incoming_side,
                                               intersection_point,
                                               intersected_side,
                                               intersected_extrema,
                                               intersection_distance,
                                               normals);
      break;
    case EDGE2:
      intersected = exitsElem<Edge2, Edge2>(elem,
                                            incoming_side,
                                            intersection_point,
                                            intersected_side,
                                            intersected_extrema,
                                            intersection_distance,
                                            normals);
      break;
    case EDGE3:
      intersected = exitsElem<Edge3, Edge2>(elem,
                                            incoming_side,
                                            intersection_point,
                                            intersected_side,
                                            intersected_extrema,
                                            intersection_distance,
                                            normals);
      break;
    case EDGE4:
      intersected = exitsElem<Edge4, Edge2>(elem,
                                            incoming_side,
                                            intersection_point,
                                            intersected_side,
                                            intersected_extrema,
                                            intersection_distance,
                                            normals);
      break;
    default:
      mooseError(
          "Element type ", Utility::enum_to_string(elem->type()), " not supported by TraceRay");
  }

  if (intersected)
  {
    if (intersected_extrema.atExtrema())
      return intersected_extrema.atVertex() ? HIT_VERTEX : HIT_EDGE;
    return HIT_FACE;
  }

  return NO_EXIT;
}

template <typename T, typename FirstOrderT>
typename std::enable_if<!std::is_base_of<Edge, T>::value, bool>::type
TraceRay::exitsElem(const Elem * elem,
                    const unsigned short incoming_side,
                    Point & intersection_point,
                    unsigned short & intersected_side,
                    ElemExtrema & intersected_extrema,
                    Real & intersection_distance,
                    const Point * normals)
{
  ++_results[INTERSECTION_CALLS];

  debugRay("Called exitsElem() in 2D or 3D");

  const auto hmax = subdomainHmax(elem);

  // Current point and distance (maybe not the best!)
  Point current_intersection_point;
  Real current_intersection_distance;
  ElemExtrema current_intersected_extrema;
  // Scale the minimum intersection distance required by hmax
  Real best_intersection_distance = TRACE_TOLERANCE * hmax;
  // Whether or not we are going to try backface culling the first time around
  // This depends on if the user set it to use culling
  bool use_backface_culling = _backface_culling;
  // If all sides have failed to find an intersection, whether or not we
  // are going to see if the nonplanar skip side (only if it IS nonplanar)
  // is also an exiting point
  bool try_nonplanar_incoming_side = false;
  // Easy access into the current direction
  const auto & direction = (*_current_ray)->direction();
  // The current side that we're checking
  unsigned short s = 0;

  // Loop over the side loop. We need this in the cases that:
  // - Backface culling is enabled and we were not able to find an intersection
  //   so we will go through all culled sides and see if we can find one
  // - The incoming_side is non-planar and the ray also exits said side,
  //   which will be checked after all other sides fail
  while (true)
  {
    debugRay("  use_backface_culling = ", use_backface_culling);
    debugRay("  try_nonplanar_incoming_side = ", try_nonplanar_incoming_side);
    // Loop over all of the sides
    while (true)
    {
      debugRay("  Side ", s, " with centroid ", _elem_side_builder(*elem, s).vertex_average());

      // All of the checks that follow are done while we're still searching through
      // all of the sides. try_nonplanar_incoming_side is our last possible check
      // and does not involve looping through all of the sides, so skip these
      // checks if we're at that point.
      if (!try_nonplanar_incoming_side)
      {
        // Don't search backwards. If we have a non-planar incoming side, we will
        // check it if all other sides have failed
        if (s == incoming_side)
        {
          debugRay("    Skipping due to incoming side");
          if (++s == T::num_sides)
            break;
          else
            continue;
        }

        // Using backface culling on this run through the sides
        // If the direction is non-entrant, skip this side
        if (use_backface_culling)
        {
          // Side is non-entrant per the culling, so skip
          if (normals[s] * direction < -LOOSE_TRACE_TOLERANCE)
          {
            debugRay("    Skipping due to backface culling dot = ", normals[s] * direction);

            if (++s == T::num_sides)
              break;
            else
              continue;

            ++_results[BACKFACE_CULLING_SUCCESSES];
          }
        }
        // We're not using backface culling but it is enabled, which means
        // we're on our second run through the sides because we could not
        // find any intersections while culling the sides. Try again and
        // check the sides that we previously skipped for intersections
        else if (_backface_culling)
        {
          // Side was non-entrant per the culling, so try again
          if (normals[s] * direction >= -LOOSE_TRACE_TOLERANCE)
          {
            debugRay("    Skipping because we already checked this side with culling enabled");
            if (++s == T::num_sides)
              break;
            else
              continue;
          }
          else
            debugRay("    Side that was skipped due to culling");

          ++_results[BACKFACE_CULLING_FAILURES];
        }
      }

      // Look for an intersection!
      current_intersection_point = RayTracingCommon::invalid_point;
      current_intersected_extrema.invalidate();
      const bool intersected = sideIntersectedByLine<FirstOrderT>(elem,
                                                                  _incoming_point,
                                                                  direction,
                                                                  s,
                                                                  _study.domainMaxLength(),
                                                                  current_intersection_point,
                                                                  current_intersection_distance,
                                                                  current_intersected_extrema,
                                                                  hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                                                                  ,
                                                                  DEBUG_RAY_IF
#endif
      );

      // Do they intersect and is it further down the path than any other intersection?
      // If so, keep track of the intersection with the furthest distance
      if (intersected)
      {
        debugRay("    Intersected at point ",
                 current_intersection_point,
                 " with distance ",
                 current_intersection_distance);
        debugRay("    Best intersection distance = ", best_intersection_distance);

#ifndef NDEBUG
        // Only validate intersections if the side is planar
        if (_study.verifyTraceIntersections() && !_study.sideIsNonPlanar(elem, s) &&
            !_elem_side_builder(*elem, s).contains_point(current_intersection_point))
          failTrace("Intersected side does not contain intersection point",
                    _study.tolerateFailure(),
                    __LINE__);
#endif

        // The intersection we just computed is further than any other intersection
        // that was found so far - mark it as the best
        if (current_intersection_distance > best_intersection_distance)
        {
          debugRay("    Best intersection so far");

          intersected_side = s;
          intersection_distance = current_intersection_distance;
          intersection_point = current_intersection_point;
          intersected_extrema = current_intersected_extrema;
          best_intersection_distance = current_intersection_distance;
        }
      }

      if (++s == T::num_sides || try_nonplanar_incoming_side)
        break;
      else
        continue;
    } // while(true)

    // Found an intersection
    if (intersected_side != RayTracingCommon::invalid_side)
    {
      debugRay("  Exiting with intersection");
      debugRay("    intersected_side = ", intersected_side);
      debugRay("    intersection_distance = ", intersection_distance);
      debugRay("    intersection_point = ", intersection_point);
      debugRay("    intersected_extrema = ", intersected_extrema);

      return true;
    }

    // This was our last possible check - no dice!
    if (try_nonplanar_incoming_side)
      return false;

    // We didn't find an intersection but we used backface culling. Try again without
    // it, checking only the sides that we skipped due to culling
    if (use_backface_culling)
    {
      debugRay("  Didn't find an intersection, retrying without backface culling");
      use_backface_culling = false;
      s = 0;
      continue;
    }

    // Have tried all sides (potentially with and without culling). If the incoming
    // side is valid is non planar, see if we also exit out of it
    if (_incoming_side != RayTracingCommon::invalid_side &&
        _study.sideIsNonPlanar(elem, incoming_side))
    {
      debugRay("  Didn't find an intersection, trying non-planar incoming_side");
      try_nonplanar_incoming_side = true;
      s = incoming_side;
      continue;
    }

    // No intersection found!
    return false;
  } // while(true)
}

template <typename T, typename FirstOrderT>
typename std::enable_if<std::is_base_of<Edge, T>::value, bool>::type
TraceRay::exitsElem(const Elem * elem,
                    const unsigned short incoming_side,
                    Point & intersection_point,
                    unsigned short & intersected_side,
                    ElemExtrema & intersected_extrema,
                    Real & intersection_distance,
                    const Point *)
{
  ++_results[INTERSECTION_CALLS];

  debugRay("Called exitsElem() in 1D");

  // Scale the tolerance based on the element size
  const auto tol = subdomainHmax(elem) * TRACE_TOLERANCE;

  // Can quickly return if we have an incoming side
  // There's only one other choice in 1D!
  if (incoming_side != RayTracingCommon::invalid_side)
  {
    intersected_side = (incoming_side == 1 ? 0 : 1);
    intersected_extrema.setVertex(intersected_side);
    intersection_point = elem->point(intersected_side);
    intersection_distance = (_incoming_point - intersection_point).norm();

    debugRay("  Incoming side is set to ", incoming_side, " so setting to other side");
    debugRay("  Intersected side ", intersected_side, " at ", intersection_point);

    return true;
  }

  // End point that is for sure out of the element
  const Point extended_end_point =
      _incoming_point + _study.domainMaxLength() * (*_current_ray)->direction();

  // We're looking for the side whose point lays between the incoming point and the
  // extended end point
  for (MooseIndex(elem->n_sides()) side = 0; side < elem->n_sides(); ++side)
  {
    const Point side_point = elem->point(side);
    debugRay("  Checking side ", side, " at ", side_point);

    const Real incoming_to_side = (side_point - _incoming_point).norm();
    if (incoming_to_side < tol)
    {
      debugRay("    Continuing because at side");
      continue;
    }

    const Real incoming_to_end = (extended_end_point - _incoming_point).norm();
    const Real side_to_end = (extended_end_point - side_point).norm();
    const Real sum = incoming_to_side + side_to_end - incoming_to_end;
    debugRay("    Sum = ", sum);

    if (std::abs(sum) < tol)
    {
      intersected_side = side;
      intersected_extrema.setVertex(side);
      intersection_point = side_point;
      intersection_distance = incoming_to_side;
      debugRay("    Intersected at ", intersection_point);
      return true;
    }
  }

  return false;
}

TraceRay::ExitsElemResult
TraceRay::moveThroughNeighbors(const std::vector<NeighborInfo> & neighbors,
                               const Elem * last_elem,
                               const Elem *& best_elem,
                               unsigned short & best_elem_incoming_side)
{
  ++_results[MOVED_THROUGH_NEIGHBORS];

  debugRay("Called moveThroughNeighbors() with ", neighbors.size(), " neighbors, and:");
  debugRay("  last_elem->id() = ", last_elem ? last_elem->id() : DofObject::invalid_id);
  debugRay("  _incoming_point = ", _incoming_point);

  traceAssert(!best_elem, "Best elem should be null");
  traceAssert(best_elem_incoming_side == RayTracingCommon::invalid_side,
              "Best elem side should be invalid");
  traceAssert(_intersection_point == RayTracingCommon::invalid_point, "Point should be invalid");
  traceAssert(_intersected_side == RayTracingCommon::invalid_side, "Side should be invalid");
  traceAssert(_intersected_extrema.isInvalid(), "Extrema should be invalid");
  traceAssert(_intersection_distance == RayTracingCommon::invalid_distance,
              "Distance should be invalid");

  // Marker for the longest ray segment distance we've found in a neighbor. Start with a quite
  // small number because at this point we're desperate and will take anything!
  Real longest_distance = 1.0e-12;
  // Temporaries for the intersection checks
  unsigned short current_incoming_side;
  Point current_intersection_point;
  unsigned short current_intersected_side;
  ElemExtrema current_intersected_extrema;
  Real current_intersection_distance;
  TraceRay::ExitsElemResult best_exit_result = NO_EXIT;

  // The NeighborInfo for the last_elem
  // (to store so we can try it later if we fail for everyone else)
  const NeighborInfo * last_elem_info = nullptr;

  for (const NeighborInfo & neighbor_info : neighbors)
  {
    // If we're at the element we want to do last, skip it and store the info so
    // we can get to it later in case we fail at finding anything
    if (neighbor_info._elem == last_elem)
    {
      debugRay("Skipping last elem ", last_elem->id());
      last_elem_info = &neighbor_info;
      continue;
    }

    const auto exit_result = moveThroughNeighbor(neighbor_info,
                                                 current_incoming_side,
                                                 current_intersection_point,
                                                 current_intersected_side,
                                                 current_intersected_extrema,
                                                 current_intersection_distance);

    // Found one! See if it's the best way
    if (exit_result != NO_EXIT)
    {
      debugRay("Ray can exit through neighbor ", neighbor_info._elem->id());

      if (current_intersection_distance > longest_distance)
      {
        best_elem = neighbor_info._elem;
        best_elem_incoming_side = current_incoming_side;
        _intersection_point = current_intersection_point;
        _intersected_side = current_intersected_side;
        _intersected_extrema = current_intersected_extrema;
        _intersection_distance = current_intersection_distance;
        longest_distance = current_intersection_distance;
        best_exit_result = exit_result;
      }
    }
  }

  // Didn't find someone to exit, so try the last_elem (if any)
  if (!best_elem && last_elem_info)
  {
    const auto exit_result = moveThroughNeighbor(*last_elem_info,
                                                 current_incoming_side,
                                                 current_intersection_point,
                                                 current_intersected_side,
                                                 current_intersected_extrema,
                                                 current_intersection_distance);

    if (exit_result != NO_EXIT && current_intersection_distance > longest_distance)
    {
      debugRay("Ray can exit through last_elem ", last_elem->id());

      best_elem = last_elem;
      best_elem_incoming_side = current_incoming_side;
      _intersection_point = current_intersection_point;
      _intersected_side = current_intersected_side;
      _intersected_extrema = current_intersected_extrema;
      _intersection_distance = current_intersection_distance;
      best_exit_result = exit_result;
    }
  }

  debugRay("moveThroughNeighbors() best result:");
  debugRay("  best_elem = ", best_elem ? best_elem->id() : DofObject::invalid_id);
  debugRay("  best_elem_incoming_side = ", best_elem_incoming_side);
  debugRay("  _intersection_point = ", _intersection_point);
  debugRay("  _intersected_side = ", _intersected_side);
  debugRay("  _intersected_extrema = ", _intersected_extrema);
  debugRay("  _intersection_distance = ", _intersection_distance);
  if (best_elem)
  {
    debugRay("moveThroughNeighbors() next neighbor elem info:");
    debugRay(best_elem->get_info());
  }

  return best_exit_result;
}

TraceRay::ExitsElemResult
TraceRay::moveThroughNeighbor(const NeighborInfo & neighbor_info,
                              unsigned short & incoming_side,
                              Point & intersection_point,
                              unsigned short & intersected_side,
                              ElemExtrema & intersected_extrema,
                              Real & intersection_distance)
{
  if (!neighbor_info._valid)
    return NO_EXIT;

  const Elem * neighbor = neighbor_info._elem;
  debugRay("Checking neighbor ", neighbor->id(), " with centroid ", neighbor->vertex_average());

  // Find an entrant side (if any)
  incoming_side = RayTracingCommon::invalid_side;
  for (MooseIndex(neighbor_info._sides.size()) i = 0; i < neighbor_info._sides.size(); ++i)
    if (neighbor_info._side_normals[i] * (*_current_ray)->direction() < LOOSE_TRACE_TOLERANCE)
    {
      incoming_side = neighbor_info._sides[i];
      break;
    }

  // No entrant sides on this neighbor were found
  if (incoming_side == RayTracingCommon::invalid_side)
    return NO_EXIT;

  intersection_point = RayTracingCommon::invalid_point;
  intersected_side = RayTracingCommon::invalid_side;
  intersected_extrema.invalidate();
  intersection_distance = RayTracingCommon::invalid_distance;

  // See if there is a way through the neighbor element
  debugRay("Called exitsElem() from moveThroughNeighbor()");
  const auto exit_result =
      exitsElem(neighbor,
                neighbor->type(),
                incoming_side,
                intersection_point,
                intersected_side,
                intersected_extrema,
                intersection_distance,
                _backface_culling ? _study.getElemNormals(neighbor, _tid) : nullptr);
  debugRay("Done with exitsElem() from moveThroughNeighbor()");

  return exit_result;
}

void
TraceRay::applyOnExternalBoundary(const std::shared_ptr<Ray> & ray)
{
  debugRay("Called applyOnExternalBoundary() with");
  debugRay("  _current_elem->id() = ", _current_elem->id());
  debugRay("  _intersection_point = ", _intersection_point);
  debugRay("  _intersected_side = ", _intersected_side);
  debugRay("  _intersected_extrema = ", _intersected_extrema);

  // Clear storage for the list of ConstBndElement that we need to apply RayBCs to
  _boundary_elems.clear();

  // If on the elem extrema (at a vertex or within an edge), check for external sidesets on
  // neighbors (which will include _current_elem)
  if (_dim != 1 && _intersected_extrema.atExtrema())
  {
    const auto & neighbors = getNeighbors(_current_elem, _intersected_extrema, _intersection_point);
    debugRay("  Found ", neighbors.size(), " vertex/edge neighbors (including self)");
    traceAssert(std::count_if(neighbors.begin(),
                              neighbors.end(),
                              [this](const NeighborInfo & ni)
                              { return ni._elem == _current_elem; }),
                "_current_elem not in neighbors");

    for (const auto & neighbor_info : neighbors)
    {
      if (!neighbor_info._valid)
        continue;

      const Elem * elem = neighbor_info._elem;
      const auto & sides = neighbor_info._sides;
      const auto & side_normals = neighbor_info._side_normals;

      for (MooseIndex(side_normals.size()) i = 0; i < side_normals.size(); ++i)
        if (!elem->neighbor_ptr(sides[i]) // is a boundary side that has our point
            && side_normals[i] * ray->direction() > TRACE_TOLERANCE) // and is entrant
        {
          // TODO: this could likely be optimized
          ElemExtrema extrema;
          withinExtremaOnSide(elem, _intersection_point, sides[i], _dim, extrema);

          _boundary_info.boundary_ids(elem, sides[i], _boundary_ids);
          possiblyAddToBoundaryElems(elem, sides[i], _boundary_ids, extrema);
        }
    }
  }
  // Not on the periphery (at vertex/edge), so we just need to add the external
  // sidesets from _current_elem on _intersected_side
  else
  {
    _boundary_info.boundary_ids(_current_elem, _intersected_side, _boundary_ids);
    possiblyAddToBoundaryElems(
        _current_elem, _intersected_side, _boundary_ids, _intersected_extrema);
  }

  debugRay("Calling external onBoundary() with ", _boundary_elems.size(), " boundaries");
  onBoundary(ray, /* external = */ true);
}

void
TraceRay::applyOnInternalBoundary(const std::shared_ptr<Ray> & ray)
{
  traceAssert(_last_elem, "Must be valid");

  debugRay("Called applyOnInternalBoundary() with");
  debugRay("  intersection_point = ", _intersection_point);
  debugRay("  _current_elem->id() = ", _current_elem->id());
  debugRay("  _incoming_side = ", _incoming_side);
  debugRay("  _last_elem->id() = ", _last_elem->id());
  debugRay("  _intersected_side = ", _intersected_side);
  debugRay("  _intersected_extrema = ", _intersected_extrema);
  traceAssert(_study.hasInternalSidesets(), "Do not have internal sidesets");
  traceAssert(_intersection_point.absolute_fuzzy_equals(_incoming_point, TRACE_TOLERANCE),
              "Intersection and incoming points should be the same");

  // Clear storage for the list of ConstBndElement that we need to apply RayBCs to
  _boundary_elems.clear();

  ElemExtrema temp_extrema;

  // If on the elem extrema (at a vertex or within an edge), we need to check for internal
  // sidesets on neighbors (which will include _last_elem and _current_elem)
  if (_dim != 1 && _intersected_extrema.atExtrema())
  {
    debugRay("Checking point neighbors for internal sidesets");

    // Get the neighbors
    const auto & neighbors = getNeighbors(_last_elem, _intersected_extrema, _intersection_point);
    debugRay("  Found ", neighbors.size(), " vertex/edge neighbors");
    traceAssert(std::count_if(neighbors.begin(),
                              neighbors.end(),
                              [this](const NeighborInfo & ni)
                              { return ni._elem == _current_elem || ni._elem == _last_elem; }) == 2,
                "_current_elem/_last_elem not in neighbors");

    for (const auto & neighbor_info : neighbors)
    {
      if (!neighbor_info._valid)
        continue;

      const Elem * elem = neighbor_info._elem;

      // Grab the internal sidesets for this elem
      const auto & sidesets = _study.getInternalSidesets(elem);
      // It has none to contribute, so we can continue
      if (sidesets.empty())
      {
        debugRay("    Elem ", elem->id(), " has no internal sidesets");
        continue;
      }

      // The sides on this elem that contain our point and their outward normals
      const auto & sides = neighbor_info._sides;
      const auto & side_normals = neighbor_info._side_normals;

      // See if any of the internal sidesets are relevant to this point
      for (std::size_t i = 0; i < sides.size(); ++i)
      {
        const auto side = sides[i];
        // Side has internal sidesets and is entrant
        if (sidesets[side].size() && std::abs(side_normals[i] * ray->direction()) > TRACE_TOLERANCE)
        {
          // TODO: this could likely be optimized
          temp_extrema.invalidate();
          withinExtremaOnSide(elem, _intersection_point, side, _dim, temp_extrema);

          possiblyAddToBoundaryElems(elem, side, sidesets[side], temp_extrema);
        }
      }
    }
  }
  // Not on the periphery (at vertex/edge), so we just need to add the sidesets from _current_elem
  // on _incoming_side and _last_elem on _intersected_side
  else
  {
    const auto & current_elem_sidesets = _study.getInternalSidesets(_current_elem);
    if (current_elem_sidesets.size() && current_elem_sidesets[_incoming_side].size())
    {
      // This is possible but we need extrema checks on _incoming_elem to pass to the
      // boundary conditions. For now, we just pass in _intersected_extrema which could
      // be very wrong with adaptivity but is correct without it. I struggled with getting
      // the actual extrema checks and we're not using this now so I bet this error will
      // come back to haunt me in the future :-)
      if (!_study.hasSameLevelActiveElems())
        mooseError("Internal sidesets are not currently supported with adaptivity in tracing");

      // Special case for 1D
      if (_dim == 1)
        temp_extrema.setVertex(atVertex(_current_elem, _intersection_point));

      possiblyAddToBoundaryElems(_current_elem,
                                 _incoming_side,
                                 current_elem_sidesets[_incoming_side],
                                 _dim != 1 ? _intersected_extrema : temp_extrema);
    }

    const auto & last_elem_sidesets = _study.getInternalSidesets(_last_elem);
    if (last_elem_sidesets.size() && last_elem_sidesets[_intersected_side].size())
      possiblyAddToBoundaryElems(_last_elem,
                                 _intersected_side,
                                 last_elem_sidesets[_intersected_side],
                                 _intersected_extrema);
  }

  if (!_boundary_elems.empty())
  {
    debugRay("  Calling internal onBoundary() with ", _boundary_elems.size(), " boundaries");
    onBoundary(ray, /* external = */ false);
  }
}

void
TraceRay::possiblyAddToBoundaryElems(const Elem * elem,
                                     const unsigned short side,
                                     const std::vector<BoundaryID> & bnd_ids,
                                     const ElemExtrema & extrema)
{
  if (!_study.sideIsNonPlanar(elem, side))
    traceAssert(extrema.isValid(elem, _intersection_point), "Extrema not correct");

  for (const auto bnd_id : bnd_ids)
  {
    bool found = false;
    for (const auto & bnd_elem : _boundary_elems)
      if (bnd_elem.bnd_id == bnd_id)
      {
        found = true;
        break;
      }

    if (!found)
    {
      debugRay("  Need to apply boundary on elem ",
               elem->id(),
               " and side ",
               side,
               " for bnd_id ",
               bnd_id);

      _boundary_elems.emplace_back(elem, side, bnd_id, extrema);
    }
  }
}

void
TraceRay::findExternalBoundarySide(unsigned short & boundary_side,
                                   ElemExtrema & boundary_extrema,
                                   const Elem *& boundary_elem)
{
  traceAssert(_current_elem->neighbor_ptr(_intersected_side), "Already on boundary");
  traceAssert(boundary_side == RayTracingCommon::invalid_side, "Side should be invalid");
  traceAssert(boundary_extrema.isInvalid(), "Extrema should be invalid");
  traceAssert(!boundary_elem, "Elem should be invalid");
  traceAssert(_current_elem->dim() != 1, "1D traces shouldn't make it here");
  traceAssert(_current_elem->n_sides() == _current_elem_n_sides, "_current_elem_n_sides incorrect");
  traceAssert(_intersected_extrema.atExtrema(), "Should be at extrema");
  debugRay("Called findExternalBoundarySide() on side ", _intersected_side);
  debugRay("  _intersected_side = ", _intersected_side);
  debugRay("  _intersected_extrema", _intersected_extrema);

  const auto & direction = (*_current_ray)->direction();
  const auto at_edge = _intersected_extrema.atEdge();

  // First, look for other sides on _current_elem that touch the intersected vertex/edge
  // that are on the boundary and are outgoing
  for (unsigned short s = 0; s < _current_elem_n_sides; ++s)
    if (!_current_elem->neighbor_ptr(s) && s != _intersected_side &&
        _current_elem->is_node_on_side(_intersected_extrema.first, s) &&
        (!at_edge || _current_elem->is_node_on_side(_intersected_extrema.second, s)) &&
        !_study.sideIsIncoming(_current_elem, s, direction, _tid))
    {
      debugRay("  Side ", s, " is a boundary side and the Ray exits");
      boundary_side = s;
      boundary_extrema = _intersected_extrema;
      boundary_elem = _current_elem;
      return;
    }

  // No luck in our element, so see if any neighbors at this vertex/edge are
  // on the boundary and are outgoing
  const auto & neighbors = getNeighbors(_current_elem, _intersected_extrema, _intersection_point);
  debugRay("Checking current element failed, now checking neighbors");

  debugRay("Found ", neighbors.size(), " candidate neighbors (including self)");
  for (const auto & neighbor_info : neighbors)
  {
    // This will be false when we have an edge neighbor that isn't a neighbor at
    // _intersection_point
    if (!neighbor_info._valid)
      continue;

    const Elem * neighbor = neighbor_info._elem;
    // We've already checked ourself
    if (neighbor == _current_elem)
      continue;

    debugRay("Checking neighbor ", neighbor->id());

    // Loop through the sides we touch and look for one that this Ray exits and is on the boundary
    for (MooseIndex(neighbor_info._sides.size()) i = 0; i < neighbor_info._sides.size(); ++i)
      if (neighbor_info._side_normals[i] * direction > TRACE_TOLERANCE &&
          !neighbor->neighbor_ptr(neighbor_info._sides[i]))
      {
        withinExtremaOnSide(
            neighbor, _intersection_point, neighbor_info._sides[i], _dim, boundary_extrema);
        traceAssert(boundary_extrema.atExtrema(), "Should be at extrema");
        boundary_side = neighbor_info._sides[i];
        boundary_elem = neighbor;
        return;
      }
  }
}

void
TraceRay::trace(const std::shared_ptr<Ray> & ray)
{
  mooseAssert(_study.currentlyPropagating(), "Should only use while propagating rays");

  _current_ray = &ray;
  _current_elem = ray->currentElem();
  _last_elem = nullptr;
  _incoming_point = ray->currentPoint();
  _incoming_side = ray->currentIncomingSide();
  _should_continue = true;

  _study.preTrace(_tid, ray);

  traceAssert(_current_elem, "Current element is not set");
  traceAssert(_current_elem->active(), "Current element is not active");
  traceAssert(!ray->invalidCurrentPoint(), "Current point is invalid");
  traceAssert(ray->shouldContinue(), "Ray should not continue");
  if (_study.verifyRays() && !ray->invalidCurrentIncomingSide() &&
      !_study.sideIsNonPlanar(_current_elem, _incoming_side) &&
      !_study.sideIsIncoming(_current_elem, _incoming_side, ray->direction(), _tid))
    failTrace("Ray incoming side is not incoming", /* warning = */ false, __LINE__);

#ifdef DEBUG_RAY_IF
  if (DEBUG_RAY_IF)
    libMesh::err << "\n\n";
#endif
  debugRay("At top of trace for Ray");
  debugRay("Top of trace loop Ray info\n", ray->getInfo());
  debugRay("Top of trace loop starting elem info\n", ray->currentElem()->get_info());

  // Invalidate this up front because it's copied immedtiately into _last_intersected_extrema
  _intersected_extrema.invalidate();

#ifdef DEBUG_RAY_MESH_IF
  _debug_mesh = nullptr;
  _debug_node_count = 0;
  if (DEBUG_RAY_MESH_IF)
  {
    _debug_mesh = new Mesh(_debug_comm, _dim);
    _debug_mesh->skip_partitioning(true);
  }
#endif

  // Caching trace along the way: init for this Ray
  if (_study.shouldCacheTrace(ray))
  {
    debugRay("Trying to init threaded cached trace");

    _current_cached_trace = &_study.initThreadedCachedTrace(ray, _tid);

    // Add starting data
    if (_study.dataOnCacheTraces())
      _current_cached_trace->lastPoint()._data = ray->data();
    if (_study.auxDataOnCacheTraces())
      _current_cached_trace->lastPoint()._aux_data = ray->auxData();
  }
  else
    _current_cached_trace = nullptr;

  // Need to call subdomain setup
  if (_current_elem->subdomain_id() != _current_subdomain_id || _study.rayDependentSubdomainSetup())
    onSubdomainChanged(ray, /* same_ray = */ false);
  // If we didn't change subdomains, we still need to call this on the RayKernels
  else
    for (RayKernelBase * rk : _study.currentRayKernels(_tid))
      rk->preTrace();

  // Ray tracing loop through each segment for a single Ray on this processor
  do
  {
#ifdef DEBUG_RAY_IF
    if (DEBUG_RAY_IF)
      libMesh::err << "\n\n";
#endif
    debugRay("At top of ray tracing loop");
    debugRay("  ray->id() = ", ray->id());
    debugRay("  _incoming_point = ", _incoming_point);
    debugRay("  _incoming_side = ", _incoming_side);
    debugRay("  _current_elem->id() = ", _current_elem->id());
    debugRay("  _current_elem->subdomain_id() = ", _current_elem->subdomain_id());
    debugRay("  _current_subdomain_id = ", _current_subdomain_id);
    debugRay("  _current_elem_type = ", Utility::enum_to_string(_current_elem_type));
    debugRay("Top of ray tracing loop Ray info\n", ray->getInfo());
    debugRay("Top of ray tracing loop current elem info\n", _current_elem->get_info());

    traceAssert(_current_ray == &ray, "Current ray mismatch");

    // Copy over in case we need to use it
    _last_intersected_extrema = _intersected_extrema;
    // Invalidate all intersections as we're tracing again
    _exits_elem = false;
    _intersection_point = RayTracingCommon::invalid_point;
    _intersected_side = RayTracingCommon::invalid_side;
    _intersected_extrema.invalidate();
    _intersection_distance = RayTracingCommon::invalid_distance;

    // If we haven't hit a vertex or an edge, do the normal exit algorithm first.
    // In the case of a Ray that previously moved through point neighbors due to
    // being at a vertex/edge, this will be true because we do not communicate the
    // vertex/edge intersection. The previous processor already set us up for
    // the best intersection because it already computed it and chose to
    // send it our way as such.
    if (!_last_intersected_extrema.atExtrema())
    {
      traceAssert(_current_elem->processor_id() == _pid, "Trace elem not on processor");
      debugRay("Didn't hit vertex or edge: doing normal exits elem check");

      if (_backface_culling)
        _current_normals = _study.getElemNormals(_current_elem, _tid);

      const auto exits_elem_result = exitsElem(_current_elem,
                                               _current_elem_type,
                                               _incoming_side,
                                               _intersection_point,
                                               _intersected_side,
                                               _intersected_extrema,
                                               _intersection_distance,
                                               _current_normals);

      if (exits_elem_result != NO_EXIT)
      {
        storeExitsElemResult(exits_elem_result);
        _exits_elem = true;
        ray->setCurrentPoint(_intersection_point);
      }
    }
    else
    {
      debugRay("Will not do normal exits elem check because at a vertex/edge");
    }

    // At this point, we either did a regular exit check and it failed, or we hit a vertex/edge
    // on the previous intersection and didn't bother to do a regular exit check on
    // _current_elem
    if (!_exits_elem)
    {
      debugRay("Moving through neighbors");

      // The element we want moveThroughNeighbors() to try last. That is - if all others fail,
      // this is the last resort. If we have a last intersection that tells us we're going
      // through a vertex or edge, we probably won't go back to that elem. If we don't, it means
      // that we failed the exitsElem() check on _current_elem and probably won't return to it
      // either.
      const Elem * move_through_neighbors_last = nullptr;

      // Get the neighbors
      const std::vector<NeighborInfo> * neighbors = nullptr;
      // If we have previous vertex/edge information, use it
      if (_last_intersected_extrema.atExtrema())
      {
        traceAssert(_last_elem, "Should be valid");
        move_through_neighbors_last = _last_elem;
        neighbors = &getNeighbors(_last_elem, _last_intersected_extrema, _incoming_point);
      }
      // Without previous vertex/edge information, let's try to find some. We could just do
      // a pure point neighbor check at the current point, but we'd rather be able to cache
      // this information so that someone else can use it - this requires the more unique
      // identifier of which vertex/edge we are at.
      else
      {
        debugRay("  Searching for vertex/edge hit with incoming side ",
                 _incoming_side,
                 " on elem ",
                 _current_elem->id(),
                 " at ",
                 _incoming_point);

        move_through_neighbors_last = _current_elem;

        // If we have side info: check for vertices on said side, otherwise, check everywhere
        const auto at_v = _incoming_side != RayTracingCommon::invalid_side
                              ? atVertexOnSide(_current_elem, _incoming_point, _incoming_side)
                              : atVertex(_current_elem, _incoming_point);

        if (at_v != RayTracingCommon::invalid_vertex)
          neighbors = &getVertexNeighbors(_current_elem, at_v);
        // If still nothing and in 3D, check if we're on an edge
        // TODO: Handle 2D with a similar check for if we're on a side
        else if (_dim == 3)
        {
          ElemExtrema extrema;
          if (_incoming_side != RayTracingCommon::invalid_side)
            withinEdgeOnSide(_current_elem, _incoming_point, _incoming_side, extrema);
          else
            withinEdge(_current_elem, _incoming_point, extrema);

          if (extrema.atEdge())
            neighbors = &getEdgeNeighbors(_current_elem, extrema, _incoming_point);
        }

        // If we still haven't found anything - let's try a last-ditch effort
        if (!neighbors || neighbors->empty())
          neighbors = &getPointNeighbors(_current_elem, _incoming_point);

        // Couldn't find anything
        if (neighbors->empty())
        {
          failTrace("Could not find neighbors to move through", _study.tolerateFailure(), __LINE__);
          return;
        }
      }

      // Move through a neighbor
      const Elem * best_neighbor = nullptr;
      auto best_neighbor_side = RayTracingCommon::invalid_side;
      const auto exits_elem_result = moveThroughNeighbors(
          *neighbors, move_through_neighbors_last, best_neighbor, best_neighbor_side);

      // If we didn't find anything... we're out of luck
      if (exits_elem_result == NO_EXIT)
      {
        failTrace("Could not find intersection after trying to move through point neighbors",
                  _study.tolerateFailure(),
                  __LINE__);
        return;
      }

      // At this point, we've successfully made it through an element and also started
      // through another element, so set that
      _exits_elem = true;
      _last_elem = _current_elem;
      _current_elem = best_neighbor;
      _incoming_side = best_neighbor_side;
      ray->setCurrentElem(best_neighbor);
      ray->setCurrentIncomingSide(best_neighbor_side);
      ray->setCurrentPoint(_intersection_point);

      // Don't own this element - return exits trace for this Ray on this proc
      if (best_neighbor->processor_id() != _pid)
      {
        // We've already computed the next intersection but said intersection is
        // on an elem on another processor. Therefore, the next proc will re-trace
        // this Ray on the perfect elem that we've picked (best_neighbor)
        ray->setCurrentPoint(_incoming_point);
        _intersection_distance = RayTracingCommon::invalid_distance;

        continueTraceOffProcessor(ray);
        return;
      }

      // Subdomain changed
      if (_current_elem->subdomain_id() != _current_subdomain_id)
        onSubdomainChanged(ray, /* same_ray = */ true);

      // Do own this element - tally the result as we're the ones tracing it
      storeExitsElemResult(exits_elem_result);
    }

    debugRay("Done with trace");
    debugRay("  _exits_elem: ", _exits_elem);
    debugRay("  _intersection_point: ", _intersection_point);
    debugRay("  _intersected_side: ", _intersected_side);
    debugRay("  _intersected_side centroid: ",
             _intersected_side == RayTracingCommon::invalid_side
                 ? RayTracingCommon::invalid_point
                 : _current_elem->side_ptr(_intersected_side)->vertex_average());
    debugRay("  _intersected_extrema = ", _intersected_extrema);
    debugRay("  _intersection_distance = ", _intersection_distance);

    // Increment intersections
    debugRay("Incrementing ray intersections by 1 to ", ray->intersections() + 1);
    ray->addIntersection();
    _results[INTERSECTIONS]++;

    // Increment distance
    ray->addDistance(_intersection_distance);
    debugRay("Incremented ray distance by ", _intersection_distance);

    // The effective max distance that the Ray should travel - minimum of the two
    const auto max_distance = std::min(ray->maxDistance(), _study.rayMaxDistance());
    debugRay("Max distance checks");
    debugRay("  _study.rayMaxDistance() = ", _study.rayMaxDistance());
    debugRay("  ray->maxDistance() = ", ray->maxDistance());
    debugRay("  max_distance (effective) = ", max_distance);

    // At the maximum distance - distinguish between this and past the maximum
    // distance because in this case we're close enough to the intersection point
    // that we can keep it, its intersected side, and intersected extrema
    if (MooseUtils::absoluteFuzzyEqual(ray->distance(), max_distance))
    {
      debugRay("At max distance");

      ray->setShouldContinue(false);
      _should_continue = false;
    }
    // Past the max distance - need to remove the additional distance we traveled,
    // change the point and invalidate the intersection data (moves the Ray back)
    else if (ray->distance() > max_distance)
    {
      debugRay("Past max distance");

      // The distance past the max distance we have traveled
      const auto difference = ray->distance() - max_distance;
      traceAssert(difference > 0, "Negative distance change after past_max_distance");

      debugRay("Removing distance ", difference);
      ray->addDistance(-difference);
      debugRay("  New ray->distance() = ", ray->distance());

      _intersection_point -= ray->direction() * difference;
      _intersection_distance -= difference;
      _intersected_side = RayTracingCommon::invalid_side;
      _intersected_extrema.invalidate();
      ray->setCurrentPoint(_intersection_point);

      ray->setShouldContinue(false);
      _should_continue = false;

      traceAssert(_intersection_distance >= 0, "Negative _intersection_distance");
#ifndef NDEBUG
      if (_study.verifyTraceIntersections() && !_current_elem->contains_point(_intersection_point))
        failTrace("Does not contain point after past max distance",
                  /* warning = */ false,
                  __LINE__);
#endif
    }

    if (!_study.currentRayKernels(_tid).empty())
    {
      debugRay("Calling onSegment() with");
      debugRay("  current_elem->id() = ", _current_elem->id());
      debugRay("  _incoming_point = ", _incoming_point);
      debugRay("  _incoming_side = ", _incoming_side);
      debugRay("  _intersection_point = ", _intersection_point);
      debugRay("  _intersected_side = ", _intersected_side);
      debugRay("  _intersected_extrema = ", _intersected_extrema);
      debugRay("  _intersection_distance = ", _intersection_distance);
      onSegment(ray);

      _study.postOnSegment(_tid, ray);

      // RayKernel killed a Ray or we're at the end
      traceAssert(_should_continue == ray->shouldContinue(), "Should be the same");
      if (!_should_continue)
      {
        debugRay("RayKernel killed the ray or past max distance");
        traceAssert(!ray->trajectoryChanged(),
                    "RayKernels should not change trajectories of Rays at end");

        onCompleteTrace(ray);
        return;
      }

      // RayKernel moved a Ray
      if (ray->trajectoryChanged())
      {
        debugRay("RayKernel changed the Ray's trajectory");
        debugRay("  new direction = ", ray->direction());
        debugRay("  old incoming point = ", _incoming_point);
        debugRay("  new incoming point = ", ray->currentPoint());
        possiblyAddDebugRayMeshPoint(_incoming_point, ray->currentPoint());

        _incoming_point = ray->currentPoint();
        _incoming_side = RayTracingCommon::invalid_side;
        _intersected_extrema.invalidate();
        ray->setCurrentIncomingSide(_incoming_side);

        const auto new_intersection_distance = (ray->currentPoint() - _incoming_point).norm();
        ray->addDistance(-_intersection_distance + new_intersection_distance);
        _intersection_distance = new_intersection_distance;

        onTrajectoryChanged(ray);
        onContinueTrace(ray);
        continue;
      }
      else
        possiblyAddDebugRayMeshPoint(_incoming_point, _intersection_point);
    }
    else
    {
      _study.postOnSegment(_tid, ray);

      if (!_should_continue)
      {
        debugRay("Killing due to at end without RayKernels");
        traceAssert(!ray->shouldContinue(), "Ray shouldn't continue");

        onCompleteTrace(ray);
        return;
      }
    }

    // If at a vertex/on an edge and not on the boundary, we may actually be on the boundary,
    // just not on a boundary side. Check for that here. If the domain is rectangular we will
    // do a quick check against the bounding box as if we're not on the boundary of the
    // bounding box for a rectangular problem, we can skip this.
    // TODO: see how much the tolerance for the bounding box check can be tightened
    if (_dim > 1 && _intersected_extrema.atExtrema() &&
        _current_elem->neighbor_ptr(_intersected_side) &&
        (!_is_rectangular_domain ||
         onBoundingBoxBoundary(_study.boundingBox(),
                               _intersection_point,
                               _dim,
                               LOOSE_TRACE_TOLERANCE * _study.domainMaxLength())))
    {
      auto boundary_side = RayTracingCommon::invalid_side;
      ElemExtrema boundary_extrema;
      const Elem * boundary_elem = nullptr;

      findExternalBoundarySide(boundary_side, boundary_extrema, boundary_elem);

      if (boundary_elem)
      {
        // At this point, the new incoming side is very difficult to find
        // and may require some re-tracing backwards. Let's not do that -
        // we don't need it to continue the trace. This is reason why
        // _incoming_side is not available in RayBCs.
        _last_elem = _current_elem;
        _current_elem = boundary_elem;
        _intersected_side = boundary_side;
        _intersected_extrema = boundary_extrema;
        ray->setCurrentElem(_current_elem);

        debugRay("Found a neighbor boundary side with:");
        debugRay("  _current_elem->id() = ", _current_elem->id());
        debugRay("  _intersected_side = ", _intersected_side);
        debugRay("  _intersected_extrema = ", _intersected_extrema);
      }
    }

    // The next element
    const Elem * neighbor = nullptr;

    // Set up where to go next
    _incoming_point = _intersection_point;
    debugRay("Set _incoming point to ", _incoming_point);

    debugRay("Looking for neighbor on side ", _intersected_side, " of elem ", _current_elem->id());

    // Get the neighbor on that side
    // If the mesh has active elements of the same level, just grab the neighbor directly. In
    // this case, we can avoid a call to active() on the neighbor in getActiveNeighbor(), which
    // can be expensive depending on caching.
    neighbor = _study.hasSameLevelActiveElems()
                   ? _current_elem->neighbor_ptr(_intersected_side)
                   : getActiveNeighbor(_current_elem, _intersected_side, _intersection_point);

    // Found one - not on the boundary so set the next info
    if (neighbor)
    {
      traceAssert(neighbor->active(), "Inactive neighbor");
      traceAssert(_current_elem->subdomain_id() == _current_subdomain_id,
                  "_current_subdomain_id invalid");

      // If the mesh has active elements of the same level, don't use
      // neighbor->which_neighbor_am_i(), which calls active() and an n_sides() virtual call
      if (_study.hasSameLevelActiveElems())
      {
        // If the subdomain hasn't changed, we can guarantee n_sides is the same as
        // _current_elem. Prefer this because neighbor->n_sides() is an expensive virtual. Even
        // better, subdomain_id() isn't a virtual!
        const unsigned short n_sides = neighbor->subdomain_id() == _current_subdomain_id
                                           ? _current_elem_n_sides
                                           : neighbor->n_sides();
        traceAssert(n_sides == neighbor->n_sides(), "n_sides incorrect");

        for (_incoming_side = 0; _incoming_side < n_sides; ++_incoming_side)
          if (neighbor->neighbor_ptr(_incoming_side) == _current_elem)
            break;
      }
      else
        _incoming_side = neighbor->which_neighbor_am_i(_current_elem);

      _last_elem = _current_elem;
      _current_elem = neighbor;
      ray->setCurrentElem(neighbor);
      ray->setCurrentIncomingSide(_incoming_side);

      debugRay("Next elem: ", neighbor->id(), " with centroid ", neighbor->vertex_average());
      debugRay("Next _incoming_side: ",
               _incoming_side,
               " with centroid ",
               neighbor->side_ptr(_incoming_side)->vertex_average());
      traceAssert(_last_elem->subdomain_id() == _current_subdomain_id,
                  "_current_subdomain_id invalid");

      // Whether or not the subdomain changed
      const bool subdomain_changed = neighbor->subdomain_id() != _current_subdomain_id;

      // See if we hit any internal sides leaving this element and apply
      // We require that all internal RayBCs be on internal sidesets that have different
      // subdomain ids on each side. Therefore, only check if at vertex/edge or if
      // the subdomain changes
      if (_study.hasInternalSidesets() && (subdomain_changed || _intersected_extrema.atExtrema()))
      {
        applyOnInternalBoundary(ray);

        // Internal RayBC killed a Ray
        if (!_should_continue)
        {
          traceAssert(!ray->shouldContinue(), "Should be the same");
          debugRay("Internal RayBC killed the ray");

          onCompleteTrace(ray);
          return;
        }

        // Internal RayBC changed the Ray
        if (ray->trajectoryChanged())
        {
          debugRay("Internal RayBC changed the trajectory:");
          debugRay("  new direction = ", ray->direction());

          // Is this side still incoming?
          const auto normal = _study.getSideNormal(_current_elem, _incoming_side, _tid);
          const auto dot = normal * ray->direction();
          debugRay("Dot product with new direction and side = ", dot);
          if (dot > -TRACE_TOLERANCE)
          {
            _incoming_side = _last_elem->which_neighbor_am_i(_current_elem);
            _current_elem = _last_elem;
            neighbor = _last_elem;
            ray->setCurrentElem(_current_elem);
            ray->setCurrentIncomingSide(_incoming_side);
            debugRay("  Dot > 0 (Ray turned around): Setting _current_elem = ",
                     _current_elem->id(),
                     " and _incoming_side = ",
                     _incoming_side);
          }

          onTrajectoryChanged(ray);
        }

        traceAssert(ray->currentPoint().absolute_fuzzy_equals(_intersection_point, TRACE_TOLERANCE),
                    "Internal RayBC changed the Ray point");
      }

      // Neighbor is off processor
      // If we hit at a vertex/edge, we will continue and let the move through neighbor exit
      // figure out who to send to
      if (neighbor->processor_id() != _pid)
      {
        if (_intersected_extrema.atExtrema())
        {
          debugRay("Neighbor is off processor but continuing to move through neighbors");
        }
        else
        {
          continueTraceOffProcessor(ray);
          return;
        }
      }

      // Neighbor is on processor, call subdomain setup if needed
      if (subdomain_changed)
        onSubdomainChanged(ray, /* same_ray = */ true);
    }
    // No neighbor found: on the boundary
    else
    {
      debugRay("No neighbor found - on the boundary");

      // Apply boundary conditions
      applyOnExternalBoundary(ray);

      traceAssert(ray->currentPoint().absolute_fuzzy_equals(_intersection_point, TRACE_TOLERANCE),
                  "RayBC changed the Ray point");

      // Quit tracing if the Ray was killed by a BC
      if (!_should_continue)
      {
        traceAssert(!ray->shouldContinue(), "Should be the same");
        debugRay("Exiting due to death by BC");

        onCompleteTrace(ray);
        return;
      }
      // RayBC changed the direction of the Ray
      if (ray->trajectoryChanged())
      {
        debugRay("RayBC changed the trajectory");
        debugRay("  new direction = ", ray->direction());
        traceAssert(ray->direction() *
                            _study.getSideNormal(_current_elem, _intersected_side, _tid) <
                        TRACE_TOLERANCE,
                    "Reflected ray is not incoming");
        possiblyAddDebugRayMeshPoint(_incoming_point, _intersection_point);

        _last_elem = _current_elem;
        _incoming_point = _intersection_point;
        _incoming_side = _intersected_side;
        ray->setCurrentPoint(_incoming_point);
        ray->setCurrentIncomingSide(_incoming_side);
        onTrajectoryChanged(ray);
      }
    }

    onContinueTrace(ray);
  } while (true);

  // If a trace made its way down here and didn't return... it failed
  failTrace("Could not find an intersection", _study.tolerateFailure(), __LINE__);
}

void
TraceRay::onCompleteTrace(const std::shared_ptr<Ray> & ray)
{
  debugRay("Called onCompleteTrace()\n", (*_current_ray)->getInfo());
  if (_intersection_distance > 0)
    possiblyAddDebugRayMeshPoint(_incoming_point, _intersection_point);
  possiblySaveDebugRayMesh();

  if (_current_cached_trace)
  {
    _current_cached_trace->_last = true;

    if (_intersection_distance > 0)
    {
      _current_cached_trace->addPoint(ray->currentPoint());
      if (_study.dataOnCacheTraces())
        _current_cached_trace->lastPoint()._data = ray->data();
      if (_study.auxDataOnCacheTraces())
        _current_cached_trace->lastPoint()._aux_data = ray->auxData();
    }
  }
}

void
TraceRay::onContinueTrace(const std::shared_ptr<Ray> & ray)
{
  traceAssert(ray->shouldContinue(), "Ray must continue");

  if (_current_cached_trace && _study.segmentsOnCacheTraces() && _intersection_distance > 0)
  {
    _current_cached_trace->addPoint(ray->currentPoint());
    if (_study.dataOnCacheTraces())
      _current_cached_trace->lastPoint()._data = ray->data();
    if (_study.auxDataOnCacheTraces())
      _current_cached_trace->lastPoint()._aux_data = ray->auxData();
  }
}

void
TraceRay::continueTraceOffProcessor(const std::shared_ptr<Ray> & ray)
{
  traceAssert(ray->currentElem() == _current_elem, "Ray currentElem() invalid");
  traceAssert(ray->currentIncomingSide() == _incoming_side, "Ray currentIncomingSide() invalid");
  traceAssert(ray->currentPoint() == _incoming_point, "Ray currentPoint() invalid");
  traceAssert(_current_elem->processor_id() != _pid, "Off processor trace is not off processor");
  debugRay("Ray going off processor to ", _current_elem->processor_id());

  ray->addProcessorCrossing();

  if (_current_cached_trace && _intersection_distance > 0)
  {
    _current_cached_trace->addPoint(_incoming_point);
    if (_study.dataOnCacheTraces())
      _current_cached_trace->lastPoint()._data = ray->data();
    if (_study.auxDataOnCacheTraces())
      _current_cached_trace->lastPoint()._aux_data = ray->auxData();
  }

  if (_intersection_distance > 0)
    possiblyAddDebugRayMeshPoint(_incoming_point, _intersection_point);
  possiblySaveDebugRayMesh();
}

void
TraceRay::onTrajectoryChanged(const std::shared_ptr<Ray> & ray)
{
#ifndef NDEBUG
  if (_study.verifyTraceIntersections() &&
      (_intersected_extrema.atExtrema()
           ? !_current_elem->close_to_point(ray->currentPoint(), LOOSE_TRACE_TOLERANCE)
           : !_current_elem->contains_point(ray->currentPoint())))
    failTrace("Elem does not contain point after trajectory change",
              /* warning = */ false,
              __LINE__);
#endif

  traceAssert(ray->shouldContinue(), "Ray should continue when trajectory is being changed");

  ray->setTrajectoryChanged(false);
  ray->addTrajectoryChange();

  if (_current_cached_trace && !_study.segmentsOnCacheTraces())
  {
    if (_intersection_distance > 0)
      _current_cached_trace->addPoint(ray->currentPoint());
    if (_study.dataOnCacheTraces())
      _current_cached_trace->lastPoint()._data = ray->data();
    if (_study.auxDataOnCacheTraces())
      _current_cached_trace->lastPoint()._aux_data = ray->auxData();
  }
}

void
TraceRay::onSubdomainChanged(const std::shared_ptr<Ray> & ray, const bool same_ray)
{
  debugRay("Calling onSubdomainChanged() on subdomain ", _current_elem->subdomain_id());
  debugRay("  _current_subdomain_id = ", _current_subdomain_id);

  _current_subdomain_id = _current_elem->subdomain_id();
  _current_subdomain_hmax = _study.subdomainHmax(_current_subdomain_id);
  _current_elem_type = _current_elem->type();
  _current_elem_n_sides = _current_elem->n_sides();

  if (_has_ray_kernels)
  {
    auto & current_ray_kernels = _study.currentRayKernels(_tid);

    // If we're still tracing the same Ray, keep track of our old RayKernels
    // so that we don't call preTrace() on them again
    if (same_ray)
      _old_ray_kernels.insert(current_ray_kernels.begin(), current_ray_kernels.end());
    // If we're not tracing the same Ray, we need to call preTrace() on everything
    else
      _old_ray_kernels.clear();

    // Call segmentSubdomainSetup to get new kernels etc
    _study.segmentSubdomainSetup(_current_subdomain_id, _tid, ray->id());

    // Call preTrace() on all of the RayKernels that need it
    for (RayKernelBase * rk : current_ray_kernels)
      // Haven't called preTrace() for this Ray on this RayKernel yet
      if (!_old_ray_kernels.count(rk))
        rk->preTrace();
  }
}

std::string
TraceRay::failTraceMessage(const std::string & reason, const int line)
{
  std::stringstream oss;
  oss << "Ray on processor " << _pid << " and thread " << _tid << " failed to trace";
  if (line != -1)
    oss << " at line " << line;
  oss << "\n\n" << reason << "\n\n";
  oss << ((*_current_ray))->getInfo() << "\n";
  oss << "Current trace information\n";
  oss << "  _current_subdomain_id = ";
  if (_current_subdomain_id == Elem::invalid_subdomain_id)
    oss << "invalid subdomain id\n";
  else
    oss << _current_subdomain_id << "\n";
  oss << "  _current_elem_type = " << Utility::enum_to_string(_current_elem_type) << "\n";
  oss << "  _current_elem_n_sides = " << _current_elem_n_sides << "\n";
  oss << "  _incoming_point = ";
  if (_incoming_point == RayTracingCommon::invalid_point)
    oss << "invalid point\n";
  else
    oss << _incoming_point << "\n";
  oss << "  _incoming_side = ";
  if (_incoming_side == RayTracingCommon::invalid_side)
    oss << "invalid side\n";
  else
    oss << _incoming_side << "\n";
  oss << "  _intersection_point = ";
  if (_intersection_point == RayTracingCommon::invalid_point)
    oss << "invalid point\n";
  else
    oss << _intersection_point << "\n";
  oss << "  _intersected_side = ";
  if (_intersected_side == RayTracingCommon::invalid_side)
    oss << "invalid side\n";
  else
    oss << _intersected_side << "\n";
  oss << "  _intersected_extrema = " << _intersected_extrema << "\n";
  oss << "  _exits_elem = " << _exits_elem << "\n";
  if (_current_elem)
    oss << _current_elem->get_info();
  else
    oss << "_current_elem = invalid\n";

  possiblySaveDebugRayMesh();
  return oss.str();
}

void
TraceRay::failTrace(const std::string & reason, const bool warning, const int line)
{
  const auto message = failTraceMessage(reason, line);

  if (warning)
  {
    ++_results[FAILED_TRACES];
    mooseWarning(message);
    (*_current_ray)->setShouldContinue(false);
    _should_continue = false;
  }
  else
    mooseError(message);
}

const std::vector<NeighborInfo> &
TraceRay::getVertexNeighbors(const Elem * elem, const Node * vertex)
{
  traceAssert(elem, "Elem must be valid");
  traceAssert(vertex, "Vertex must be valid");

  debugRay("Called getVertexNeighbors() with:");
  debugRay("  elem->id() = ", elem->id(), " with centroid ", elem->vertex_average());
  debugRay("  vertex->id() = ", vertex->id(), ", at ", (Point)*vertex);

  traceAssert(elem->get_node_index(vertex) != libMesh::invalid_uint, "Doesn't contain node");
  traceAssert(elem->is_vertex(elem->get_node_index(vertex)), "Node is not a vertex");

  ++_results[VERTEX_NEIGHBOR_LOOKUPS];

  // Return the entry if we have it cached
  auto search = _vertex_neighbors.find(vertex);
  if (search != _vertex_neighbors.end())
    return search->second;

  ++_results[VERTEX_NEIGHBOR_BUILDS];

  // Make a new entry
  debugRay("Building vertex neighbors");
  std::vector<NeighborInfo> & entry =
      _vertex_neighbors.emplace(vertex, std::vector<NeighborInfo>()).first->second;

  findNodeNeighbors(elem,
                    vertex,
                    _neighbor_set,
                    _neighbor_untested_set,
                    _neighbor_next_untested_set,
                    _neighbor_active_neighbor_children,
                    entry);

  // Fill the side normals
  for (auto & neighbor_info : entry)
    for (MooseIndex(neighbor_info._sides.size()) i = 0; i < neighbor_info._sides.size(); ++i)
      neighbor_info._side_normals[i] =
          _study.getSideNormal(neighbor_info._elem, neighbor_info._sides[i], _tid);

  return entry;
}

const std::vector<NeighborInfo> &
TraceRay::getVertexNeighbors(const Elem * elem, const unsigned short vertex)
{
  traceAssert(vertex < elem->n_vertices(), "Invalid vertex");

  return getVertexNeighbors(elem, elem->node_ptr(vertex));
}

const std::vector<NeighborInfo> &
TraceRay::getEdgeNeighbors(const Elem * elem,
                           const std::pair<const Node *, const Node *> & vertices,
                           const Point & point)
{
  traceAssert(elem, "Invalid elem");
  traceAssert(vertices.first, "Must be valid");
  traceAssert(vertices.second, "Must be valid");

  debugRay("Called getEdgeNeighbors() with:");
  debugRay("  elem->id() = ", elem->id(), " with centroid ", elem->vertex_average());
  debugRay("  vertices.first = ", vertices.first->id(), " at ", (Point)*vertices.first);
  debugRay("  vertices.second = ", vertices.second->id(), " at ", (Point)*vertices.second);
  debugRay("  point = ", point);

  traceAssert(elem->get_node_index(vertices.first) != libMesh::invalid_uint,
              "Doesn't contain vertex");
  traceAssert(elem->get_node_index(vertices.second) != libMesh::invalid_uint,
              "Doesn't contain vertex");
  traceAssert(isWithinSegment(
                  (Point)*vertices.first, (Point)*vertices.second, point, LOOSE_TRACE_TOLERANCE),
              "Point not within edge");

  ++_results[EDGE_NEIGHBOR_LOOKUPS];

  const auto ordered_vertices = vertices.first->id() < vertices.second->id()
                                    ? vertices
                                    : std::make_pair(vertices.second, vertices.first);

  // Look for the entry and build if necessary
  std::pair<bool, std::vector<NeighborInfo>> * entry;
  auto search = _edge_neighbors.find(ordered_vertices);
  if (search != _edge_neighbors.end())
    entry = &search->second;
  else
  {
    debugRay("Building edge neighbors");
    ++_results[EDGE_NEIGHBOR_BUILDS];
    entry = &_edge_neighbors
                 .emplace(ordered_vertices, std::make_pair(true, std::vector<NeighborInfo>()))
                 .first->second;
    findEdgeNeighbors(elem,
                      ordered_vertices.first,
                      ordered_vertices.second,
                      _neighbor_set,
                      _neighbor_untested_set,
                      _neighbor_next_untested_set,
                      _neighbor_active_neighbor_children,
                      entry->second);

    bool all_same_edge = true;
    for (auto & neighbor_info : entry->second)
    {
      traceAssert(neighbor_info._lower_bound <= neighbor_info._upper_bound,
                  "Bound order incorrect");

      // Fill the side normals
      for (MooseIndex(neighbor_info._sides.size()) i = 0; i < neighbor_info._sides.size(); ++i)
        neighbor_info._side_normals[i] =
            _study.getSideNormal(neighbor_info._elem, neighbor_info._sides[i], _tid);

      // See if the bounds are the same as the target edge
      if (neighbor_info._lower_bound != 0 || neighbor_info._upper_bound != 1)
        all_same_edge = false;
    }
    entry->first = all_same_edge;
  }

  // Means that all neighbors are not on the exact same edge, so we must
  // validate/invalidate based on if the neighbor's edge contains our point
  if (!entry->first)
  {
    const auto edge_length =
        ((Point)*ordered_vertices.first - (Point)*ordered_vertices.second).norm();
    const auto point_location = ((Point)*ordered_vertices.first - point).norm() / edge_length;
    for (auto & info : entry->second)
      info._valid = (info._lower_bound - TRACE_TOLERANCE) < point_location &&
                    point_location < (info._upper_bound + TRACE_TOLERANCE);
  }

  return entry->second;
}

const std::vector<NeighborInfo> &
TraceRay::getEdgeNeighbors(const Elem * elem,
                           const std::pair<unsigned short, unsigned short> & vertices,
                           const Point & point)
{
  debugRay("Called getEdgeNeighbors(), local index version with:");
  debugRay("  vertices.first = ", vertices.first);
  debugRay("  vertices.second = ", vertices.second);
  traceAssert(vertices.first < elem->n_vertices(),
              "Invalid vertex with ray " + std::to_string((*_current_ray)->id()));
  traceAssert(vertices.second < elem->n_vertices(), "Invalid vertex");

  return getEdgeNeighbors(
      elem, std::make_pair(elem->node_ptr(vertices.first), elem->node_ptr(vertices.second)), point);
}

const std::vector<NeighborInfo> &
TraceRay::getNeighbors(const Elem * elem, const ElemExtrema & extrema, const Point & point)
{
  if (!extrema.atExtrema())
    return getPointNeighbors(elem, point);
  if (extrema.atVertex())
    return getVertexNeighbors(elem, extrema.vertex());
  return getEdgeNeighbors(elem, extrema.edgeVertices(), point);
}

const std::vector<NeighborInfo> &
TraceRay::getPointNeighbors(const Elem * elem, const Point & point)
{
  traceAssert(elem, "Invalid elem");

  debugRay("Called getPointNeighbors()");
  debugRay(" elem = ", elem->id());
  debugRay(" point = ", point);

  ++_results[POINT_NEIGHBOR_BUILDS];
  _point_neighbor_helper.clear();

  findPointNeighbors(elem,
                     point,
                     _neighbor_set,
                     _neighbor_untested_set,
                     _neighbor_next_untested_set,
                     _neighbor_active_neighbor_children,
                     _point_neighbor_helper);

  // Fill the side normals
  for (auto & neighbor_info : _point_neighbor_helper)
    for (MooseIndex(neighbor_info._sides.size()) i = 0; i < neighbor_info._sides.size(); ++i)
      neighbor_info._side_normals[i] =
          _study.getSideNormal(neighbor_info._elem, neighbor_info._sides[i], _tid);

  return _point_neighbor_helper;
}

void
TraceRay::storeExitsElemResult(const TraceRay::ExitsElemResult result)
{
  if (result == HIT_FACE)
    ++_results[FACE_HITS];
  else if (result == HIT_VERTEX)
    ++_results[VERTEX_HITS];
  else if (result == HIT_EDGE)
    ++_results[EDGE_HITS];
  else
    mooseError("Should not call storeExitsElemResult() with result ", result);
}

void
TraceRay::onSegment(const std::shared_ptr<Ray> & ray)
{
  traceAssert((*_current_ray)->currentElem() == _current_elem, "Ray currentElem() incorrect");
  traceAssert((*_current_ray)->currentPoint() == _intersection_point,
              "Ray currentPoint() incorrect");
  traceAssert((*_current_ray)->currentIncomingSide() == _incoming_side,
              "Ray currentIncomingSide() incorrect");
#ifndef NDEBUG
  if (_study.verifyTraceIntersections())
  {
    if (_current_elem->has_affine_map())
      traceAssert(_current_elem->contains_point(_incoming_point),
                  "_current_elem does not contain incoming point");

    if (_intersected_side != RayTracingCommon::invalid_side &&
        !_study.sideIsNonPlanar(_current_elem, _intersected_side))
    {
      traceAssert(_elem_side_builder(*_current_elem, _intersected_side)
                      .close_to_point(_intersection_point, LOOSE_TRACE_TOLERANCE),
                  "Intersected point is not on intersected side");
      traceAssert(!_study.sideIsIncoming(
                      _current_elem, _intersected_side, (*_current_ray)->direction(), _tid),
                  "Intersected side is not outgoing");
    }
    if (_incoming_side != RayTracingCommon::invalid_side &&
        !_study.sideIsNonPlanar(_current_elem, _incoming_side))
    {
      traceAssert(_elem_side_builder(*_current_elem, _incoming_side)
                      .close_to_point(_incoming_point, LOOSE_TRACE_TOLERANCE),
                  "Incoming point is not on incoming side");
      traceAssert(
          _study.sideIsIncoming(_current_elem, _incoming_side, (*_current_ray)->direction(), _tid),
          "Incoming side is not incoming");
    }
  }
#endif
  traceAssert(MooseUtils::absoluteFuzzyEqual(_intersection_distance,
                                             (_intersection_point - _incoming_point).norm()),
              "_intersection_distance is incorrect");
  traceAssert(_current_subdomain_id == _current_elem->subdomain_id(), "Subdomain incorrect");
  traceAssert(MooseUtils::absoluteFuzzyEqual((_incoming_point - _intersection_point).norm(),
                                             _intersection_distance),
              "Invalid intersection distance");

  _study.reinitSegment(
      _current_elem, _incoming_point, _intersection_point, _intersection_distance, _tid);

  const auto & rks = _study.currentRayKernels(_tid);
  for (auto & rk : rks)
  {
    rk->onSegment();
    postRayTracingObject(ray, rk);
  }
}

void
TraceRay::onBoundary(const std::shared_ptr<Ray> & ray, const bool external)
{
  traceAssert(ray->currentPoint().absolute_fuzzy_equals(_intersection_point),
              "Ray currentPoint() not set before onBoundary()");

  // Get the RayBCs on bnd_elems
  _study.getRayBCs(_on_boundary_ray_bcs, _boundary_elems, _tid, ray->id());

  // Store this information temprorarily because we are going to change it as we
  // apply each boundary condition
  const auto old_current_elem = _current_elem;
  const auto old_intersected_side = _intersected_side;
  const auto old_intersected_extrema = _intersected_extrema;
  const auto old_subdomain_id = _current_subdomain_id;

  // For each RayBC we found, apply it on the boundaries that we need
  for (RayBoundaryConditionBase * rbc : _on_boundary_ray_bcs)
  {
    // First, find the boundaries this RayBC is valid on that are also in _boundary_elems.
    // We do this ahead of time so that we can pass in to RayBC::apply if the same
    // boundary condition is being appled multiple times on different boundaries.
    // This is useful in situations like reflection where multiple reflections are
    // necessary at a corner to perfectly reflect.
    _on_boundary_apply_index.clear();
    for (MooseIndex(_boundary_elems.size()) bnd_elems_i = 0; bnd_elems_i < _boundary_elems.size();
         ++bnd_elems_i)
      if (rbc->hasBoundary(_boundary_elems[bnd_elems_i].bnd_id))
        _on_boundary_apply_index.push_back(bnd_elems_i);

    traceAssert(!_on_boundary_apply_index.empty(), "Must not be empty");

    // Apply the RayBC on each of the relevant boundary elements
    for (const auto bnd_elems_i : _on_boundary_apply_index)
    {
      auto & bnd_elem = _boundary_elems[bnd_elems_i];
      _current_elem = bnd_elem.elem;
      _current_bnd_id = bnd_elem.bnd_id;
      _intersected_side = bnd_elem.side;
      _intersected_extrema = bnd_elem.extrema;
      _current_subdomain_id = _current_elem->subdomain_id();

      rbc->onBoundary(_on_boundary_apply_index.size());
      postRayTracingObject(ray, rbc);
    }
  }

  // Set this info back now that we're done applying BCs
  _current_elem = old_current_elem;
  _intersected_side = old_intersected_side;
  _intersected_extrema = old_intersected_extrema;
  _current_bnd_id = BoundaryInfo::invalid_id;
  _current_subdomain_id = old_subdomain_id;

  // When on an external boundary, the Ray must have been changed or killed.
  // Otherwise, we don't know what to do with it now! If this didn't happen,
  // output a detailed error message.
  if (external && !ray->trajectoryChanged() && ray->shouldContinue())
  {
    std::stringstream oss;
    oss << "Don't know what to do with a Ray after it hit an external\n";
    oss << "boundary at point " << _intersection_point << "!\n\n";
    oss << "When hitting an external RayBC, a Ray must either:\n";
    oss << "  Be killed by a RayBC\n";
    oss << "  Have its trajectory changed by the RayBC\n";
    oss << "by at least one of the executed RayBCs.\n\n";
    oss << "You need to either:\n";
    oss << "  Kill/change the Ray sooner with RayKernels, internal RayBCs, or a max distance\n";
    oss << "  Kill/change the Ray on the boundary with a RayBC\n\n";
    if (!_on_boundary_ray_bcs.empty())
    {
      oss << "RayBCs executed that did not kill or change the Ray:\n";
      for (const RayBoundaryConditionBase * rbc : _on_boundary_ray_bcs)
        for (const auto & bnd_elem : _boundary_elems)
          if (rbc->hasBoundary(bnd_elem.bnd_id))
            oss << "  " << rbc->typeAndName() << " on boundary " << bnd_elem.bnd_id << " ("
                << _mesh.getBoundaryName(bnd_elem.bnd_id) << ")\n";
      oss << "\n";
    }
    bool output_header = false;
    for (std::size_t i = 0; i < _boundary_elems.size(); ++i)
    {
      const auto bnd_id = _boundary_elems[i].bnd_id;
      bool found = false;
      for (const RayBoundaryConditionBase * rbc : _on_boundary_ray_bcs)
        if (rbc->hasBoundary(bnd_id))
        {
          found = true;
          break;
        }

      if (!found)
      {
        if (!output_header)
        {
          oss << "Boundaries that did not have any RayBCs:\n";
          output_header = true;
        }
        oss << "  " << bnd_id << " (" << _mesh.getBoundaryName(bnd_id) << ")\n";
      }
    }

    failTrace(oss.str(), _study.tolerateFailure(), __LINE__);
  }
}

Real
TraceRay::subdomainHmax(const Elem * elem) const
{
  const auto subdomain_id = elem->subdomain_id();
  return subdomain_id == _current_subdomain_id ? _current_subdomain_hmax
                                               : _study.subdomainHmax(subdomain_id);
}

void
TraceRay::postRayTracingObject(const std::shared_ptr<Ray> & ray, const RayTracingObject * rto)
{
  if (!ray->shouldContinue())
  {
    if (_should_continue)
      _should_continue = false;
  }
  else if (!_should_continue)
    failTrace(rto->typeAndName() +
                  " set a Ray to continue that was previously set to not continue.\n\n" +
                  "Once a Ray has been set to not continue, its continue status cannot change.",
              /* warning = */ false,
              __LINE__);

  if (!_should_continue && ray->trajectoryChanged())
    failTrace(rto->typeAndName() +
                  " changed the trajectory of a Ray that was set to not continue,\n" +
                  "or set a Ray whose trajectory was changed to not continue.",
              /* warning = */ false,
              __LINE__);
}
