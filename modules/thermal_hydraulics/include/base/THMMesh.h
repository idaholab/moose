//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"
#include "libmesh/enum_elem_type.h"

/**
 * Mesh for THM
 */
class THMMesh : public MooseMesh
{
public:
  THMMesh(const InputParameters & parameters);
  THMMesh(const THMMesh & other_mesh);

  virtual unsigned int dimension() const override;
  virtual unsigned int effectiveSpatialDimension() const override;
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void buildMesh() override;

  /// Add a new node into the mesh
  Node * addNode(const Point & pt);

  /// Add a new element into the mesh
  Elem * addElement(libMesh::ElemType elem_type, const std::vector<dof_id_type> & node_ids);
  Elem * addNodeElement(dof_id_type node);
  Elem * addElementEdge2(dof_id_type node0, dof_id_type node1);
  Elem * addElementEdge3(dof_id_type node0, dof_id_type node1, dof_id_type node2);
  Elem *
  addElementQuad4(dof_id_type node0, dof_id_type node1, dof_id_type node2, dof_id_type node3);
  Elem * addElementQuad9(dof_id_type node0,
                         dof_id_type node1,
                         dof_id_type node2,
                         dof_id_type node3,
                         dof_id_type node4,
                         dof_id_type node5,
                         dof_id_type node6,
                         dof_id_type node7,
                         dof_id_type node8);
  /// Gets the next subdomain ID
  virtual SubdomainID getNextSubdomainId();
  /// Gets the next nodeset or sideset ID
  virtual BoundaryID getNextBoundaryId();

protected:
  /// Gets the next node ID
  virtual dof_id_type getNextNodeId();
  /// Gets the next element ID
  virtual dof_id_type getNextElementId();

  /// The dimension of the mesh
  unsigned int _dim;

  /// The next node ID in the mesh (used for mesh generation)
  dof_id_type _next_node_id;
  /// The next element ID in the mesh (used for mesh generation)
  dof_id_type _next_element_id;
  /// The next subdomain ID in the mesh (used for mesh generation)
  SubdomainID _next_subdomain_id;
  /// The next boundary ID in the mesh (used for mesh generation)
  BoundaryID _next_boundary_id;

public:
  static const BoundaryName INVALID_BOUNDARY_ID;

  static InputParameters validParams();
};
