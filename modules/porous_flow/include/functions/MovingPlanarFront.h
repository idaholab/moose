/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef MOVINGPLANARFRONT_H
#define MOVINGPLANARFRONT_H

#include "Function.h"
#include "FunctionInterface.h"

// Forward Declarations
class MovingPlanarFront;

template <>
InputParameters validParams<MovingPlanarFront>();

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
  MovingPlanarFront(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) override;

protected:
  /// Initial position of front
  const RealVectorValue _start_posn;

  /// Final position of the front: together with start_posn this defines the front's normal
  const RealVectorValue _end_posn;

  /// The front's distance from start_posn (along the normal direction)
  Function & _distance;

  /// active length
  const Real _active_length;

  /// true value to return
  const Real _true_value;

  /// false value to return
  const Real _false_value;

  /// activation time
  const Real _activation_time;

  /// deactivation time
  const Real _deactivation_time;

  /// front unit normal
  RealVectorValue _front_normal;
};

#endif // MOVINGPLANARFRONT_H
