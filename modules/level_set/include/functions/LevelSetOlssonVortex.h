//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LEVELSETOLSSONVORTEX_H
#define LEVELSETOLSSONVORTEX_H

#include "Function.h"

// Forward declarations
class LevelSetOlssonVortex;

template <>
InputParameters validParams<LevelSetOlssonVortex>();

/**
 * Defines a vortex velocity field in the x-y plane.
 */
class LevelSetOlssonVortex : public Function
{
public:
  LevelSetOlssonVortex(const InputParameters & parameters);

  Real value(Real t, const Point & p) override;

  RealVectorValue vectorValue(Real t, const Point & p) override;

protected:
  /// Total time for the velocity field to complete reverse
  const Real & _reverse_time;

  /// Type of reverse (instantaneous or smooth)
  const MooseEnum & _reverse_type;

  /// The vector component to return
  const MooseEnum & _component;

  /// The velocity field computed
  RealVectorValue _output;

  /// The time reversal coefficient
  Real _reverse_coefficient;

  // Convenience for libMesh::pi
  const Real _pi;
};

#endif // LEVELSETOLSSONVORTEX_H
