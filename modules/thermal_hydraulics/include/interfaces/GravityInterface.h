//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "libmesh/vector_value.h"

/**
 * Interface for specifying gravity at the component level
 */
class GravityInterface
{
public:
  GravityInterface(const InputParameters & parameters);

  /**
   * Gets gravity magnitude
   */
  Real getGravityMagnitude() const { return _gravity_magnitude; }

protected:
  /// Gravitational acceleration vector
  const RealVectorValue & _gravity_vector;
  /// Gravitational acceleration magnitude
  const Real _gravity_magnitude;
  /// Gravitational acceleration magnitude is zero?
  const bool _gravity_is_zero;
  /// Gravitational acceleration unit direction
  const RealVectorValue _gravity_direction;

public:
  static InputParameters validParams();
};
