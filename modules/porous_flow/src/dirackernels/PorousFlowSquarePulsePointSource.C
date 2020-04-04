//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowSquarePulsePointSource.h"

registerMooseObject("PorousFlowApp", PorousFlowSquarePulsePointSource);

InputParameters
PorousFlowSquarePulsePointSource::validParams()
{
  InputParameters params = DiracKernel::validParams();
  params.addRequiredParam<Real>(
      "mass_flux",
      "The mass flux at this point in kg/s (positive is flux in, negative is flux out)");
  params.addRequiredParam<Point>("point", "The x,y,z coordinates of the point source (sink)");
  params.addParam<Real>(
      "start_time", 0.0, "The time at which the source will start (Default is 0)");
  params.addParam<Real>(
      "end_time", 1.0e30, "The time at which the source will end (Default is 1e30)");
  params.addClassDescription("Point source (or sink) that adds (removes) fluid at a constant mass "
                             "flux rate for times between the specified start and end times.");
  return params;
}

PorousFlowSquarePulsePointSource::PorousFlowSquarePulsePointSource(
    const InputParameters & parameters)
  : DiracKernel(parameters),
    _mass_flux(getParam<Real>("mass_flux")),
    _p(getParam<Point>("point")),
    _start_time(getParam<Real>("start_time")),
    _end_time(getParam<Real>("end_time"))
{
  // Sanity check to ensure that the end_time is greater than the start_time
  if (_end_time <= _start_time)
    mooseError(name(),
               ": start time for PorousFlowSquarePulsePointSource is ",
               _start_time,
               " but it must be less than end time ",
               _end_time);
}

void
PorousFlowSquarePulsePointSource::addPoints()
{
  addPoint(_p, 0);
}

Real
PorousFlowSquarePulsePointSource::computeQpResidual()
{
  Real factor = 0.0;

  /**
   * There are six cases for the start and end time in relation to t-dt and t.
   * If the interval (t-dt,t) is only partly but not fully within the (start,end)
   * interval, then the  mass_flux is scaled so that the total mass added
   * (or removed) is correct
   */
  if (_t < _start_time || _t - _dt >= _end_time)
    factor = 0.0;
  else if (_t - _dt < _start_time)
  {
    if (_t <= _end_time)
      factor = (_t - _start_time) / _dt;
    else
      factor = (_end_time - _start_time) / _dt;
  }
  else
  {
    if (_t <= _end_time)
      factor = 1.0;
    else
      factor = (_end_time - (_t - _dt)) / _dt;
  }

  // Negative sign to make a positive mass_flux in the input file a source
  return -_test[_i][_qp] * factor * _mass_flux;
}
