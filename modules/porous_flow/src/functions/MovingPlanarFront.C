//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MovingPlanarFront.h"

registerMooseObject("PorousFlowApp", MovingPlanarFront);

InputParameters
MovingPlanarFront::validParams()
{
  InputParameters params = Function::validParams();
  params.addRequiredParam<RealVectorValue>("start_posn", "Initial position of the front");
  params.addRequiredParam<RealVectorValue>("end_posn", "Final position of the front");
  params.addRequiredParam<FunctionName>(
      "distance",
      "The front is an infinite plane with normal pointing from start_posn to "
      "end_posn.  The front's distance from start_posn is defined by distance.  You "
      "should ensure that distance is positive");
  params.addParam<Real>(
      "active_length",
      std::numeric_limits<Real>::max(),
      "Points greater than active_length behind the front will return false_value");
  params.addParam<Real>("true_value", 1.0, "Return this value if a point is in the active zone.");
  params.addParam<Real>(
      "false_value", 0.0, "Return this value if a point is not in the active zone.");
  params.addParam<Real>("activation_time",
                        std::numeric_limits<Real>::lowest(),
                        "This function will return false_value when t < activation_time");
  params.addParam<Real>("deactivation_time",
                        std::numeric_limits<Real>::max(),
                        "This function will return false_value when t >= deactivation_time");
  params.addClassDescription(
      "This function defines the position of a moving front.  The front is "
      "an infinite plane with normal pointing from start_posn to end_posn.   The front's distance "
      "from start_posn is defined by 'distance', so if the 'distance' function is time dependent, "
      "the front's position will change with time.  Roughly speaking, the function returns "
      "true_value for points lying in between start_posn and start_posn + distance.  Precisely "
      "speaking, two planes are constructed, both with normal pointing from start_posn to "
      "end_posn.  The first plane passes through start_posn; the second plane passes through "
      "end_posn.  Given a point p and time t, this function returns false_value if ANY of the "
      "following are true: (a) t<activation_time; (b) t>=deactivation_time; (c) p is 'behind' "
      "start_posn (ie, p lies on one side of the start_posn plane and end_posn lies on the other "
      "side); (d) p is 'ahead' of the front (ie, p lies one one side of the front and start_posn "
      "lies on the other side); (e) the distance between p and the front is greater than "
      "active_length.  Otherwise, the point is 'in the active zone' and the function returns "
      "true_value.");
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
  if (_front_normal.norm() == 0)
    mooseError("MovingPlanarFront: start_posn and end_posn must be different points");
  _front_normal /= _front_normal.norm();
}

Real
MovingPlanarFront::value(Real t, const Point & p) const
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
