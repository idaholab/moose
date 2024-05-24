//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshAlignment2D3D.h"
#include "Assembly.h"
#include "KDTree.h"
#include "MooseUtils.h"

#include "libmesh/elem.h"

MeshAlignment2D3D::MeshAlignment2D3D(const MooseMesh & mesh) : MeshAlignmentOneToMany(mesh) {}

void
MeshAlignment2D3D::initialize(
    const std::vector<std::tuple<dof_id_type, unsigned short int>> & primary_boundary_info,
    const std::vector<std::tuple<dof_id_type, unsigned short int>> & secondary_boundary_info,
    const Point & axis_point,
    const RealVectorValue & axis_direction)
{
  extractFromBoundaryInfo(primary_boundary_info,
                          _primary_elem_ids,
                          _primary_side_ids,
                          _primary_elem_points,
                          _primary_node_ids,
                          _primary_node_points);

  extractFromBoundaryInfo(secondary_boundary_info,
                          _secondary_elem_ids,
                          _secondary_side_ids,
                          _secondary_elem_points,
                          _secondary_node_ids,
                          _secondary_node_points);

  buildMapping();
  checkAlignment(axis_point, axis_direction);
}

void
MeshAlignment2D3D::buildCoupledElemQpIndexMapSecondary(Assembly & assembly)
{
  for (const auto i_secondary : index_range(_secondary_elem_ids))
  {
    const auto secondary_elem_id = _secondary_elem_ids[i_secondary];
    const Elem * secondary_elem = _mesh.queryElemPtr(secondary_elem_id);
    // The PID check is needed to exclude ghost elements, since those don't
    // necessarily have a coupled element on this processor:
    if (secondary_elem && secondary_elem->processor_id() == _mesh.processor_id())
    {
      const auto secondary_side_id = _secondary_side_ids[i_secondary];
      assembly.setCurrentSubdomainID(secondary_elem->subdomain_id());
      assembly.reinit(secondary_elem, secondary_side_id);
      const auto secondary_qps = assembly.qPointsFace().stdVector();
      _n_qp_secondary = secondary_qps.size();

      const auto primary_elem_id = getCoupledPrimaryElemID(secondary_elem_id);
      auto it = std::find(_primary_elem_ids.begin(), _primary_elem_ids.end(), primary_elem_id);
      const auto i_primary = std::distance(_primary_elem_ids.begin(), it);
      const auto primary_side_id = _primary_side_ids[i_primary];

      // This element should be ghosted, so this should be safe
      const Elem * primary_elem = _mesh.elemPtr(primary_elem_id);
      assembly.setCurrentSubdomainID(primary_elem->subdomain_id());
      assembly.reinit(primary_elem, primary_side_id);
      auto primary_qps = assembly.qPointsFace().stdVector();
      _n_qp_primary = primary_qps.size();

      // compute the area for each quadrature point on the primary side
      if (_primary_elem_id_to_area.find(primary_elem_id) == _primary_elem_id_to_area.end())
      {
        const auto & JxW = assembly.JxWFace();
        const auto & coord = assembly.coordTransformation();
        std::vector<Real> area(_n_qp_primary, 0.0);
        for (unsigned int qp = 0; qp < _n_qp_primary; qp++)
          area[qp] = JxW[qp] * coord[qp];
        _primary_elem_id_to_area[primary_elem_id] = area;
      }

      _secondary_elem_id_to_qp_indices[secondary_elem_id].resize(secondary_qps.size());
      KDTree kd_tree_qp(primary_qps, _mesh.getMaxLeafSize());
      for (const auto i : index_range(secondary_qps))
      {
        unsigned int patch_size = 1;
        std::vector<std::size_t> return_index(patch_size);
        kd_tree_qp.neighborSearch(secondary_qps[i], patch_size, return_index);
        _secondary_elem_id_to_qp_indices[secondary_elem_id][i] = return_index[0];
      }
    }
  }
}

const std::vector<Real> &
MeshAlignment2D3D::getPrimaryArea(const dof_id_type primary_elem_id) const
{
  auto it = _primary_elem_id_to_area.find(primary_elem_id);
  mooseAssert(it != _primary_elem_id_to_area.end(), "The element ID has no area stored.");
  return it->second;
}
