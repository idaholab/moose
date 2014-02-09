/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include "RichardsExcavGeom.h"

template<>
InputParameters validParams<RichardsExcavGeom>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<RealVectorValue>("start_posn", "Start point of the excavation.  This is an (x,y,z) point in the middle of the coal face at the very beginning of the panel.");
  params.addRequiredParam<Real>("start_time", "Commencement time of the excavation");
  params.addRequiredParam<RealVectorValue>("end_posn", "End position of the excavation.  This is an (x,y,z) point in the middle of the coal face at the very end of the panel.");
  params.addRequiredParam<Real>("end_time", "Time at the completion of the excavation");
  params.addClassDescription("This function defines excavation geometry.  It can be used to enforce pressures at the boundary of excavations, and to record fluid fluxes into excavations.");
  return params;
}

RichardsExcavGeom::RichardsExcavGeom(const std::string & name, InputParameters parameters) :
    Function(name, parameters),
    _start_posn(getParam<RealVectorValue>("start_posn")),
    _start_time(getParam<Real>("start_time")),
    _end_posn(getParam<RealVectorValue>("end_posn")),
    _end_time(getParam<Real>("end_time")),
    _retreat_vel(_end_posn - _start_posn)
{
  if (_start_time >= _end_time)
    mooseError("Start time for excavation set to " << _start_time << " but this must be less than the end time, which is " << _end_time);
  _retreat_vel /= (_end_time - _start_time); // this is now a velocity
}


/// this returns the effective saturation as a function of
/// pressure.  Input pressure in the first slot (t).
/// This uses the van Genuchten expression.
Real
RichardsExcavGeom::value(Real t, const Point & p)
{
  if (t < _start_time || (p - _start_posn)*_retreat_vel < 0)
    // point is behind start posn - it'll never be active
    {
      return 0.0;
    }

  RealVectorValue current_posn;
  if (t >= _end_time) {
    current_posn = _end_posn;
  }
  else {
    current_posn = _start_posn + (t - _start_time)*_retreat_vel;
  }

  if ((p - current_posn)*_retreat_vel > 0)
    // point is ahead of current_posn
    {
      return 0.0;
    }

  return 1.0;
}

