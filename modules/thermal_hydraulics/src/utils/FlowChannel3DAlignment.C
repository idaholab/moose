//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowChannel3DAlignment.h"
#include "KDTree.h"
#include "libmesh/elem.h"
#include "libmesh/fe_type.h"
#include "libmesh/fe_interface.h"

FlowChannel3DAlignment::FlowChannel3DAlignment(const THMMesh & mesh) : _mesh(mesh) {}

void
FlowChannel3DAlignment::build(
    const std::vector<std::tuple<dof_id_type, unsigned short int>> & hs_boundary_info,
    const std::vector<dof_id_type> & fch_elem_ids)
{
  _hs_boundary_info = hs_boundary_info;
  _fch_elem_ids = fch_elem_ids;

  for (const auto & t : _hs_boundary_info)
  {
    auto elem_id = std::get<0>(t);
    const Elem * elem = _mesh.elemPtr(elem_id);
    _hs_elem_ids.push_back(elem_id);
    _hs_points.push_back(elem->vertex_average());
  }

  for (auto & elem_id : _fch_elem_ids)
  {
    const Elem * elem = _mesh.elemPtr(elem_id);
    _fch_points.push_back(elem->vertex_average());
  }

  if (_hs_points.size() > 0 && _fch_points.size() > 0)
  {
    // find the hs elements that are nearest to the fch elements
    KDTree kd_tree(_fch_points, _mesh.getMaxLeafSize());
    for (std::size_t i = 0; i < _hs_points.size(); i++)
    {
      unsigned int patch_size = 1;
      std::vector<std::size_t> return_index(patch_size);
      kd_tree.neighborSearch(_hs_points[i], patch_size, return_index);

      _nearest_elem_ids.insert(
          std::pair<dof_id_type, dof_id_type>(fch_elem_ids[return_index[0]], _hs_elem_ids[i]));
      _nearest_elem_ids.insert(
          std::pair<dof_id_type, dof_id_type>(_hs_elem_ids[i], fch_elem_ids[return_index[0]]));
    }
  }
}

const dof_id_type &
FlowChannel3DAlignment::getNearestElemID(const dof_id_type & elem_id) const
{
  auto it = _nearest_elem_ids.find(elem_id);
  if (it != _nearest_elem_ids.end())
    return it->second;
  else
    return DofObject::invalid_id;
}
