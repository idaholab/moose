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

  /// Range of the elements who changed their subdomain ID
  ConstElemRange & movedElemsRange() const { return *_moved_elems_range; }

  /// Range of the boundary nodes on moved elements
  ConstBndNodeRange & movedBndNodesRange() const { return *_moved_bnd_nodes_range; }

  /// Pointer to the displaced problem
  DisplacedProblem * _displaced_problem;

private:
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

  /// Helper function to build the range of moved elements
  void buildMovedElemsRange();

  /// Helper function to build the range of boundary nodes on moved elements
  void buildMovedBndNodesRange();

  /// Set old and older solutions to be the same as the current solution
  void setOldAndOlderSolutionsForMovedNodes(SystemBase & sys);

  /// Elements on the undisplaced mesh whose subdomain IDs have changed
  std::vector<const Elem *> _moved_elems;

  /// Elements on the displaced mesh whose subdomain IDs have changed
  std::vector<const Elem *> _moved_displaced_elems;

  /// Nodes on the moved elements
  std::set<dof_id_type> _moved_nodes;

  /// Range of the moved elements
  std::unique_ptr<ConstElemRange> _moved_elems_range;

  /// Range of the boundary nodes on the moved elements
  std::unique_ptr<ConstBndNodeRange> _moved_bnd_nodes_range;

  /// Whether to re-apply ICs on moved elements and moved nodes
  const bool _apply_ic;

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
};
