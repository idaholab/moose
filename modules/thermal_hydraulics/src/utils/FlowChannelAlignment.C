//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowChannelAlignment.h"
#include "KDTree.h"
#include "libmesh/elem.h"
#include "libmesh/fe_type.h"
#include "libmesh/fe_interface.h"

FlowChannelAlignment::FlowChannelAlignment(const THMMesh & mesh) : _mesh(mesh) {}

void
FlowChannelAlignment::build(
    const std::vector<std::tuple<dof_id_type, unsigned short int>> & master_boundary_info,
    const std::vector<dof_id_type> & slave_elem_ids)
{
  _master_boundary_info = master_boundary_info;
  _slave_elem_ids = slave_elem_ids;

  // element IDs corresponding to the centroids in `_master_points`
  std::vector<dof_id_type> master_elem_ids;
  // local side number corresponding to the element ID in `master_elem_ids`
  std::vector<dof_id_type> master_elem_sides;

  for (const auto & t : _master_boundary_info)
  {
    auto elem_id = std::get<0>(t);
    auto side_id = std::get<1>(t);
    const Elem * elem = _mesh.elemPtr(elem_id);

    master_elem_ids.push_back(elem_id);
    master_elem_sides.push_back(side_id);
    _master_points.push_back(elem->vertex_average());
    _nearest_elem_side.insert(std::pair<dof_id_type, unsigned int>(elem_id, side_id));
  }

  for (auto & elem_id : _slave_elem_ids)
  {
    const Elem * elem = _mesh.elemPtr(elem_id);
    _slave_points.push_back(elem->vertex_average());
  }

  if (_master_points.size() > 0 && _slave_points.size() > 0)
  {
    // find the master elements that are nearest to the slave elements
    KDTree kd_tree(_master_points, _mesh.getMaxLeafSize());
    for (std::size_t i = 0; i < _slave_points.size(); i++)
    {
      unsigned int patch_size = 1;
      std::vector<std::size_t> return_index(patch_size);
      kd_tree.neighborSearch(_slave_points[i], patch_size, return_index);

      _nearest_elem_ids.insert(
          std::pair<dof_id_type, dof_id_type>(slave_elem_ids[i], master_elem_ids[return_index[0]]));
      _nearest_elem_ids.insert(
          std::pair<dof_id_type, dof_id_type>(master_elem_ids[return_index[0]], slave_elem_ids[i]));
    }
  }
}

bool
FlowChannelAlignment::check(const std::vector<dof_id_type> & fch_elem_ids) const
{
  if (_master_points.size() > 0 && _slave_points.size() > 0)
  {
    // Go over all elements in the flow channel. For the first element in the
    // loop compute the translation vector between its centroid and its neighbor
    // heat structure centroid. Then for the remaining elements, check that the
    // neighbors all have the same translation vector. If this is true, then it
    // can be inferred that all flow channel elements have a unique neighbor.
    // Note that for very coarse discretizations, it is possible for the heat
    // structure to be offset in the axial direction from the flow channel.
    bool set_translation_vec = false;
    RealVectorValue translation_vec;
    for (const auto & elem_id : fch_elem_ids)
    {
      const Elem * elem = _mesh.elemPtr(elem_id);
      Point center_pt = elem->vertex_average();

      const dof_id_type & hs_elem_id = _nearest_elem_ids.at(elem_id);
      const unsigned int & hs_elem_side = _nearest_elem_side.at(hs_elem_id);
      const Elem * neighbor = _mesh.elemPtr(hs_elem_id);
      const Elem * neighbor_side_elem = neighbor->build_side_ptr(hs_elem_side).release();
      const Point hs_pt = neighbor_side_elem->vertex_average();
      delete neighbor_side_elem;

      if (!set_translation_vec)
      {
        translation_vec = hs_pt - center_pt;
        set_translation_vec = true;
      }

      if (!(center_pt + translation_vec).absolute_fuzzy_equals(hs_pt))
        return false;
    }
  }

  return true;
}

const dof_id_type &
FlowChannelAlignment::getNearestElemID(const dof_id_type & elem_id) const
{
  auto it = _nearest_elem_ids.find(elem_id);
  if (it != _nearest_elem_ids.end())
    return it->second;
  else
    return DofObject::invalid_id;
}
