//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IntersectionElemsHelper.h"
#include "LineSegment.h"

// libMesh includes
#include "libmesh/bounding_box.h"
#include "libmesh/enum_point_locator_type.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel_sync.h"
#include "libmesh/parallel_elem.h"
#include "libmesh/parallel_node.h"
#include "libmesh/plane.h"
#include "libmesh/mesh_serializer.h"

#include "TraceRayTools.h"
#include "DebugRay.h"

IntersectionElemsHelper::IntersectionElemsHelper(MeshBase & mesh, MeshBase & overlay_mesh)
  : _mesh(mesh), _overlay_mesh(overlay_mesh)
{
  // Make sure both meshes are serial; we need this only for now and in the future
  // we should remove this requirement and support distributed meshes
  MeshSerializer serialize_mesh(mesh);
  MeshSerializer serialize_overlay_mesh(overlay_mesh);
}

void
IntersectionElemsHelper::ElemIntersectionMap()
{
  for (const auto & elem : _mesh.active_local_element_ptr_range())
  {

    std::vector<const Elem *> intersect_elems;
    if (elem->on_boundary())
    {
      for (const auto & overlay_elem : _overlay_mesh.active_local_element_ptr_range())
      {
        if (isElemIntersection(elem, overlay_elem))
          intersect_elems.push_back(overlay_elem);
      }
      if (!intersect_elems.empty())
        _main_elems_to_overlay[elem] = intersect_elems;
    }
    intersect_elems.clear();
  }

  for (const auto & overlay_elem : _overlay_mesh.active_local_element_ptr_range())
  {
    std::vector<const Elem *> intersect_elems;
    for (const auto & elem : _mesh.active_local_element_ptr_range())
    {
      if (isElemIntersection(elem, overlay_elem) && elem->on_boundary())
        intersect_elems.push_back(elem);
    }
    if (!intersect_elems.empty())
      _overlay_elems_to_main[overlay_elem] = intersect_elems;
    intersect_elems.clear();
  }
}

bool
IntersectionElemsHelper::isElemIntersection(const Elem * elem, const Elem * cut_elem)
{
  if (elem->dim() != cut_elem->dim())
    mooseError("Cannot use cutElemIntersectionPoints() with elems of different dimension.");
  if (elem->dim() == 1)
    mooseError("cutElemIntersectionPoints() works with only 2D and 3D elems.");

  Point point;

  // Add intersections with edges on each element (sides in 2D)
  for (const auto e : elem->edge_index_range())
  {
    const auto edge = elem->build_edge_ptr(e);
    for (const auto cut_e : cut_elem->edge_index_range())
      if (edgesIntersect(*edge, *cut_elem->build_edge_ptr(cut_e), point))
        return true;
  }

  // Edge and side intersections for 3D
  if (elem->dim() == 3)
  {
    // Add intersections with sides on elem and edges on cut_elem
    for (const auto s : elem->side_index_range())
    {
      const auto & side = _elem_side_builder(*elem, s);
      for (const auto cut_e : cut_elem->edge_index_range())
        if (edgeIntersectsSide(*cut_elem->build_edge_ptr(cut_e), side, point))
          return true;
    }
  }
  return false;
}

bool
IntersectionElemsHelper::edgesIntersect(const Elem & edge1,
                                        const Elem & edge2,
                                        Point & intersection_point) const
{
  mooseAssert(edge1.dim() == 1 && edge2.dim() == 1, "Not edges");
  const LineSegment segment1(edge1.point(0), edge1.point(1));
  const LineSegment segment2(edge2.point(0), edge2.point(1));
  return segment1.intersect(segment2, intersection_point) &&
         edge1.contains_point(intersection_point) && edge2.contains_point(intersection_point);
}

bool
IntersectionElemsHelper::edgeIntersectsSide(const Elem & edge,
                                            const Elem & side,
                                            Point & intersection_point) const
{
  const LineSegment segment(edge.point(0), edge.point(1));
  const Plane plane(side.point(0), side.point(1), side.point(2));
  return segment.intersect(plane, intersection_point) && side.contains_point(intersection_point) &&
         edge.contains_point(intersection_point);
}
