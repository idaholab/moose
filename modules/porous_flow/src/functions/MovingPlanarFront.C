/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MovingPlanarFront.h"

template <>
InputParameters
validParams<MovingPlanarFront>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<RealVectorValue>("start_posn", "Initial position of the front");
  params.addRequiredParam<RealVectorValue>("end_posn", "Final position of the front");
  params.addRequiredParam<FunctionName>(
      "distance",
      "The front is an infinite plane with normal pointing from start_posn to "
      "end_posn.  The front's distance from start_posn is defined by distance.  You "
      "should ensure that distance is positive");
  params.addRequiredParam<Real>(
      "active_length",
      "This function will return true_value at a point if: (a) t >= "
      "activation_time; (b) t < deactivation_time; (c) the point lies in the "
      "domain between start_posn and the front position; (d) the distance between "
      "the point and the front position <= active_length.");
  params.addParam<Real>("true_value", 1.0, "Return this value if a point is in the active zone.");
  params.addParam<Real>(
      "false_value", 0.0, "Return this value if a point is not in the active zone.");
  params.addParam<Real>("activation_time",
                        std::numeric_limits<Real>::lowest(),
                        "This function will return false_value when t < activation_time");
  params.addParam<Real>("deactivation_time",
                        std::numeric_limits<Real>::max(),
                        "This function will return false_value when t >= deactivation_time");
  params.addClassDescription("This function defines the position of a moving front.  The front is "
                             "an infinite plane with normal pointing from start_posn to end_posn.  "
                             "The front's distance from start_posn is defined by distance");
  return params;
}

MovingPlanarFront::MovingPlanarFront(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),
    _start_posn(getParam<RealVectorValue>("start_posn")),
    _end_posn(getParam<RealVectorValue>("end_posn")),
    _distance(getFunction("distance")),
    _active_length(getParam<Real>("active_length")),
    _true_value(getParam<Real>("true_value")),
    _false_value(getParam<Real>("false_value")),
    _activation_time(getParam<Real>("activation_time")),
    _deactivation_time(getParam<Real>("deactivation_time")),
    _front_normal(_end_posn - _start_posn)
{
  if (_front_normal.size() == 0)
    mooseError("MovingPlanarFront: start_posn and end_posn must be different points");
  _front_normal /= _front_normal.size();
}

Real
MovingPlanarFront::value(Real t, const Point & p)
{
  if (t < _activation_time)
    return _false_value;

  if (t >= _deactivation_time)
    return _false_value;

  if ((p - _start_posn) * _front_normal < 0)
    // point is behind start posn - it'll never be active
    return _false_value;

  const RealVectorValue current_posn = _start_posn + _distance.value(t, p) * _front_normal;

  const Real distance_ahead_of_front = (p - current_posn) * _front_normal;

  if (distance_ahead_of_front > 0)
    return _false_value;

  if (distance_ahead_of_front < -_active_length)
    // point is too far behind front
    return _false_value;

  return _true_value;
}
