//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"

class ElementSubdomainModifier : public ElementUserObject
{
public:
  static InputParameters validParams();

  ElementSubdomainModifier(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & /*uo*/) override;
  virtual void finalize() override;
  virtual void meshChanged() override;

protected:
  /// Compute the subdomain ID of the current element
  virtual SubdomainID computeSubdomainID() = 0;

  bool subdomainIsActive(SubdomainID id) const;

  /// Range of activated elements
  ConstElemRange & activatedElemRange(bool displaced = false);

  /// Range of activated boundary nodes
  ConstBndNodeRange & activatedBndNodeRange(bool displaced = false);

  /// Initialize internal data for moving boundary definitions
  virtual void initializeMovingBoundaries();

  /// Pointer to the displaced problem
  DisplacedProblem * _displaced_problem;

  /// Displaced mesh
  MooseMesh * _displaced_mesh;

  /// Nonlinear system
  NonlinearSystemBase & _nl_sys;

  /// Auxiliary system
  AuxiliarySystem & _aux_sys;

  /// Newly activated elements
  std::unordered_set<dof_id_type> _activated_elems;

  /// Newly activated nodes
  std::unordered_set<dof_id_type> _activated_nodes;

  /// The active subdomains on which the problem is being solved
  const std::vector<SubdomainID> _active_subdomains;

  /// Variables to initialize
  const std::vector<VariableName> _init_vars;

  const dof_id_type _dbg_elem_id;

  /// Boundary names associated with each moving boundary ID
  std::unordered_map<BoundaryID, BoundaryName> _moving_boundary_names;

  /// Moving boundaries associated with each subdomain pair
  typedef std::pair<SubdomainID, SubdomainID> SubdomainPair;
  std::unordered_map<SubdomainPair, BoundaryID> _moving_boundaries;

  /**
   * Element subdomain changes for the current execution are stored in this map
   *
   * Key of this map is the element ID, first element of the value is the subdomain ID this element
   * is moving _from_, and the second element of the value is the subdomain ID this element is
   * moving _to_.
   */
  std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> _moved_elems;

private:
  /// Create moving boundaries
  void createMovingBoundaries(MooseMesh & mesh);

  /// Determine if a node is newly activated
  bool nodeIsNewlyActivated(dof_id_type node_id) const;

  void applySubdomainChanges(MooseMesh & mesh);

  /// Change the subdomain ID of all ancestor elements
  void setAncestorsSubdomainIDs(const SubdomainID & subdomain_id, const dof_id_type & elem_id);

  void removeInactiveMovingBoundary(MooseMesh & mesh);

  void gatherMovingBoundaryChanges();

  void gatherMovingBoundaryChangesHelper(const Elem * elem,
                                         const Elem * neigh,
                                         SubdomainID to_subdomain);

  void applyMovingBoundaryChanges(MooseMesh & mesh);

  void applyIC(bool displaced);

  void initElementStatefulProps(bool displaced);

  /// Element sides to be added
  std::unordered_map<dof_id_type, std::unordered_map<unsigned short, BoundaryID>>
      _add_element_sides;

  /// Element sides to be removed
  std::unordered_map<dof_id_type, std::unordered_map<unsigned short, BoundaryID>>
      _remove_element_sides;

  /// Neighbor sides to be added
  std::unordered_map<dof_id_type, std::unordered_map<unsigned short, BoundaryID>>
      _add_neighbor_sides;

  /// Neighbor sides to be removed
  std::unordered_map<dof_id_type, std::unordered_map<unsigned short, BoundaryID>>
      _remove_neighbor_sides;

  /// Range of activated elements
  std::unique_ptr<ConstElemRange> _activated_elem_range;
  std::unique_ptr<ConstElemRange> _activated_displaced_elem_range;

  /// Range of activated boundary nodes
  std::unique_ptr<ConstBndNodeRange> _activated_bnd_node_range;
  std::unique_ptr<ConstBndNodeRange> _activated_displaced_bnd_node_range;
};
