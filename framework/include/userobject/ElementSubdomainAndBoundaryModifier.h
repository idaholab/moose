//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementSubdomainModifier.h"

class ElementSubdomainAndBoundaryModifier : public ElementSubdomainModifier
{
public:
  static InputParameters validParams();

  ElementSubdomainAndBoundaryModifier(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  // The ID of the moving boundary that this object creates/modifies.
  BoundaryID movingBoundaryID() const
  {
    if (!_moving_boundary_specified)
      mooseError("no moving boundary specified");
    return _moving_boundary_id;
  }

  // The name of the moving boundary that this object creates/modifies.
  const BoundaryName movingBoundaryName() const
  {
    if (!_moving_boundary_specified)
      mooseError("no moving boundary specified");
    return _moving_boundary_name;
  }

  void updateBoundaryInfo(MooseMesh & mesh, const std::vector<const Elem *> & moved_elems) override;

private:
  // Set the name of the moving boundary. Create the nodeset/sideset if not exist.
  void setMovingBoundaryName(MooseMesh & mesh);

  // Remove ghosted element sides
  void pushBoundarySideInfo(
      MooseMesh & mesh,
      std::unordered_map<processor_id_type, std::vector<std::pair<dof_id_type, unsigned int>>> &
          elems_to_push);

  // Remove ghosted boundary nodes
  void pushBoundaryNodeInfo(
      MooseMesh & mesh,
      std::unordered_map<processor_id_type, std::vector<dof_id_type>> & nodes_to_push);

  /// Whether a moving boundary name is provided
  const bool _moving_boundary_specified;

  /// The name of the moving boundary
  BoundaryName _moving_boundary_name;

  /// The Id of the moving boundary
  BoundaryID _moving_boundary_id;
};
