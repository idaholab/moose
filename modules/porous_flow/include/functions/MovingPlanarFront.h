//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"
#include "FunctionInterface.h"

/**
 * Defines the position of a moving front.
 * The front is an infinite plane with normal pointing from start_posn to end_posn.
 * The front's distance from start_posn is defined by the 'distance' function
 *
 * This Function may be used to define the geometry of an underground excavation,
 * probably in conjunction with a predefined sideset.
 */
class MovingPlanarFront : public Function, protected FunctionInterface
{
public:
  static InputParameters validParams();

  MovingPlanarFront(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

protected:
  /// Initial position of front
  const RealVectorValue _start_posn;

  /// Final position of the front: together with start_posn this defines the front's normal
  const RealVectorValue _end_posn;

  /// The front's distance from start_posn (along the normal direction)
  const Function & _distance;

  /// Active length
  const Real _active_length;

  /// True value to return
  const Real _true_value;

  /// False value to return
  const Real _false_value;

  /// Activation time
  const Real _activation_time;

  /// Deactivation time
  const Real _deactivation_time;

  /// Front unit normal
  RealVectorValue _front_normal;
};
