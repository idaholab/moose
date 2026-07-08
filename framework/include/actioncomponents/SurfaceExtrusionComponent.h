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
 * Component living on a mesh that is extruded from a surface of the mesh of another component
 */
class SurfaceExtrusionComponent : public virtual ActionComponent,
                                  public ComponentPhysicsInterface,
                                  public ComponentMaterialPropertyInterface,
                                  public ComponentInitialConditionInterface,
                                  public ComponentBoundaryConditionInterface
{
public:
  static InputParameters validParams();
  SurfaceExtrusionComponent(const InputParameters & params);

protected:
  virtual void addMeshGenerators() override;
  virtual void checkIntegrity() override;

  /// Length of the extrusion
  const Real _length;
  /// Outer boundaries of the component
  std::vector<BoundaryName> _outer_boundaries;

  virtual Real volume() const override { mooseError("Not implemented"); }
  virtual Real outerSurfaceArea() const override { mooseError("Not implemented"); }
  virtual const std::vector<BoundaryName> & outerSurfaceBoundaries() const override
  {
    return _outer_boundaries;
  };
};
