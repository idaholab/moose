//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundingBoxIntersectionHelper.h"

// libMesh includes
#include "libmesh/bounding_box.h"
#include "libmesh/face_quad4.h"
#include "libmesh/cell_hex8.h"

#include "TraceRayTools.h"
#include "DebugRay.h"

using namespace libMesh;

BoundingBoxIntersectionHelper::BoundingBoxIntersectionHelper(const BoundingBox & bbox,
                                                             const unsigned int dim)
  : _comm(), _mesh(std::make_unique<Mesh>(_comm, dim))
{
  // Add the nodes that represent our bounding box
  _mesh->add_point(Point(bbox.min()(0), bbox.min()(1), bbox.min()(2)), 0, 0);
  _mesh->add_point(Point(bbox.max()(0), bbox.min()(1), bbox.min()(2)), 1, 0);
  if (dim > 1)
  {
    _mesh->add_point(Point(bbox.max()(0), bbox.max()(1), bbox.min()(2)), 2, 0);
    _mesh->add_point(Point(bbox.min()(0), bbox.max()(1), bbox.min()(2)), 3, 0);

    if (dim == 3)
    {
      _mesh->add_point(Point(bbox.min()(0), bbox.min()(1), bbox.max()(2)), 4, 0);
      _mesh->add_point(Point(bbox.max()(0), bbox.min()(1), bbox.max()(2)), 5, 0);
      _mesh->add_point(Point(bbox.max()(0), bbox.max()(1), bbox.max()(2)), 6, 0);
      _mesh->add_point(Point(bbox.min()(0), bbox.max()(1), bbox.max()(2)), 7, 0);
    }
  }

  // Build the element and set the nodes accordingly
  Elem * elem;
  if (dim == 1)
    elem = Elem::build(EDGE2).release();
  else if (dim == 2)
    elem = Elem::build(QUAD4).release();
  else
    elem = Elem::build(HEX8).release();
  elem = _mesh->add_elem(elem);
  elem->subdomain_id() = 0;
  elem->processor_id(0);
  elem->set_id() = 0;
  for (unsigned int n = 0; n < _mesh->n_nodes(); ++n)
    elem->set_node(n) = _mesh->node_ptr(n);

  // All done with the mesh
  _mesh->skip_partitioning(true);
  _mesh->prepare_for_use();

  // This costs a little bit so why not cache it now
  _hmax = elem->hmax();
}

Point
BoundingBoxIntersectionHelper::intersection(const Point & point, const Point & direction) const
{
  mooseAssert(std::abs(1.0 - direction.norm()) < TOLERANCE, "Unnormalized direction");

  const Elem * elem = _mesh->elem_ptr(0);

  // We're going to check all possible intersections and keep the one that's furthest away
  Point best_intersection_point = RayTracingCommon::invalid_point;
  Real best_intersection_distance = -std::numeric_limits<Real>::max();

  // For use in the intersection routines
  Point intersection_point;
  ElemExtrema intersected_extrema;
  Real intersection_distance;
  bool intersected;

  // Check intersection with sides
  for (const auto s : elem->side_index_range())
  {
    intersection_point = RayTracingCommon::invalid_point;
    intersected_extrema.invalidate();

    if (elem->dim() == 3) // 3D: Intersect HEX8
      intersected = TraceRayTools::sideIntersectedByLine<Hex8>(elem,
                                                               point,
                                                               direction,
                                                               s,
                                                               _hmax,
                                                               intersection_point,
                                                               intersection_distance,
                                                               intersected_extrema,
                                                               _hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                                                               ,
                                                               false
#endif
      );
    else if (elem->dim() == 2) // 2D: Intersect QUAD4
    {
      intersected = TraceRayTools::sideIntersectedByLine<Quad4>(elem,
                                                                point,
                                                                direction,
                                                                s,
                                                                _hmax,
                                                                intersection_point,
                                                                intersection_distance,
                                                                intersected_extrema,
                                                                _hmax
#ifdef DEBUG_RAY_INTERSECTIONS
                                                                ,
                                                                false
#endif
      );
    }
    else // 1D: see if side point is between point and end far away
    {
      // Dummy end point outside of the element
      const Point dummy_end = point + 1.01 * direction * _hmax;

      intersected = TraceRayTools::isWithinSegment(point, dummy_end, elem->point(s));
      if (intersected)
      {
        intersection_distance = (elem->point(s) - point).norm();
        intersection_point = elem->point(s);
      }
    }

    if (intersected && intersection_distance > best_intersection_distance)
    {
      best_intersection_distance = intersection_distance;
      best_intersection_point = intersection_point;
    }
  }

  return best_intersection_point;
}
