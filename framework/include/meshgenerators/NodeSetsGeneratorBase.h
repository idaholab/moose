//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

/*
 * Base class for mesh generators that add nodesets to the mesh
 */
class NodeSetsGeneratorBase : public MeshGenerator
{
public:
  static InputParameters validParams();

  NodeSetsGeneratorBase(const InputParameters & parameters);

protected:
  /**
   * This method prepares a few attributes which are commonly needed for nodeset generation such
   * as a map from nodes to elements, and checks the existence and validity of several user
   * parameters
   */
  void setup(MeshBase & mesh);

  /**
   * Determines whether the node is on the exterior of the mesh
   * @param node node of interest
   * @param node_elems vector of the ids of all the elements sharing this node
   * @param mesh mesh which contains the node
   */
  bool nodeOnMeshExteriorBoundary(const Node * node,
                                  const std::vector<dof_id_type> & node_elems,
                                  const MeshBase & mesh) const;

  /**
   * Determines whether any neighbor element of the node has a subdomain id in the given
   * subdomain_id_list.
   * @param node_elems vector of the ids of all the elements sharing this node
   * @param mesh mesh which contains the node (and the elements)
   */
  bool nodeElementsInIncludedSubdomains(const std::vector<dof_id_type> node_elems,
                                        const MeshBase & mesh) const;

  /**
   * Determines whether the given node belongs to any nodesets in the
   * included_nodesets parameter.
   * @param node_nodesets vector of the ids of all the nodesets the node is part of
   */
  bool nodeInIncludedNodesets(const std::vector<BoundaryID> & node_nodesets) const;

  /**
   * Determines whether the given node of an element belongs to any nodesets in the
   * excluded_nodesets parameter.
   * @param node_nodesets vector of the ids of all the nodesets the node is part of
   */
  bool nodeInExcludedNodesets(const std::vector<BoundaryID> & node_nodesets) const;

  /**
   * Determines whether the given node satisfies the user-specified constraints
   * @param node node of interest
   * @param node_nodesets vector of the ids of all the nodesets the node is part of
   * @param node_elems vector of the ids of all the elements sharing this node
   * @param mesh mesh which contains the node
   */
  bool nodeSatisfiesRequirements(const Node * node,
                                 const std::vector<BoundaryID> & node_nodesets,
                                 const std::vector<dof_id_type> & node_elems,
                                 const MeshBase & mesh) const;

  /// the mesh to add the nodesets to
  std::unique_ptr<MeshBase> & _input;

  /// The list of new nodeset names
  std::vector<BoundaryName> _nodeset_names;

  /// Whether or not to remove the old nodesets (all of them, if any) when adding nodesets
  const bool _replace;

  /// whether to check nodeset ids against the included nodeset list when adding nodes or not
  const bool _check_included_nodesets;

  /// whether to check nodeset ids against the excluded nodeset list when adding nodes or not
  const bool _check_excluded_nodesets;

  /// whether to check subdomain ids of the element that included this node
  const bool _check_included_subdomains;

  /// A list of nodeset ids that the node has to be part of, extracted from the included_nodesets parameter
  std::vector<boundary_id_type> _included_nodeset_ids;

  /// A list of nodeset ids that the node must not be a part of, extracted from the excluded_nodesets parameter
  std::vector<boundary_id_type> _excluded_nodeset_ids;

  /// A list of included subdomain ids that the node has to be part of, extracted from the included_subdomains parameter
  std::vector<subdomain_id_type> _included_subdomain_ids;

  /// Whether to only include external node when considering nodes to add to the nodeset
  const bool _include_only_external_nodes;

  /// A map from nodes (ids) to  local elements (ids) which comprise the node
  // Build the node to element map, which is usually provided by a MooseMesh but here we have a
  // MeshBase.
  std::unordered_map<dof_id_type, std::vector<dof_id_type>> _node_to_elem_map;
};
