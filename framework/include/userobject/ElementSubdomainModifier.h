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

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & /*uo*/) override{};
  virtual void finalize() override;

protected:
  // Compute the subdomain ID of the current element
  virtual SubdomainID computeSubdomainID() const = 0;

  // Range of the elements who changed their subdomain ID
  ConstElemRange & movedElemsRange() const { return *_moved_elems_range; }

  // Range of the boundary nodes on moved elements
  ConstBndNodeRange & movedBndNodesRange() const { return *_moved_bnd_nodes_range; }

  // Pointer to the displaced problem
  DisplacedProblem * _displaced_problem;

  // Update the moving boundary (both the underlying sideset and nodeset)
  virtual void updateBoundaryInfo(MooseMesh & mesh, const std::vector<const Elem *> & moved_elems);

private:
  // Helper function to add nodes on a side of an element to a set
  void recordNodeIdsOnElemSide(const Elem * elem,
                               const unsigned short int side,
                               std::set<dof_id_type> & node_ids);

  // Helper function to build the range of moved elements
  void buildMovedElemsRange();

  // Helper function to build the range of boundary nodes on moved elements
  void buildMovedBndNodesRange();

  // Set old and older solutions to be the same as the current solution
  void setOldAndOlderSolutionsForMovedNodes(SystemBase & sys);

  // Elements on the undisplaced mesh whose subdomain IDs have changed
  std::vector<const Elem *> _moved_elems;

  // Elements on the displaced mesh whose subdomain IDs have changed
  std::vector<const Elem *> _moved_displaced_elems;

  // Nodes on the moved elements
  std::set<dof_id_type> _moved_nodes;

  // Range of the moved elements
  std::unique_ptr<ConstElemRange> _moved_elems_range;

  // Range of the boundary nodes on the moved elements
  std::unique_ptr<ConstBndNodeRange> _moved_bnd_nodes_range;

  // Whether to re-apply ICs on moved elements and moved nodes
  const bool _apply_ic;
};
