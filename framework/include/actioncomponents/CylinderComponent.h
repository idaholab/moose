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
 * Cylinder on which one can define a Physics. The mesh is automatically created
 */
class CylinderComponent : public virtual ActionComponent, public PhysicsComponentBase
{
public:
  static InputParameters validParams();
  CylinderComponent(const InputParameters & params);

protected:
  virtual void addMeshGenerators() override;
  virtual void setupComponent() override;

  /// Radius of the cylinder
  const Real _radius;
  /// Height of the cylinder
  const Real _height;

  virtual Real volume() const override { return _height * libMesh::pi * Utility::pow<2>(_radius); }
  virtual Real outerSurfaceArea() const override
  {
    return 2 * libMesh::pi * (Utility::pow<2>(_radius) + _radius * _height);
  }
};
