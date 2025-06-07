//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActionComponent.h"
#include "InputParameters.h"
#include "MooseTypes.h"

/**
 * Helper class to help Components be located with the orientation and position we want.
 * The template argument is the dimension: 0D components don't need to be re-oriented,
 * while 1D, 2D and 3D components could need this feature.
 */
class ComponentMeshTransformHelper : public virtual ActionComponent
{
public:
  static InputParameters validParams();

  ComponentMeshTransformHelper(const InputParameters & params);

protected:
  virtual void addMeshGenerators() override;

  /// Rotation angles
  const RealVectorValue _rotation;
  /// Direction vector (easier to conceptualize than rotation)
  const RealVectorValue _direction;
  /// Translation vector
  const Point _translation;
};
