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

class ActivateElementsUserObjectBase : public ElementUserObject
{
public:
  static InputParameters validParams();

  ActivateElementsUserObjectBase(const InputParameters & parameters);

  const std::set<dof_id_type> & getNewlyActivatedElements() const { return _newly_activated_elem; };

  BoundaryID getExpandedBoundaryID()
  {
    mooseAssert(!_boundary_ids.empty(), "Boundary ID is empty");
    return _boundary_ids[0];
  }

  virtual bool isElementActivated() = 0;

  void initialize() override{};
  void execute() override;
  void threadJoin(const UserObject & /*uo*/) override{};
  void finalize() override;

protected:
  void setNewBoundayName();

  void updateBoundaryInfo(MooseMesh & mesh);

  void push_boundary_side_info(
      MooseMesh & mesh,
      std::unordered_map<processor_id_type, std::vector<std::pair<dof_id_type, unsigned int>>> &
          elems_to_push);

  void push_boundary_node_info(
      MooseMesh & mesh,
      std::unordered_map<processor_id_type, std::vector<dof_id_type>> & nodes_to_push);
  /**
   * Initialize solutions for the nodes
   */
  void initSolutions(ConstElemRange & elem_range, ConstBndNodeRange & bnd_node_range);
  /**
   * Returns true if all the connected elements are in the _newly_activated_elem
   */
  bool isNewlyActivated(const Node * node);

  void getNodesToRemoveFromBnd(std::set<dof_id_type> & remove_set, std::set<dof_id_type> & add_set);

  void insertNodeIdsOnSide(const Elem * ele,
                           const unsigned short int side,
                           std::set<dof_id_type> & node_ids);

  /**
   * Get ranges for use with threading.
   */
  ConstElemRange * getNewlyActivatedElementRange();
  ConstBndNodeRange * getNewlyActivatedBndNodeRange();
  ConstNodeRange * getNewlyActivatedNodeRange();

  std::set<dof_id_type> _newly_activated_elem;
  std::set<dof_id_type> _newly_activated_node;
  /**
   * Somes nodes are to be removed from the boundary
   * when adding/removing sides
   */
  std::set<dof_id_type> _node_to_remove_from_bnd;

  /**
   * Ranges for use with threading.
   */
  std::unique_ptr<ConstElemRange> _activated_elem_range;
  std::unique_ptr<ConstBndNodeRange> _activated_bnd_node_range;
  std::unique_ptr<ConstNodeRange> _activated_node_range;

  /// activate subdomain ID
  const subdomain_id_type _active_subdomain_id;
  /// inactivate subdomain ID (the subdomain that you want to keep the same)
  const subdomain_id_type _inactive_subdomain_id;
  /// expanded boundary name
  const std::vector<BoundaryName> _expand_boundary_name;
  /// expanded boundary IDs
  std::vector<BoundaryID> _boundary_ids, _disp_boundary_ids;
};
