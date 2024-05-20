//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshAlignmentOneToMany.h"
#include "KDTree.h"
#include "MooseUtils.h"

MeshAlignmentOneToMany::MeshAlignmentOneToMany(const MooseMesh & mesh)
  : MeshAlignmentBase(mesh), _max_coupling_size(0)
{
}

void
MeshAlignmentOneToMany::buildMapping()
{
  // Build the element mapping
  if (_primary_elem_points.size() > 0 && _secondary_elem_points.size() > 0)
  {
    // find the primary elements that are nearest to the secondary elements
    KDTree kd_tree(_primary_elem_points, _mesh.getMaxLeafSize());
    for (std::size_t i_secondary = 0; i_secondary < _secondary_elem_points.size(); i_secondary++)
    {
      unsigned int patch_size = 1;
      std::vector<std::size_t> return_index(patch_size);
      kd_tree.neighborSearch(_secondary_elem_points[i_secondary], patch_size, return_index);
      const std::size_t i_primary = return_index[0];

      const auto primary_elem_id = _primary_elem_ids[i_primary];
      const auto secondary_elem_id = _secondary_elem_ids[i_secondary];

      _secondary_elem_id_to_primary_elem_id[secondary_elem_id] = primary_elem_id;

      auto it = _primary_elem_id_to_secondary_elem_ids.find(primary_elem_id);
      if (it == _primary_elem_id_to_secondary_elem_ids.end())
        _primary_elem_id_to_secondary_elem_ids.insert({primary_elem_id, {secondary_elem_id}});
      else
        it->second.push_back(secondary_elem_id);
    }

    // Determine max coupling size
    for (const auto primary_elem_id : _primary_elem_ids)
      if (hasCoupledSecondaryElemIDs(primary_elem_id))
      {
        const auto & secondary_elem_ids = getCoupledSecondaryElemIDs(primary_elem_id);
        _max_coupling_size = std::max(secondary_elem_ids.size(), _max_coupling_size);
      }
  }
}

void
MeshAlignmentOneToMany::checkAlignment(const Point & axis_point,
                                       const RealVectorValue & axis_direction)
{
  if (_primary_elem_points.size() > 0 && _secondary_elem_points.size() > 0)
  {
    // Check if meshes are aligned: all secondary boundary faces paired to a
    // primary element/face must have the same axial coordinate.
    _meshes_are_aligned = true;
    for (const auto i_primary : index_range(_primary_elem_ids))
    {
      const auto primary_elem_id = _primary_elem_ids[i_primary];
      const auto primary_elem_point = _primary_elem_points[i_primary];
      const auto primary_ax_coord = axis_direction * (primary_elem_point - axis_point);

      if (hasCoupledSecondaryElemIDs(primary_elem_id))
      {
        const auto & secondary_elem_ids = getCoupledSecondaryElemIDs(primary_elem_id);
        for (const auto secondary_elem_id : secondary_elem_ids)
        {
          auto it =
              std::find(_secondary_elem_ids.begin(), _secondary_elem_ids.end(), secondary_elem_id);
          const auto i_secondary = std::distance(_secondary_elem_ids.begin(), it);
          const auto secondary_elem_point = _secondary_elem_points[i_secondary];
          const auto secondary_ax_coord = axis_direction * (secondary_elem_point - axis_point);
          if (!MooseUtils::absoluteFuzzyEqual(secondary_ax_coord, primary_ax_coord))
            _meshes_are_aligned = false;
        }
      }
      else
        _meshes_are_aligned = false;
    }
  }
}

bool
MeshAlignmentOneToMany::hasCoupledPrimaryElemID(const dof_id_type & secondary_elem_id) const
{
  return _secondary_elem_id_to_primary_elem_id.find(secondary_elem_id) !=
         _secondary_elem_id_to_primary_elem_id.end();
}

dof_id_type
MeshAlignmentOneToMany::getCoupledPrimaryElemID(const dof_id_type & secondary_elem_id) const
{
  mooseAssert(hasCoupledPrimaryElemID(secondary_elem_id),
              "The element ID has no coupled elements.");
  return _secondary_elem_id_to_primary_elem_id.find(secondary_elem_id)->second;
}

bool
MeshAlignmentOneToMany::hasCoupledSecondaryElemIDs(const dof_id_type & primary_elem_id) const
{
  return _primary_elem_id_to_secondary_elem_ids.find(primary_elem_id) !=
         _primary_elem_id_to_secondary_elem_ids.end();
}

const std::vector<dof_id_type> &
MeshAlignmentOneToMany::getCoupledSecondaryElemIDs(const dof_id_type & primary_elem_id) const
{
  mooseAssert(hasCoupledSecondaryElemIDs(primary_elem_id),
              "The element ID has no coupled elements.");
  return _primary_elem_id_to_secondary_elem_ids.find(primary_elem_id)->second;
}

unsigned int
MeshAlignmentOneToMany::getCoupledPrimaryElemQpIndex(const dof_id_type & secondary_elem_id,
                                                     const unsigned int & secondary_qp) const
{
  auto it = _secondary_elem_id_to_qp_indices.find(secondary_elem_id);
  mooseAssert(it != _secondary_elem_id_to_qp_indices.end(),
              "The element ID has no coupled quadrature point indices.");
  mooseAssert(secondary_qp < it->second.size(), "The quadrature index does not exist in the map.");
  return it->second[secondary_qp];
}
