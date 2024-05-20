//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshAlignment1D3D.h"
#include "Assembly.h"
#include "KDTree.h"

#include "libmesh/elem.h"

MeshAlignment1D3D::MeshAlignment1D3D(const MooseMesh & mesh) : MeshAlignmentOneToMany(mesh) {}

void
MeshAlignment1D3D::initialize(
    const std::vector<dof_id_type> & primary_elem_ids,
    const std::vector<std::tuple<dof_id_type, unsigned short int>> & secondary_boundary_info)
{
  _primary_elem_ids = primary_elem_ids;
  extractFrom1DElements(
      _primary_elem_ids, _primary_elem_points, _primary_node_ids, _primary_node_points);

  extractFromBoundaryInfo(secondary_boundary_info,
                          _secondary_elem_ids,
                          _secondary_side_ids,
                          _secondary_elem_points,
                          _secondary_node_ids,
                          _secondary_node_points);

  buildMapping();
}

void
MeshAlignment1D3D::buildCoupledElemQpIndexMap(Assembly & assembly)
{
  for (const auto & primary_elem_id : _primary_elem_ids)
  {
    const Elem * primary_elem = _mesh.queryElemPtr(primary_elem_id);
    // The PID check is needed to exclude ghost elements, since those don't
    // necessarily have a coupled element on this processor:
    if (primary_elem && primary_elem->processor_id() == _mesh.processor_id())
    {
      assembly.setCurrentSubdomainID(primary_elem->subdomain_id());
      assembly.reinit(primary_elem);
      std::vector<Point> primary_qps = assembly.qPoints().stdVector();
      _n_qp_primary = primary_qps.size();

      const auto & secondary_elem_ids = getCoupledSecondaryElemIDs(primary_elem_id);
      for (const auto & secondary_elem_id : secondary_elem_ids)
      {
        auto it =
            std::find(_secondary_elem_ids.begin(), _secondary_elem_ids.end(), secondary_elem_id);
        mooseAssert(it != _secondary_elem_ids.end(), "Secondary element ID not found.");
        const unsigned int i_secondary = std::distance(_secondary_elem_ids.begin(), it);
        const unsigned short int secondary_side_id = _secondary_side_ids[i_secondary];

        const Elem * secondary_elem = _mesh.elemPtr(secondary_elem_id);
        assembly.setCurrentSubdomainID(secondary_elem->subdomain_id());
        assembly.reinit(secondary_elem, secondary_side_id);
        const std::vector<Point> secondary_qps = assembly.qPointsFace().stdVector();
        _n_qp_secondary = secondary_qps.size();

        _secondary_elem_id_to_qp_indices[secondary_elem_id].resize(secondary_qps.size());
        KDTree kd_tree_qp(primary_qps, _mesh.getMaxLeafSize());
        for (std::size_t i = 0; i < secondary_qps.size(); i++)
        {
          unsigned int patch_size = 1;
          std::vector<std::size_t> return_index(patch_size);
          kd_tree_qp.neighborSearch(secondary_qps[i], patch_size, return_index);
          _secondary_elem_id_to_qp_indices[secondary_elem_id][i] = return_index[0];
        }
      }
    }
  }
}
