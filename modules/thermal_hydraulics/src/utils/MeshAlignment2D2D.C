//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/primary/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshAlignment2D2D.h"
#include "KDTree.h"

#include "libmesh/elem.h"

MeshAlignment2D2D::MeshAlignment2D2D(const MooseMesh & mesh)
  : _mesh(mesh), _all_points_are_coincident(false)
{
}

void
MeshAlignment2D2D::initialize(
    const std::vector<std::tuple<dof_id_type, unsigned short int>> & primary_boundary_info,
    const std::vector<std::tuple<dof_id_type, unsigned short int>> & secondary_boundary_info)
{
  // Extract primary boundary info
  _primary_elem_ids.clear();
  std::vector<Point> primary_side_points;
  std::vector<dof_id_type> primary_node_ids;
  std::vector<Point> primary_node_points;
  for (const auto & elem_id_and_side : primary_boundary_info)
  {
    auto elem_id = std::get<0>(elem_id_and_side);
    auto side = std::get<1>(elem_id_and_side);
    const Elem * elem = _mesh.elemPtr(elem_id);
    const Elem * side_elem = elem->build_side_ptr(side).release();
    const Point side_center = side_elem->vertex_average();
    for (const auto j : side_elem->node_index_range())
    {
      const Node & node = side_elem->node_ref(j);
      const auto node_id = node.id();
      if (std::find(primary_node_ids.begin(), primary_node_ids.end(), node_id) ==
          primary_node_ids.end())
      {
        primary_node_ids.push_back(node_id);
        primary_node_points.push_back(node);
      }
    }
    delete side_elem;

    _primary_elem_ids.push_back(elem_id);
    primary_side_points.push_back(side_center);
  }

  // Extract secondary boundary info
  std::vector<dof_id_type> secondary_elem_ids;
  std::vector<Point> secondary_side_points;
  std::vector<dof_id_type> secondary_node_ids;
  std::vector<Point> secondary_node_points;
  for (const auto & elem_id_and_side : secondary_boundary_info)
  {
    auto elem_id = std::get<0>(elem_id_and_side);
    auto side = std::get<1>(elem_id_and_side);
    const Elem * elem = _mesh.elemPtr(elem_id);
    const Elem * side_elem = elem->build_side_ptr(side).release();
    const Point side_center = side_elem->vertex_average();
    for (const auto j : side_elem->node_index_range())
    {
      const Node & node = side_elem->node_ref(j);
      const auto node_id = node.id();
      if (std::find(secondary_node_ids.begin(), secondary_node_ids.end(), node_id) ==
          secondary_node_ids.end())
      {
        secondary_node_ids.push_back(node_id);
        secondary_node_points.push_back(node);
      }
    }
    delete side_elem;

    secondary_elem_ids.push_back(elem_id);
    secondary_side_points.push_back(side_center);
  }

  _all_points_are_coincident = true;

  // Build the element mapping
  if (primary_side_points.size() > 0 && secondary_side_points.size() > 0)
  {
    // find the primary elements that are nearest to the secondary elements
    KDTree kd_tree(primary_side_points, _mesh.getMaxLeafSize());
    for (std::size_t i_secondary = 0; i_secondary < secondary_side_points.size(); i_secondary++)
    {
      unsigned int patch_size = 1;
      std::vector<std::size_t> return_index(patch_size);
      kd_tree.neighborSearch(secondary_side_points[i_secondary], patch_size, return_index);
      const std::size_t i_primary = return_index[0];

      // Flip flag if any pair of points are not coincident
      if (!secondary_side_points[i_secondary].absolute_fuzzy_equals(primary_side_points[i_primary]))
        _all_points_are_coincident = false;

      const auto primary_elem_id = _primary_elem_ids[i_primary];
      const auto secondary_elem_id = secondary_elem_ids[i_secondary];

      _elem_id_map.insert({primary_elem_id, secondary_elem_id});
      _elem_id_map.insert({secondary_elem_id, primary_elem_id});
    }
  }

  // Build the node mapping
  if (primary_node_points.size() > 0 && secondary_node_points.size() > 0)
  {
    // find the primary nodes that are nearest to the secondary nodes
    KDTree kd_tree(primary_node_points, _mesh.getMaxLeafSize());
    for (std::size_t i_secondary = 0; i_secondary < secondary_node_points.size(); i_secondary++)
    {
      unsigned int patch_size = 1;
      std::vector<std::size_t> return_index(patch_size);
      kd_tree.neighborSearch(secondary_node_points[i_secondary], patch_size, return_index);
      const std::size_t i_primary = return_index[0];

      // Flip flag if any pair of points are not coincident
      if (!secondary_node_points[i_secondary].absolute_fuzzy_equals(primary_node_points[i_primary]))
        _all_points_are_coincident = false;

      const auto primary_node_id = primary_node_ids[i_primary];
      const auto secondary_node_id = secondary_node_ids[i_secondary];

      _node_id_map.insert({primary_node_id, secondary_node_id});
      _node_id_map.insert({secondary_node_id, primary_node_id});
    }
  }
}

dof_id_type
MeshAlignment2D2D::getNeighborElemID(const dof_id_type & elem_id) const
{
  auto it = _elem_id_map.find(elem_id);
  if (it != _elem_id_map.end())
    return it->second;
  else
    mooseError("No neighbor element ID was found for element ID ", elem_id, ".");
}

bool
MeshAlignment2D2D::hasNeighborNode(const dof_id_type & node_id) const
{
  return _node_id_map.find(node_id) != _node_id_map.end();
}

dof_id_type
MeshAlignment2D2D::getNeighborNodeID(const dof_id_type & node_id) const
{
  auto it = _node_id_map.find(node_id);
  if (it != _node_id_map.end())
    return it->second;
  else
    mooseError("No neighbor node ID was found for node ID ", node_id, ".");
}
