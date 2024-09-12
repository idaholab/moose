//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ActionComponent.h"
#include "PhysicsComponentBase.h"

/**
 * Component whose mesh is generated in the [Mesh] block on which one can define a Physics.
 */
class MeshGeneratorComponent : public virtual ActionComponent, public PhysicsComponentBase
{
public:
  /**
   * Class constructor
   */
  static InputParameters validParams();

  MeshGeneratorComponent(const InputParameters & params);

protected:
  virtual void addMeshGenerators() override;
  virtual void setupComponent() override;

private:
  /// Whether the component uses a saved mesh or the final mesh
  const MooseEnum _mesh_generator_type;
};
