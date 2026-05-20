//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ActionComponent.h"
#include "ComponentPhysicsInterface.h"
#include "ComponentMaterialPropertyInterface.h"
#include "ComponentInitialConditionInterface.h"
#include "ComponentBoundaryConditionInterface.h"
#include "ComponentMeshTransformHelper.h"

/**
 * ActionComponent to connect two components. The connection options are based on the mesh
 * generators available in the framework.
 */
class ComponentJunction : public virtual ActionComponent,
                          public ComponentPhysicsInterface,
                          public ComponentMaterialPropertyInterface,
                          public ComponentInitialConditionInterface,
                          public ComponentBoundaryConditionInterface
{
public:
  static InputParameters validParams();
  ComponentJunction(const InputParameters & params);

protected:
  virtual void addMeshGenerators() override;
  virtual void checkIntegrity() override;

  /// How to connect the two components
  const MooseEnum _junction_method;
  /// Whether to enforce that all nodes match on a boundary when stitching them
  bool _enforce_all_nodes_match_on_boundaries;
};
