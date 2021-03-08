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
 * Compute a displacement angle field w.r.t. a point and an optional plane
 */
class DeflectionAngle : public AuxKernel
{
public:
  static InputParameters validParams();

  DeflectionAngle(const InputParameters & parameters);

protected:
  Real computeValue() override;

  /// origin point to determine teh angle w.r.t.
  const Point _origin;

  /// was a direction vector specified?
  const bool _has_direction;

  /// optional direction. If specified compute angles in the plane defined by this vector
  RealVectorValue _direction;

  /// displacements
  std::vector<const VariableValue *> _disp;
};
