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

  /// Range of activated elements
  ConstElemRange & activatedElemRange(bool displaced = false);

  /// Range of activated boundary nodes
  ConstBndNodeRange & activatedBndNodeRange(bool displaced = false);

  /// Pointer to the displaced problem
  DisplacedProblem * _displaced_problem;

  /// Displaced mesh
  MooseMesh * _displaced_mesh;

  /// Nonlinear system
  NonlinearSystemBase & _nl_sys;

  /// Auxiliary system
  AuxiliarySystem & _aux_sys;

private:
  /// Create moving boundaries
  void createMovingBoundaries(MooseMesh & mesh);

  bool nodeIsNewlyActivated(dof_id_type node_id) const;

  /// Update the moving boundary (both the underlying sideset and nodeset)
  void updateMovingBoundaryInfo(MooseMesh & mesh);

  /// Change the subdomain ID of all ancestor elements
  void setAncestorsSubdomainIDs(const SubdomainID & subdomain_id, const dof_id_type & elem_id);

  /// Elements on the undisplaced mesh whose subdomain IDs have changed
  std::unordered_set<dof_id_type> _moved_elems;

  /// Newly activated elements
  std::unordered_set<dof_id_type> _activated_elems;

  /// Newly activated nodes
  std::unordered_set<dof_id_type> _activated_nodes;

  // @{ For variable initialization
  /// The active subdomains on which the problem is being solved
  const std::vector<SubdomainID> _active_subdomains;
  /// Variables to initialize
  const std::vector<VariableName> _init_vars;
  /// Range of activated elements
  std::unique_ptr<ConstElemRange> _activated_elem_range;
  std::unique_ptr<ConstElemRange> _activated_displaced_elem_range;
  /// Range of activated boundary nodes
  std::unique_ptr<ConstBndNodeRange> _activated_bnd_node_range;
  std::unique_ptr<ConstBndNodeRange> _activated_displaced_bnd_node_range;
  // @}

  // @{ For moving boundary
  const std::vector<BoundaryName> _moving_boundary_names;
  struct MovingBoundary
  {
    BoundaryName name;
    BoundaryID id;
    std::vector<SubdomainID> subdomains;
  };
  std::vector<MovingBoundary> _moving_boundaries;
  // @}

  /// Any subdomain change is stored in this map and only applied in finalize to avoid messing up other UOs
  std::unordered_map<dof_id_type, SubdomainID> _moved_elems;
};
