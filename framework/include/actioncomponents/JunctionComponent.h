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

class JunctionComponent : public virtual ActionComponent,
                          public ComponentPhysicsInterface,
                          public ComponentMaterialPropertyInterface,
                          public ComponentInitialConditionInterface,
                          public ComponentBoundaryConditionInterface,
                          public ComponentMeshTransformHelper
{
public:
  static InputParameters validParams();
  JunctionComponent(const InputParameters & params);

protected:
  virtual void addMeshGenerators() override;
  virtual void setupComponent() override;
  virtual void checkIntegrity() override;

  /// starting point/edge/face
  /// start direction
  const libMesh::RealVectorValue start_vec;
  /// end direction
  const libMesh::RealVectorValue end_vec;
}