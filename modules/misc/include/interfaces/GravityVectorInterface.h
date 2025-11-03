//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
class GravityVectorInterface
{
public:
  static InputParameters validParams();

  GravityVectorInterface(const MooseObject * moose_object);

  /// Gets gravity magnitude
  Real gravityMagnitude() const { return _gravity_magnitude; }
  /// Gets gravity magnitude
  const RealVectorValue & gravityVector() const { return _gravity_vector; }

private:
  /// Gravitational acceleration vector
  RealVectorValue _gravity_vector;
  /// Gravitational acceleration magnitude
  Real _gravity_magnitude;
  /// Gravitational acceleration unit direction
  RealVectorValue _gravity_direction;
};
