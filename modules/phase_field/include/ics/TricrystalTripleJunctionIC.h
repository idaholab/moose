//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

/**
 * TricrystalTripleJunctionIC creates a 3-grain structure with a triple junction
 * centered at _junction as specified by the user.
 */
class TricrystalTripleJunctionIC : public InitialCondition
{
public:
  static InputParameters validParams();

  TricrystalTripleJunctionIC(const InputParameters & parameters);

  virtual Real value(const Point & p);

protected:
  const MooseMesh & _mesh;

  /// Number of order parameters
  const unsigned int _op_num;

  // Order parameter index
  const unsigned int _op_index;

  /// Point where the triple junction occurs
  Point _junction;

  /// Angle of first grain at triple junction in radians
  Real _theta1;

  /// Angle of third grain at triple junction in radians
  Real _theta2;

  ///tangent of the first angle after a shift of pi/2
  Real _tan_theta1;

  ///tangent of the second angle after a shift of pi/2
  Real _tan_theta2;
};
