/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowSquarePulsePointSource.h"

template<>
InputParameters validParams<PorousFlowSquarePulsePointSource>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<Real>("mass_flux", "The mass flux at this point in kg/s (positive is flux in, negative is flux out)");
  params.addRequiredParam<std::vector<Real> >("point", "The x,y,z coordinates of the point");
  params.addParam<Real>("start_time", 0.0, "The time at which the source will start");
  params.addParam<Real>("end_time", 1.0e30, "The time at which the source will end");
  return params;
}

PorousFlowSquarePulsePointSource::PorousFlowSquarePulsePointSource(const InputParameters & parameters) :
    DiracKernel(parameters),
    _mass_flux(getParam<Real>("mass_flux")),
    _point_param(getParam<std::vector<Real> >("point")),
    _start_time(getParam<Real>("start_time")),
    _end_time(getParam<Real>("end_time"))
{
  _p(0) = _point_param[0];

  if (_point_param.size() > 1)
    _p(1) = _point_param[1];
  if (_point_param.size() > 2)
      _p(2) = _point_param[2];
}

void
PorousFlowSquarePulsePointSource::addPoints()
{
  addPoint(_p);
}

Real
PorousFlowSquarePulsePointSource::computeQpResidual()
{
  Real factor=.0, rate = 0.;

  /**
   * There are six cases for the start and end time in relation to t-dt and t.
   * If the interval (t-dt,t) is only partly but not fully within the (start,end)
   * interval, then the  mass_flux is scaled so that
   *  the total mass added (or removed) is correct
   */

  if ((_t <= _start_time) || (_t - _dt >= _end_time))
    factor = 0;
  else if (_t - _dt >= _start_time)
    {
      if (_t <= _end_time)
        factor = 1.0;
      else
        factor = (_end_time - (_t - _dt)) / _dt ;
    }
  else
    {
      if (_t <= _end_time)
        factor = (_t - _start_time) / _dt ;
      else
        factor = (_end_time - _start_time) / _dt ;
    }

  if (factor < 0.0 || factor > 1.0)
    mooseError("Internal rate factor is " << factor << " but should be between 0 and 1.");

  if (_end_time <= _start_time)
    mooseError("Start time for source is " << _start_time << " but should be less than end time " << _end_time);

  rate = factor * _mass_flux;

  return - _test[_i][_qp] * rate;
}
