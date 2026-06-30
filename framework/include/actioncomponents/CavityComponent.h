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

/**
 * Cavity mesh on which one can define a Physics. The volumetric mesh is automatically created using
 * general volume meshing techniques in between the provided surface mesh and the enclosed
 * components
 */
class CavityComponent : public virtual ActionComponent,
                        public ComponentPhysicsInterface,
                        public ComponentMaterialPropertyInterface,
                        public ComponentInitialConditionInterface,
                        public ComponentBoundaryConditionInterface
{
public:
  static InputParameters validParams();
  CavityComponent(const InputParameters & params);

protected:
  virtual void addMeshGenerators() override;
  virtual void checkIntegrity() override;

  /// Target element volume for cavity meshing
  const Real _target_elem_volume;
  /// Point to re-center the surface mesh on
  const Point _center_surface_mesh;
  /// Name of the mesh generator providing the surface (enclosure) mesh
  const MeshGeneratorName _surface_mesh_mg;
  /// Names of the components to enclose in the cavity
  const std::vector<ComponentName> & _components_to_enclose;
};
