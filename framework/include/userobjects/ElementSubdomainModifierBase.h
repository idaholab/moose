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

class ElementSubdomainModifierBase : public ElementUserObject
{
public:
  static InputParameters validParams();

  ElementSubdomainModifierBase(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void meshChanged() override;

protected:
  /**
   * Modify the element subdomains
   *
   * This method performs the following mesh/system modifications given the list of element
   * subdomain assignment:
   *   1. Moving boundaries are created (if it doesn't already exist) according to the specified
   *      subdomain pairs.
   *   2. Active (in the context of adaptivity) elements' subdomain IDs are modified per request.
   *   3. Existing moving boundaries are updated according to the new subdomains.
   *   4. The equation system is reinitialized to (de)allocate for the new subdomains.
   *   5. Variables and stateful material properties on newly activated elements and nodes are
   *      initialized.
   *
   * @param moved_elems A dictionary of element subdomain assignments. Key of this dictionary is the
   * element ID, first entry of the value pair is the subdomain ID this element is moving _from_,
   * and the second entry of the value pair is the subdomain ID this element is moving _to_.
   */
  virtual void
  modify(const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems);

  /// Determine if a subdomain is active
  bool subdomainIsActive(SubdomainID id) const;

  /// Pointer to the displaced problem
  DisplacedProblem * _displaced_problem;

  /// Displaced mesh
  MooseMesh * _displaced_mesh;

  /// Boundary names associated with each moving boundary ID
  std::unordered_map<BoundaryID, BoundaryName> _moving_boundary_names;

private:
  /// Create moving boundaries
  void createMovingBoundaries(MooseMesh & mesh);

  void applySubdomainChanges(
      const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems,
      MooseMesh & mesh);

  /// Change the subdomain ID of all ancestor elements
  void setAncestorsSubdomainIDs(Elem * elem, const SubdomainID subdomain_id);

  void gatherMovingBoundaryChanges(
      const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems);

  void gatherMovingBoundaryChangesHelper(const Elem * elem,
                                         unsigned short side,
                                         const Elem * neigh,
                                         unsigned short neigh_side);

  void applyMovingBoundaryChanges(MooseMesh & mesh);

  void removeInactiveMovingBoundary(MooseMesh & mesh);

  void findActivatedElemsAndNodes(
      const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems);

  /// Determine if a node is newly activated
  bool nodeIsNewlyActivated(dof_id_type node_id) const;

  void applyIC(bool displaced);

  void initElementStatefulProps(bool displaced);

  /// Range of activated elements
  ConstElemRange & activatedElemRange(bool displaced = false);

  /// Range of activated boundary nodes
  ConstBndNodeRange & activatedBndNodeRange(bool displaced = false);

  /// The active subdomains on which the problem is being solved
  const std::vector<SubdomainID> _active_subdomains;

  /// Whether to consider active->active subdomain changes as "activated"
  const bool _amorphous_activation;

  /// Moving boundaries associated with each subdomain pair
  typedef std::pair<SubdomainID, SubdomainID> SubdomainPair;
  std::unordered_map<SubdomainPair, BoundaryID> _moving_boundaries;

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
  std::unordered_set<dof_id_type> _activated_elems;
  std::unique_ptr<ConstElemRange> _activated_elem_range;
  std::unique_ptr<ConstElemRange> _activated_displaced_elem_range;

  /// Range of activated boundary nodes
  std::unordered_set<dof_id_type> _activated_nodes;
  std::unique_ptr<ConstBndNodeRange> _activated_bnd_node_range;
  std::unique_ptr<ConstBndNodeRange> _activated_displaced_bnd_node_range;
};
