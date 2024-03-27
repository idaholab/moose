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
  virtual void timestepSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & /*uo*/) override;
  virtual void finalize() override;
  virtual void meshChanged() override;

protected:
  /// Compute the subdomain ID of the current element
  virtual SubdomainID computeSubdomainID() = 0;

  /// The ID of the moving boundary that this object creates/modifies.
  BoundaryID movingBoundaryID() const
  {
    if (!_moving_boundary_specified)
      mooseError("no moving boundary specified");
    return _moving_boundary_id;
  }

  /// The ID of the complement moving boundary that this object creates/modifies.
  BoundaryID complementMovingBoundaryID() const
  {
    if (!_complement_moving_boundary_specified)
      mooseError("no complement moving boundary specified");
    return _complement_moving_boundary_id;
  }

  /// The name of the moving boundary that this object creates/modifies.
  const BoundaryName movingBoundaryName() const
  {
    if (!_moving_boundary_specified)
      mooseError("no moving boundary specified");
    return _moving_boundary_name;
  }

  /// The name of the complement moving boundary that this object creates/modifies.
  const BoundaryName complementMovingBoundaryName() const
  {
    if (!_complement_moving_boundary_specified)
      mooseError("no complement moving boundary specified");
    return _complement_moving_boundary_name;
  }

  /// Range of activated elements
  ConstElemRange & activatedElemsRange() const { return *_activated_elems_range; }

  /// Range of activated nodes
  ConstNodeRange & activatedNodesRange() const { return *_activated_nodes_range; }

  /// Range of activated boundary nodes
  ConstBndNodeRange & activatedBndNodesRange() const { return *_activated_bnd_nodes_range; }

  /// Pointer to the displaced problem
  DisplacedProblem * _displaced_problem;

  /// Nonlinear system
  NonlinearSystemBase & _nl_sys;

  /// Auxiliary system
  AuxiliarySystem & _aux_sys;

private:
  /// Serialize the old solution
  void serializeSolutionOld(dof_id_type ndof,
                            SystemBase & sys,
                            std::unique_ptr<NumericVector<Real>> & sol);

  /// Set the name of the moving boundary. Create the nodeset/sideset if not exist.
  void setMovingBoundaryName(MooseMesh & mesh);

  /// Set the name of the complement moving boundary. Create the nodeset/sideset if not exist.
  void setComplementMovingBoundaryName(MooseMesh & mesh);

  /// Update the moving boundary (both the underlying sideset and nodeset)
  void updateMovingBoundaryInfo(MooseMesh & mesh, const std::vector<const Elem *> & moved_elems);

  /// Update the complement boundary (both the underlying sideset and nodeset)
  void updateComplementBoundaryInfo(MooseMesh & mesh,
                                    const std::vector<const Elem *> & moved_elems);

  /// Remove ghosted element sides
  void pushBoundarySideInfo(MooseMesh & mesh);

  /// Remove ghosted boundary nodes
  void pushBoundaryNodeInfo(MooseMesh & mesh);

  /// synchronize boundary information across processors
  void synchronizeBoundaryInfo(MooseMesh & mesh);

  /// Change the subdomain ID of all ancestor elements
  void setAncestorsSubdomainIDs(const SubdomainID & subdomain_id, const dof_id_type & elem_id);

  /// Helper function to build the range of activated elements
  void buildActivatedElemsRange();

  /// Helper function to build the range of activated nodes
  void buildActivatedNodesRange();

  /// Helper function to build the range of activated boundary nodes
  void buildActivatedBndNodesRange();

  /// Find nearest dofs for each variable
  void findNearestDofs(SystemBase &, const VariableName &);

  /// Set current solution to be the same as the nearest dof's old value
  void
  setNearestSolutionForActivatedDofs(SystemBase &, const VariableName &, NumericVector<Real> &);

  /// Set activated dofs to constant
  void setConstantForActivatedDofs(SystemBase & sys, const VariableName & var_name);

  /// Elements on the undisplaced mesh whose subdomain IDs have changed
  std::vector<const Elem *> _moved_elems;

  /// Elements on the displaced mesh whose subdomain IDs have changed
  std::vector<const Elem *> _moved_displaced_elems;

  /// Newly activated elements
  std::vector<const Elem *> _activated_elems;

  /// Newly activated nodes
  std::vector<const Node *> _activated_nodes;

  /// Newly activated boundary nodes
  std::vector<const BndNode *> _activated_bnd_nodes;

  /// Range of activated elements
  std::unique_ptr<ConstElemRange> _activated_elems_range;

  /// Range of activated nodes
  std::unique_ptr<ConstNodeRange> _activated_nodes_range;

  /// Range of activated boundary nodes
  std::unique_ptr<ConstBndNodeRange> _activated_bnd_nodes_range;

  /// Do we need to initialize any variable?
  const std::vector<VariableName> _init_vars;

  /// The strategy used to apply "initial condition" for newly activated nodes
  std::vector<MooseEnum> _init_strategy;

  /// The constant to use for _init_strategy == "CONSTANT"
  std::map<VariableName, Real> _init_constant;

  /// The active subdomains on which the problem is being solved
  const std::vector<SubdomainID> _active_subdomains;

  /// Whether a moving boundary name is provided
  const bool _moving_boundary_specified;

  /// Whether a complement moving boundary name is provided
  const bool _complement_moving_boundary_specified;

  /// The name of the moving boundary
  BoundaryName _moving_boundary_name;

  /// The name of the complement moving boundary
  BoundaryName _complement_moving_boundary_name;

  /// The Id of the moving boundary
  BoundaryID _moving_boundary_id;

  /// The Id of the complement moving boundary
  BoundaryID _complement_moving_boundary_id;

  /// save the added/removed ghost sides to sync across processors
  std::unordered_map<processor_id_type, std::vector<std::pair<dof_id_type, unsigned short int>>>
      _complement_ghost_sides_to_remove, _complement_ghost_sides_to_add, _ghost_sides_to_add;

  /// save the added/removed ghost nodes to sync across processors
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> _complement_ghost_nodes_to_remove;

  /// Any subdomain change is stored in this map and only applied in finalize to avoid messing up other UOs
  std::vector<std::pair<Elem *, SubdomainID>> _cached_subdomain_assignments;

  /// Subdomains between that the moving boundary is
  std::set<SubdomainID> _moving_boundary_subdomains;

  /// Nearest dofs for each variable
  std::map<unsigned int, std::map<dof_id_type, std::vector<dof_id_type>>> _elem_nearest_dofs;
  std::map<unsigned int, std::map<dof_id_type, std::vector<dof_id_type>>> _node_nearest_dofs;

  /// Number of dofs in nonlinear system
  dof_id_type _nl_ndof;

  /// The serialized nonlinear solution vector
  std::unique_ptr<NumericVector<Real>> _nl_sol_old;

  /// Number of dofs in aux system
  dof_id_type _aux_ndof;

  /// The serialized aux solution vector
  std::unique_ptr<NumericVector<Real>> _aux_sol_old;
};
