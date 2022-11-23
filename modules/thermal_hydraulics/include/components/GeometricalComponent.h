//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component.h"
#include "DiscreteLineSegmentInterface.h"
#include "libmesh/enum_elem_type.h"

/**
 * Intermediate class for all geometrical components (i.e components that have position, direction,
 * etc. in space - they generate a mesh)
 */
class GeometricalComponent : public Component, public DiscreteLineSegmentInterface
{
public:
  GeometricalComponent(const InputParameters & parameters);

  /**
   * Gets the subdomain names for this component
   *
   * @return vector of subdomain names for this component
   */
  virtual const std::vector<SubdomainName> & getSubdomainNames() const;

  /**
   * Gets the coordinate system types for this component
   *
   * @return vector of coordinate system types for this component
   */
  virtual const std::vector<Moose::CoordinateSystemType> & getCoordSysTypes() const;

  const std::vector<dof_id_type> & getNodeIDs() const;

  const std::vector<dof_id_type> & getElementIDs() const;

protected:
  Node * addNode(const Point & pt);

  Elem * addElement(libMesh::ElemType elem_type, const std::vector<dof_id_type> & node_ids);
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
  virtual void check() const override;
  virtual void setupMesh() override;

  virtual void buildMesh() = 0;

  /**
   * Check if second order mesh is being used by this geometrical component
   *
   * @return true if second order mesh is being used, otherwise false
   */
  virtual bool usingSecondOrderMesh() const = 0;

  /**
   * Sets the next subdomain ID, name, and coordinate system
   *
   * @param[in] subdomain_id  subdomain index
   * @param[in] subdomain_name  name of the new subdomain
   * @param[in] coord_system  type of coordinate system
   */
  virtual void
  setSubdomainInfo(SubdomainID subdomain_id,
                   const std::string & subdomain_name,
                   const Moose::CoordinateSystemType & coord_system = Moose::COORD_XYZ);

  const FunctionName & getVariableFn(const FunctionName & fn_param_name);

  /// Axial region names
  const std::vector<std::string> & _axial_region_names;

  /// The name of the user object to displace nodes into the physical space
  UserObjectName _displace_node_user_object_name;

  /// Node locations along the main axis
  std::vector<Real> _node_locations;

  /// List of subdomain IDs this components owns
  std::vector<SubdomainID> _subdomain_ids;
  /// List of subdomain names this components owns
  std::vector<SubdomainName> _subdomain_names;
  /// List of coordinate system for each subdomain
  std::vector<Moose::CoordinateSystemType> _coord_sys;
  /// List of node IDs this components owns
  std::vector<dof_id_type> _node_ids;
  /// Elements ids of this flow channel component
  std::vector<dof_id_type> _elem_ids;

private:
  void generateNodeLocations();
  unsigned int computeNumberOfNodes(unsigned int n_elems);
  std::vector<Real> getUniformNodeLocations(Real length, unsigned int n_nodes);
  void placeLocalNodeLocations(Real start_length,
                               unsigned int start_node,
                               std::vector<Real> & local_node_locations);

public:
  static InputParameters validParams();
};
