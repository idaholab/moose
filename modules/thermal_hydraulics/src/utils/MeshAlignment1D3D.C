//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/primary/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshAlignment1D3D.h"
#include "Assembly.h"
#include "KDTree.h"

#include "libmesh/elem.h"

MeshAlignment1D3D::MeshAlignment1D3D(const MooseMesh & mesh) : MeshAlignmentBase(mesh) {}

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
MeshAlignment1D3D::buildMapping()
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

      auto it = _primary_elem_id_to_secondary_elem_ids.find(primary_elem_id);
      if (it == _primary_elem_id_to_secondary_elem_ids.end())
        _primary_elem_id_to_secondary_elem_ids.insert({primary_elem_id, {secondary_elem_id}});
      else
        it->second.push_back(secondary_elem_id);
    }
  }
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

bool
MeshAlignment1D3D::hasCoupledSecondaryElemIDs(const dof_id_type & primary_elem_id) const
{
  return _primary_elem_id_to_secondary_elem_ids.find(primary_elem_id) !=
         _primary_elem_id_to_secondary_elem_ids.end();
}

const std::vector<dof_id_type> &
MeshAlignment1D3D::getCoupledSecondaryElemIDs(const dof_id_type & primary_elem_id) const
{
  mooseAssert(hasCoupledSecondaryElemIDs(primary_elem_id),
              "The element ID has no coupled elements.");
  return _primary_elem_id_to_secondary_elem_ids.find(primary_elem_id)->second;
}

unsigned int
MeshAlignment1D3D::getSecondaryNumberOfQuadraturePoints(const dof_id_type & secondary_elem_id) const
{
  auto it = _secondary_elem_id_to_qp_indices.find(secondary_elem_id);
  mooseAssert(it != _secondary_elem_id_to_qp_indices.end(),
              "The element ID has no coupled quadrature point indices.");
  return it->second.size();
}

unsigned int
MeshAlignment1D3D::getCoupledPrimaryElemQpIndex(const dof_id_type & secondary_elem_id,
                                                const unsigned int & secondary_qp) const
{
  auto it = _secondary_elem_id_to_qp_indices.find(secondary_elem_id);
  mooseAssert(it != _secondary_elem_id_to_qp_indices.end(),
              "The element ID has no coupled quadrature point indices.");
  mooseAssert(secondary_qp < it->second.size(), "The quadrature index does not exist in the map.");
  return it->second[secondary_qp];
}
