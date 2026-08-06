//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackTipNodeLayerSubdomainModifier.h"
#include "SolidMechanicsAppTypes.h"

#include "libmesh/elem.h"

#include <limits>
#include <utility>
#include <vector>

registerMooseObject("XFEMApp", CrackTipNodeLayerSubdomainModifier);

InputParameters
CrackTipNodeLayerSubdomainModifier::validParams()
{
  InputParameters params = ElementSubdomainModifier::validParams();
  params.addClassDescription("Assign an enriched subdomain to elements that are within a fixed "
                             "number of node-connectivity layers from crack-tip elements.");
  params.addRequiredParam<UserObjectName>(
      "crack_front_definition",
      "Name of the CrackFrontDefinition user object that provides crack-front points.");
  params.addRequiredParam<SubdomainID>("enriched_subdomain_id",
                                       "Subdomain ID used for crack-tip enriched elements.");
  params.addParam<SubdomainID>("base_subdomain_id",
                               Moose::INVALID_BLOCK_ID,
                               "Subdomain ID to restore for elements no longer in the enriched "
                               "region. If omitted, elements outside the enriched region are left "
                               "unchanged.");
  params.addRequiredParam<unsigned int>(
      "num_node_layers", "Number of node-connectivity layers grown from crack-tip seed elements.");
  params.set<bool>("skip_restore_subdomain_changes") = true;

  ExecFlagEnum & exec = params.set<ExecFlagEnum>("execute_on");
  exec.addAvailableFlags(EXEC_XFEM_SUBDOMAIN_MODIFIER);
  params.set<ExecFlagEnum>("execute_on") = {EXEC_XFEM_SUBDOMAIN_MODIFIER};
  params.set<int>("execution_order_group") = 0;

  return params;
}

CrackTipNodeLayerSubdomainModifier::CrackTipNodeLayerSubdomainModifier(
    const InputParameters & parameters)
  : ElementSubdomainModifier(parameters),
    _enriched_subdomain_id(getParam<SubdomainID>("enriched_subdomain_id")),
    _base_subdomain_id(getParam<SubdomainID>("base_subdomain_id")),
    _num_node_layers(getParam<unsigned int>("num_node_layers")),
    _crack_front_definition(getUserObject<CrackFrontDefinition>("crack_front_definition"))
{
}

void
CrackTipNodeLayerSubdomainModifier::initialize()
{
  ElementSubdomainModifier::initialize();
  buildEnrichedElementSet();
}

void
CrackTipNodeLayerSubdomainModifier::buildEnrichedElementSet()
{
  _enriched_elem_ids.clear();

  const std::size_t num_crack_points = _crack_front_definition.getNumCrackFrontPoints();
  if (!num_crack_points)
    return;

  std::vector<const Point *> crack_points;
  crack_points.reserve(num_crack_points);
  for (const auto i : make_range(num_crack_points))
    if (const Point * point = _crack_front_definition.getCrackFrontPoint(i))
      crack_points.push_back(point);

  if (crack_points.empty())
    return;

  std::unordered_set<dof_id_type> frontier;

  for (const auto * crack_point : crack_points)
  {
    dof_id_type local_nearest_id = DofObject::invalid_id;
    Real local_min_sq = std::numeric_limits<Real>::max();

    for (const auto * elem : *_mesh.getActiveLocalElementRange())
    {
      const Real distance_sq = (elem->vertex_average() - *crack_point).norm_sq();
      if (distance_sq < local_min_sq)
      {
        local_min_sq = distance_sq;
        local_nearest_id = elem->id();
      }
    }

    // Global reduction: determine the minimum distance across all MPI ranks.
    // Only the rank(s) holding the globally nearest element seed the BFS.
    Real global_min_sq = local_min_sq;
    _communicator.min(global_min_sq);

    if (local_nearest_id != DofObject::invalid_id &&
        local_min_sq <= global_min_sq * (1.0 + 1e-10) + 1e-30)
    {
      _enriched_elem_ids.insert(local_nearest_id);
      frontier.insert(local_nearest_id);
    }
  }

  for (const auto layer : make_range(_num_node_layers))
  {
    libmesh_ignore(layer);

    if (frontier.empty())
      break;

    std::unordered_set<dof_id_type> next_frontier;

    for (const auto elem_id : frontier)
    {
      const Elem * elem = _mesh.elemPtr(elem_id);
      if (!elem)
        continue;

      for (const auto n : make_range(elem->n_nodes()))
      {
        const auto node_id = elem->node_id(n);
        const auto node_to_elem_it = _mesh.nodeToElemMap().find(node_id);
        if (node_to_elem_it == _mesh.nodeToElemMap().end())
          continue;

        for (const auto neigh_elem_id : node_to_elem_it->second)
        {
          const Elem * neigh_elem = _mesh.elemPtr(neigh_elem_id);
          if (!neigh_elem || !neigh_elem->active())
            continue;

          if (_enriched_elem_ids.insert(neigh_elem_id).second)
            next_frontier.insert(neigh_elem_id);
        }
      }
    }

    frontier = std::move(next_frontier);
  }

  // Synchronize the enriched element set across all MPI ranks.
  // Each rank may have reached different elements through its local + ghost element graph.
  // Allgathering ensures every rank has the complete set so that computeSubdomainID()
  // gives consistent results regardless of how the mesh is partitioned.
  std::vector<dof_id_type> all_ids(_enriched_elem_ids.begin(), _enriched_elem_ids.end());
  _communicator.allgather(all_ids);
  _enriched_elem_ids.insert(all_ids.begin(), all_ids.end());
}

SubdomainID
CrackTipNodeLayerSubdomainModifier::computeSubdomainID()
{
  const dof_id_type elem_id = _current_elem->id();
  const SubdomainID current_id = _current_elem->subdomain_id();

  if (_enriched_elem_ids.count(elem_id))
  {
    if (current_id != _enriched_subdomain_id)
    {
      _original_subdomain_ids.try_emplace(elem_id, current_id);
      return _enriched_subdomain_id;
    }

    return Moose::INVALID_BLOCK_ID;
  }

  if (current_id == _enriched_subdomain_id)
  {
    const auto original_it = _original_subdomain_ids.find(elem_id);
    if (original_it != _original_subdomain_ids.end())
    {
      const SubdomainID original_id = original_it->second;
      _original_subdomain_ids.erase(original_it);
      if (current_id != original_id)
        return original_id;
    }

    if (_base_subdomain_id != Moose::INVALID_BLOCK_ID && current_id != _base_subdomain_id)
      return _base_subdomain_id;
  }

  return Moose::INVALID_BLOCK_ID;
}
