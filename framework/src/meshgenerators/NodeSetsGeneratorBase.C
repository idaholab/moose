//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodeSetsGeneratorBase.h"
#include "Parser.h"
#include "InputParameters.h"
#include "MooseMeshUtils.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/elem.h"

InputParameters
NodeSetsGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::vector<BoundaryName>>("new_nodeset",
                                                     "The list of nodeset names to create.");

  params.addParam<bool>("replace",
                        false,
                        "If true, replace the old nodesets. If false, the current nodesets (if "
                        "any) will be preserved.");

  params.addParam<std::vector<BoundaryName>>(
      "included_nodesets",
      "A set of nodeset names or ids whose nodes will be included in the new nodesets.  A node "
      "is only added if it also belongs to one of these nodesets.");
  params.addParam<std::vector<BoundaryName>>(
      "excluded_nodesets",
      "A set of nodeset names or ids whose nodes will be excluded from the new nodesets.  A node "
      "is only added if does not belong to any of these nodesets.");
  params.addParam<std::vector<SubdomainName>>(
      "included_subdomains",
      "A set of subdomain names or ids whose nodes will be included in the new nodesets. A node "
      "is only added if the subdomain id of the corresponding element is in this set.");

  params.addParam<bool>(
      "include_only_external_nodes",
      false,
      "Whether to only include external nodes when considering nodes to add to the nodeset");

  // Nodeset restriction param group
  params.addParamNamesToGroup("included_nodesets excluded_nodesets included_subdomains "
                              "include_only_external_nodes",
                              "Nodeset restrictions");

  return params;
}

NodeSetsGeneratorBase::NodeSetsGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _nodeset_names(std::vector<BoundaryName>()),
    _replace(getParam<bool>("replace")),
    _check_included_nodesets(isParamValid("included_nodesets")),
    _check_excluded_nodesets(isParamValid("excluded_nodesets")),
    _check_included_subdomains(isParamValid("included_subdomains")),
    _included_nodeset_ids(std::vector<boundary_id_type>()),
    _excluded_nodeset_ids(std::vector<boundary_id_type>()),
    _included_subdomain_ids(std::vector<subdomain_id_type>()),
    _include_only_external_nodes(getParam<bool>("include_only_external_nodes"))
{
  if (isParamValid("new_nodeset"))
    _nodeset_names = getParam<std::vector<BoundaryName>>("new_nodeset");
}

void
NodeSetsGeneratorBase::setup(MeshBase & mesh)
{
  // Parameter checks and filling vector of ids (used instead of names for efficiency)
  if (_check_included_nodesets)
  {
    const auto & included_nodesets = getParam<std::vector<BoundaryName>>("included_nodesets");
    for (const auto & nodeset_name : _nodeset_names)
      if (std::find(included_nodesets.begin(), included_nodesets.end(), nodeset_name) !=
          included_nodesets.end())
        paramError(
            "new_nodeset",
            "A nodeset cannot be both the new nodeset and be included in the list of included "
            "nodesets. If you are trying to restrict an existing nodeset, you must use a "
            "different name for 'new_nodeset', delete the old nodeset, and then rename the "
            "new nodeset to the old nodeset.");

    _included_nodeset_ids = MooseMeshUtils::getBoundaryIDs(mesh, included_nodesets, false);

    // Check that the included nodeset ids/names exist in the mesh
    for (const auto i : index_range(_included_nodeset_ids))
      if (_included_nodeset_ids[i] == Moose::INVALID_BOUNDARY_ID)
        paramError("included_nodesets",
                   "The nodeset '",
                   included_nodesets[i],
                   "' was not found within the mesh");
  }

  if (_check_excluded_nodesets)
  {
    const auto & excluded_nodesets = getParam<std::vector<BoundaryName>>("excluded_nodesets");
    for (const auto & nodeset_name : _nodeset_names)
      if (std::find(excluded_nodesets.begin(), excluded_nodesets.end(), nodeset_name) !=
          excluded_nodesets.end())
        paramError(
            "new_nodeset",
            "A nodeset cannot be both the new nodeset and be excluded in the list of excluded "
            "nodesets.");
    _excluded_nodeset_ids = MooseMeshUtils::getBoundaryIDs(mesh, excluded_nodesets, false);

    // Check that the excluded nodeset ids/names exist in the mesh
    for (const auto i : index_range(_excluded_nodeset_ids))
      if (_excluded_nodeset_ids[i] == Moose::INVALID_BOUNDARY_ID)
        paramError("excluded_nodesets",
                   "The nodeset '",
                   excluded_nodesets[i],
                   "' was not found within the mesh");

    if (_check_included_nodesets)
    {
      // Check that included and excluded nodeset lists do not overlap
      for (const auto & nodeset_id : _included_nodeset_ids)
        if (std::find(_excluded_nodeset_ids.begin(), _excluded_nodeset_ids.end(), nodeset_id) !=
            _excluded_nodeset_ids.end())
          paramError("excluded_nodesets",
                     "'included_nodesets' and 'excluded_nodesets' lists should not overlap");
    }
  }

  // Get the subdomain ids from the names
  if (_check_included_subdomains)
  {
    // check that the subdomains exist in the mesh
    const auto subdomains = getParam<std::vector<SubdomainName>>("included_subdomains");
    for (const auto & name : subdomains)
      if (!MooseMeshUtils::hasSubdomainName(mesh, name))
        paramError("included_subdomains", "The block '", name, "' was not found in the mesh");

    _included_subdomain_ids = MooseMeshUtils::getSubdomainIDs(mesh, subdomains);
  }

  // Build the node to element map, which is usually provided by a MooseMesh but in the mesh
  // generation process we are working with a MeshBase
  for (const auto & elem : mesh.active_element_ptr_range())
    for (unsigned int n = 0; n < elem->n_nodes(); n++)
      _node_to_elem_map[elem->node_id(n)].push_back(elem->id());
}

bool
NodeSetsGeneratorBase::nodeOnMeshExteriorBoundary(const Node * node,
                                                  const std::vector<dof_id_type> & node_elems,
                                                  const MeshBase & mesh) const
{
  // Loop on the elements and check whether the node is part of a side with no neighbor (exterior)
  for (const auto elem_id : node_elems)
  {
    const auto elem = mesh.elem_ptr(elem_id);
    for (const auto side_i : make_range(elem->n_sides()))
    {
      // Node is part of the side
      if (elem->side_ptr(side_i)->get_node_index(node) != libMesh::invalid_uint)
      {
        // No neighbor on that side
        if (!elem->neighbor_ptr(side_i))
          return true;
      }
    }
  }
  return false;
}

bool
NodeSetsGeneratorBase::nodeElementsInIncludedSubdomains(const std::vector<dof_id_type> node_elems,
                                                        const MeshBase & mesh) const
{
  for (const auto elem_id : node_elems)
  {
    subdomain_id_type curr_subdomain = mesh.elem_ptr(elem_id)->subdomain_id();
    if (std ::find(_included_subdomain_ids.begin(),
                   _included_subdomain_ids.end(),
                   curr_subdomain) != _included_subdomain_ids.end())
      return true;
  }
  return false;
}

bool
NodeSetsGeneratorBase::nodeInIncludedNodesets(const std::vector<BoundaryID> & node_nodesets) const
{
  for (const auto bid : node_nodesets)
    if (std::find(_included_nodeset_ids.begin(), _included_nodeset_ids.end(), bid) !=
        _included_nodeset_ids.end())
      return true;
  return false;
}

bool
NodeSetsGeneratorBase::nodeInExcludedNodesets(const std::vector<BoundaryID> & node_nodesets) const
{
  for (const auto bid : node_nodesets)
    if (std::find(_excluded_nodeset_ids.begin(), _excluded_nodeset_ids.end(), bid) !=
        _excluded_nodeset_ids.end())
      return true;
  return false;
}

bool
NodeSetsGeneratorBase::nodeSatisfiesRequirements(const Node * node,
                                                 const std::vector<BoundaryID> & node_nodesets,
                                                 const std::vector<dof_id_type> & node_elems,
                                                 const MeshBase & mesh) const
{
  // Skip if side has neighbor and we only want external nodes
  if (_include_only_external_nodes && !nodeOnMeshExteriorBoundary(node, node_elems, mesh))
    return false;

  // Skip if none of the elements owning the node are in the list of accepted subdomains
  if (_check_included_subdomains && !nodeElementsInIncludedSubdomains(node_elems, mesh))
    return false;

  // Skip if side is not part of included nodesets
  if (_check_included_nodesets && !nodeInIncludedNodesets(node_nodesets))
    return false;

  // Skip if side is part of excluded nodesets
  if (_check_excluded_nodesets && nodeInExcludedNodesets(node_nodesets))
    return false;

  return true;
}
