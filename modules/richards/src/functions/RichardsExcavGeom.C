//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsExcavGeom.h"

registerMooseObject("RichardsApp", RichardsExcavGeom);

InputParameters
RichardsExcavGeom::validParams()
{
  InputParameters params = Function::validParams();
  params.addRequiredParam<RealVectorValue>(
      "start_posn",
      "Start point of the excavation.  This is an (x,y,z) point in the middle of the "
      "coal face at the very beginning of the panel.");
  params.addRequiredParam<Real>("start_time", "Commencement time of the excavation");
  params.addRequiredParam<RealVectorValue>("end_posn",
                                           "End position of the excavation.  This is "
                                           "an (x,y,z) point in the middle of the coal "
                                           "face at the very end of the panel.");
  params.addRequiredParam<Real>("end_time", "Time at the completion of the excavation");
  params.addRequiredParam<Real>("active_length",
                                "This function is only active at a point if the "
                                "distance between the point and the coal face <= "
                                "active_length.");
  params.addParam<Real>("true_value",
                        1.0,
                        "Return this value if a point is in the active zone.  "
                        "This is usually used for controlling "
                        "permeability-changes");
  params.addParam<Real>(
      "deactivation_time", 1.0E30, "Time at which this function is totally turned off");
  params.addClassDescription("This function defines excavation geometry.  It can be used to "
                             "enforce pressures at the boundary of excavations, and to record "
                             "fluid fluxes into excavations.");
  return params;
}

RichardsExcavGeom::RichardsExcavGeom(const InputParameters & parameters)
  : Function(parameters),
    _start_posn(getParam<RealVectorValue>("start_posn")),
    _start_time(getParam<Real>("start_time")),
    _end_posn(getParam<RealVectorValue>("end_posn")),
    _end_time(getParam<Real>("end_time")),
    _active_length(getParam<Real>("active_length")),
    _true_value(getParam<Real>("true_value")),
    _deactivation_time(getParam<Real>("deactivation_time")),
    _retreat_vel(_end_posn - _start_posn)
{
  if (_start_time >= _end_time)
    mooseError("Start time for excavation set to ",
               _start_time,
               " but this must be less than the end time, which is ",
               _end_time);
  _retreat_vel /= (_end_time - _start_time); // this is now a velocity
  _norm_retreat_vel = _retreat_vel.norm();
}

Real
RichardsExcavGeom::value(Real t, const Point & p) const
{
  if (t < _start_time || (p - _start_posn) * _retreat_vel < 0)
    // point is behind start posn - it'll never be active
    return 0.0;

  if (t >= _deactivation_time)
    return 0.0;

  RealVectorValue current_posn;
  if (t >= _end_time)
    current_posn = _end_posn;
  else
    current_posn = _start_posn + (t - _start_time) * _retreat_vel;

  Real distance_into_goaf = (current_posn - p) * _retreat_vel / _norm_retreat_vel;

  if (distance_into_goaf < 0)
    // point is ahead of current_posn
    return 0.0;

  if (distance_into_goaf > _active_length)
    // point is too far into goaf
    return 0.0;

  return _true_value;
}
