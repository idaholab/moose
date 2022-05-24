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
#include "libmesh/elem.h"

/*
 * A mesh generator to split a mesh by breaking all element-element interfaces in the
 * specified subdomains
 */
class ExplodeMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  ExplodeMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  // It is important to maintain the exact same ordering across processors. Otherwise nodes with the
  // same ID might have different processor IDs. std::map and std::set of dof_id_type are used here
  // to ensure the same ordering.
  typedef std::map<const dof_id_type, std::set<dof_id_type>> NodeToElemMapType;

  NodeToElemMapType
  buildSubdomainRestrictedNodeToElemMap(std::unique_ptr<MeshBase> & mesh,
                                        const std::vector<SubdomainID> & subdomains) const;

  void duplicateNodes(std::unique_ptr<MeshBase> & mesh,
                      const NodeToElemMapType & node_to_elem_map) const;

  void duplicateNode(std::unique_ptr<MeshBase> & mesh, Elem * elem, const Node * node) const;

  void createInterface(MeshBase & mesh, const NodeToElemMapType & node_to_elem_map) const;

  /// The mesh to modify
  std::unique_ptr<MeshBase> & _input;

  // The subdomains to explode
  const std::vector<SubdomainID> & _subdomains;

  // The name of the new boundary
  const BoundaryName _interface_name;
};
