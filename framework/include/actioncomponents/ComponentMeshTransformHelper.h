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

  /// Return the direction (axis for a cylinder for example) of the component
  RealVectorValue direction() const
  {
    mooseAssert(_direction, "No direction specified");
    return *_direction;
  }
  /// Return the XZX angle rotation for the component
  /// NOTE: it is often easier to work with the direction instead
  RealVectorValue rotation() const
  {
    mooseAssert(_rotation, "No rotation specified");
    return *_rotation;
  }
  /// Return the translation of the component
  /// The default translation is the null vector
  virtual Point translation() const { return _translation; }

protected:
  virtual void addMeshGenerators() override;

  /// Rotation angles
  const RealVectorValue * const _rotation;
  /// Direction vector (easier to conceptualize than rotation)
  const RealVectorValue * const _direction;
  /// Translation vector
  const Point _translation;
};
