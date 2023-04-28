//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/primary/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshAlignmentBase.h"

#include "libmesh/elem.h"

MeshAlignmentBase::MeshAlignmentBase(const MooseMesh & mesh) : _mesh(mesh) {}

void
MeshAlignmentBase::extractFrom1DElements(const std::vector<dof_id_type> & elem_ids,
                                         std::vector<Point> & elem_points,
                                         std::vector<dof_id_type> & node_ids,
                                         std::vector<Point> & node_points) const
{
  elem_points.clear();
  node_ids.clear();
  node_points.clear();

  for (const auto & elem_id : elem_ids)
  {
    const Elem * elem = _mesh.elemPtr(elem_id);
    elem_points.push_back(elem->vertex_average());

    for (const auto j : elem->node_index_range())
    {
      const Node & node = elem->node_ref(j);
      const auto node_id = node.id();
      if (std::find(node_ids.begin(), node_ids.end(), node_id) == node_ids.end())
      {
        node_ids.push_back(node_id);
        node_points.push_back(node);
      }
    }
  }
}

void
MeshAlignmentBase::extractFromBoundaryInfo(
    const std::vector<std::tuple<dof_id_type, unsigned short int>> & boundary_info,
    std::vector<dof_id_type> & elem_ids,
    std::vector<unsigned short int> & side_ids,
    std::vector<Point> & side_points,
    std::vector<dof_id_type> & node_ids,
    std::vector<Point> & node_points) const
{
  elem_ids.clear();
  side_ids.clear();
  side_points.clear();
  node_ids.clear();
  node_points.clear();

  for (const auto & elem_id_and_side : boundary_info)
  {
    auto elem_id = std::get<0>(elem_id_and_side);
    elem_ids.push_back(elem_id);

    auto side = std::get<1>(elem_id_and_side);
    side_ids.push_back(side);

    const Elem * elem = _mesh.elemPtr(elem_id);
    const Elem * side_elem = elem->build_side_ptr(side).release();
    const Point side_center = side_elem->vertex_average();
    side_points.push_back(side_center);

    for (const auto j : side_elem->node_index_range())
    {
      const Node & node = side_elem->node_ref(j);
      const auto node_id = node.id();
      if (std::find(node_ids.begin(), node_ids.end(), node_id) == node_ids.end())
      {
        node_ids.push_back(node_id);
        node_points.push_back(node);
      }
    }
    delete side_elem;
  }
}
