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

// Forward Declarations

/**
 * Defines excavation geometry.  It is used to enforce
 * pressures at the boundary of excavations (using RichardsExcav boundary condition)
 * to apply sink fluxes on these boundaries (using RichardsPiecewiseLinearSink boundary condition)
 * and to record fluid fluxes into the excavations (using RichardsExcavFlow and
 * RichardsPiecewiseLinearSinkFlux postprocessors).
 *
 * An excavation boundary evolves with time.
 * A RichardsExcavGeom object is used in conjunction with a static sideset to simulate this time
 * evolution.
 *
 * Define start_posn, and end_posn, which are Points.
 * Define start_time, end_time, and deactivation_time, which are Real numbers.
 * Define retreat_vel = (end_time - start_time)/(end_time - start_time)
 * For a given time t < end_time, define current_posn = start_posn + (t - start_time)*retreat_vel
 *                  t > end_time, define current_posn = end_posn
 * Then, the RichardsExcavGeom evaluated at time=t, position=p returns:
 * zero, if:
 *    t < start_time, or
 *    t >= deactivation_time
 *    p is behind start position, or
 *    p lies ahead of current position, or
 *    p lies greater than active_length behind current position
 * true_value, otherwise.
 */
class RichardsExcavGeom : public Function
{
public:
  static InputParameters validParams();

  RichardsExcavGeom(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const;

protected:
  /// start position
  RealVectorValue _start_posn;

  /// start time
  Real _start_time;

  /// end position
  RealVectorValue _end_posn;

  /// end time
  Real _end_time;

  /// active length
  Real _active_length;

  /// true value to return
  Real _true_value;

  /// deactivation time
  Real _deactivation_time;

  /// retreat velocity
  RealVectorValue _retreat_vel;

  /// norm of retreat velocity
  Real _norm_retreat_vel;
};
