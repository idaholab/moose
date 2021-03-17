//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Compute the field of angular rotations of points around an axis defined by an origin point and a
 * direction vector
 */
class RotationAngle : public AuxKernel
{
public:
  static InputParameters validParams();

  RotationAngle(const InputParameters & parameters);

protected:
  Real computeValue() override;

  /// origin point to determine the angle w.r.t.
  const Point _origin;

  /// compute angles in the plane defined by this vector
  RealVectorValue _direction;

  /// displacement variables
  std::vector<const VariableValue *> _disp;
};
