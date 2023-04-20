//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayTracingOverlayMeshMapping.h"
#include "LineSegment.h"
#include <timpi/parallel_sync.h>
#include <chrono>
#include <memory>
// libMesh includes
#include "libmesh/enum_point_locator_type.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel_sync.h"
#include "libmesh/parallel_elem.h"
#include "libmesh/parallel_node.h"
#include "libmesh/plane.h"
#include "libmesh/mesh_serializer.h"

registerMooseObject("RayTracingApp", RayTracingOverlayMeshMapping);

InputParameters
RayTracingOverlayMeshMapping::validParams()
{
  auto params = GeneralUserObject::validParams();

  params.addRequiredParam<MeshGeneratorName>("overlay_mesh", "The base mesh we want to overlay");

  return params;
}

RayTracingOverlayMeshMapping::RayTracingOverlayMeshMapping(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _main_mesh_name("main_mesh"),
    _overlay_mesh_name(getParam<MeshGeneratorName>("overlay_mesh")),
    _main_mesh(_fe_problem.mesh()),
    _overlay_mesh(_app.getMeshGeneratorSystem().getSavedMesh(_overlay_mesh_name))
{
  // Make sure both meshes are serial; we need this only for now and in the future
  // we should remove this requirement and support distributed meshes
  MeshSerializer serialize_mesh(_main_mesh);
  MeshSerializer serialize_overlay_mesh(*_overlay_mesh);
}

void
RayTracingOverlayMeshMapping::initialize()
{
  ElemIntersectionMap();
}

void
RayTracingOverlayMeshMapping::ElemIntersectionMap()
{
  for (const auto & elem : _main_mesh.active_local_element_ptr_range())
  {
    std::vector<const Elem *> intersect_elems;
    if (elem->on_boundary())
    {
      for (const auto & overlay_elem : _overlay_mesh->active_element_ptr_range())
      {
        if (isElemIntersection(elem, overlay_elem))
          intersect_elems.push_back(overlay_elem);
      }
      if (!intersect_elems.empty())
        _to_overlay[elem] = intersect_elems;
    }
    intersect_elems.clear();
  }

  for (const auto & overlay_elem : _overlay_mesh->active_local_element_ptr_range())
  {
    std::vector<const Elem *> intersect_elems;
    for (const auto & elem : _main_mesh.active_element_ptr_range())
    {
      if (isElemIntersection(elem, overlay_elem) && elem->on_boundary())
        intersect_elems.push_back(elem);
    }
    if (!intersect_elems.empty())
      _from_overlay[overlay_elem] = intersect_elems;
    intersect_elems.clear();
  }
}

bool
RayTracingOverlayMeshMapping::isElemIntersection(const Elem * elem, const Elem * cut_elem)
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
RayTracingOverlayMeshMapping::edgesIntersect(const Elem & edge1,
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
RayTracingOverlayMeshMapping::edgeIntersectsSide(const Elem & edge,
                                                 const Elem & side,
                                                 Point & intersection_point) const
{
  const LineSegment segment(edge.point(0), edge.point(1));
  const Plane plane(side.point(0), side.point(1), side.point(2));
  return segment.intersect(plane, intersection_point) && side.contains_point(intersection_point) &&
         edge.contains_point(intersection_point);
}

std::map<dof_id_type, std::set<dof_id_type>>
RayTracingOverlayMeshMapping::overlayIDMap(const bool to_overlay, const bool serialize) const
{
  const auto & from_map = to_overlay ? _to_overlay : _from_overlay;
  std::map<dof_id_type, std::set<dof_id_type>> overlay_map;
  std::map<dof_id_type, std::set<dof_id_type>> main_overlay_map;

  for (const auto & [elem, to_elems] : from_map)
  {
    auto & map_entry = overlay_map[elem->id()];
    for (const auto & to_elem : to_elems)
      map_entry.insert(to_elem->id());
  }

  if (serialize)
  {
    sync_map(overlay_map, main_overlay_map);
    return main_overlay_map;
  }
  return overlay_map;
}

void
RayTracingOverlayMeshMapping::sync_map(
    std::map<dof_id_type, std::set<dof_id_type>> & local_overlay_map,
    std::map<dof_id_type, std::set<dof_id_type>> & main_overlay_map) const
{
  std::map<processor_id_type, std::vector<std::tuple<dof_id_type, std::set<dof_id_type>>>>
      data_to_send;

  for (const auto & [elem, to_elems] : local_overlay_map)
  {
    data_to_send[0].emplace_back(elem, to_elems);
  }

  const auto receive_data = [&main_overlay_map](const processor_id_type, const auto & data)
  {
    for (const auto & [elem, to_elems] : data)
    {
      auto & map_entry = main_overlay_map[elem];
      for (const auto & to_elem : to_elems)
        map_entry.insert(to_elem);
    }
  };

  TIMPI::push_parallel_vector_data(comm(), data_to_send, receive_data);
}
