//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/primary/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshAlignment.h"
#include "Assembly.h"
#include "KDTree.h"

#include "libmesh/elem.h"

MeshAlignment::MeshAlignment(const MooseMesh & mesh)
  : MeshAlignmentBase(mesh), _meshes_are_coincident(false)
{
}

void
MeshAlignment::initialize(
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
MeshAlignment::initialize(
    const std::vector<std::tuple<dof_id_type, unsigned short int>> & primary_boundary_info,
    const std::vector<std::tuple<dof_id_type, unsigned short int>> & secondary_boundary_info)
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
}

void
MeshAlignment::buildMapping()
{
  _meshes_are_coincident = true;
  std::vector<unsigned int> primary_elem_pairing_count(_primary_elem_ids.size(), 0);

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

      // Flip flag if any pair of points are not coincident
      if (!_secondary_elem_points[i_secondary].absolute_fuzzy_equals(
              _primary_elem_points[i_primary]))
        _meshes_are_coincident = false;

      const auto primary_elem_id = _primary_elem_ids[i_primary];
      const auto secondary_elem_id = _secondary_elem_ids[i_secondary];

      _coupled_elem_ids.insert({primary_elem_id, secondary_elem_id});
      _coupled_elem_ids.insert({secondary_elem_id, primary_elem_id});

      primary_elem_pairing_count[i_primary]++;
    }
  }

  // Check if meshes are aligned: all primary boundary elements have exactly one pairing
  _meshes_are_aligned = true;
  for (std::size_t i_primary = 0; i_primary < _primary_elem_ids.size(); i_primary++)
    if (primary_elem_pairing_count[i_primary] != 1)
      _meshes_are_aligned = false;

  // Build the node mapping
  if (_primary_node_points.size() > 0 && _secondary_node_points.size() > 0)
  {
    // find the primary nodes that are nearest to the secondary nodes
    KDTree kd_tree(_primary_node_points, _mesh.getMaxLeafSize());
    for (std::size_t i_secondary = 0; i_secondary < _secondary_node_points.size(); i_secondary++)
    {
      unsigned int patch_size = 1;
      std::vector<std::size_t> return_index(patch_size);
      kd_tree.neighborSearch(_secondary_node_points[i_secondary], patch_size, return_index);
      const std::size_t i_primary = return_index[0];

      // Flip flag if any pair of points are not coincident
      if (!_secondary_node_points[i_secondary].absolute_fuzzy_equals(
              _primary_node_points[i_primary]))
        _meshes_are_coincident = false;

      const auto primary_node_id = _primary_node_ids[i_primary];
      const auto secondary_node_id = _secondary_node_ids[i_secondary];

      _coupled_node_ids.insert({primary_node_id, secondary_node_id});
      _coupled_node_ids.insert({secondary_node_id, primary_node_id});
    }
  }
}

void
MeshAlignment::buildCoupledElemQpIndexMap(Assembly & assembly)
{
  // get local quadrature point maps on each side
  std::map<dof_id_type, std::vector<Point>> primary_qp_map =
      getLocalQuadraturePointMap(assembly, _primary_elem_ids, _primary_side_ids);
  std::map<dof_id_type, std::vector<Point>> secondary_qp_map =
      getLocalQuadraturePointMap(assembly, _secondary_elem_ids, _secondary_side_ids);

  // build mapping
  for (const auto & primary_elem_id : _primary_elem_ids)
  {
    const Elem * primary_elem = _mesh.queryElemPtr(primary_elem_id);
    // The PID check is needed to exclude ghost elements, since those don't
    // necessarily have a coupled element on this processor:
    if (primary_elem && primary_elem->processor_id() == _mesh.processor_id())
    {
      std::vector<Point> & primary_qps = primary_qp_map[primary_elem_id];

      const dof_id_type secondary_elem_id = getCoupledElemID(primary_elem_id);
      std::vector<Point> & secondary_qps = secondary_qp_map[secondary_elem_id];

      mooseAssert(primary_qps.size() == secondary_qps.size(),
                  "The numbers of quadrature points for each element must be the same");

      _coupled_elem_qp_indices[primary_elem_id].resize(primary_qps.size());
      KDTree kd_tree_qp(secondary_qps, _mesh.getMaxLeafSize());
      for (std::size_t i = 0; i < primary_qps.size(); i++)
      {
        unsigned int patch_size = 1;
        std::vector<std::size_t> return_index(patch_size);
        kd_tree_qp.neighborSearch(primary_qps[i], patch_size, return_index);
        _coupled_elem_qp_indices[primary_elem_id][i] = return_index[0];
      }
    }
  }
}

std::map<dof_id_type, std::vector<Point>>
MeshAlignment::getLocalQuadraturePointMap(Assembly & assembly,
                                          const std::vector<dof_id_type> & elem_ids,
                                          const std::vector<unsigned short int> & side_ids) const
{
  std::map<dof_id_type, std::vector<Point>> elem_id_to_qps;
  for (unsigned int i = 0; i < elem_ids.size(); i++)
  {
    const auto elem_id = elem_ids[i];
    const Elem * elem = _mesh.queryElemPtr(elem_id);
    if (elem)
    {
      assembly.setCurrentSubdomainID(elem->subdomain_id());

      MooseArray<Point> q_points;
      if (side_ids.size() > 0)
      {
        assembly.reinit(elem, side_ids[i]);
        q_points = assembly.qPointsFace();
      }
      else
      {
        assembly.reinit(elem);
        q_points = assembly.qPoints();
      }

      for (std::size_t i = 0; i < q_points.size(); i++)
        elem_id_to_qps[elem_id].push_back(q_points[i]);
    }
  }

  return elem_id_to_qps;
}

bool
MeshAlignment::hasCoupledElemID(const dof_id_type & elem_id) const
{
  return _coupled_elem_ids.find(elem_id) != _coupled_elem_ids.end();
}

const dof_id_type &
MeshAlignment::getCoupledElemID(const dof_id_type & elem_id) const
{
  mooseAssert(hasCoupledElemID(elem_id), "The element ID has no coupled element.");
  return _coupled_elem_ids.find(elem_id)->second;
}

bool
MeshAlignment::hasCoupledNodeID(const dof_id_type & node_id) const
{
  return _coupled_node_ids.find(node_id) != _coupled_node_ids.end();
}

const dof_id_type &
MeshAlignment::getCoupledNodeID(const dof_id_type & node_id) const
{
  mooseAssert(hasCoupledNodeID(node_id), "The node ID has no coupled node.");
  return _coupled_node_ids.find(node_id)->second;
}

unsigned int
MeshAlignment::getCoupledElemQpIndex(const dof_id_type & elem_id, const unsigned int & qp) const
{
  auto it = _coupled_elem_qp_indices.find(elem_id);
  mooseAssert(it != _coupled_elem_qp_indices.end(),
              "The element ID has no coupled quadrature point indices.");
  mooseAssert(qp < it->second.size(), "The quadrature index does not exist in the map.");
  return it->second[qp];
}
