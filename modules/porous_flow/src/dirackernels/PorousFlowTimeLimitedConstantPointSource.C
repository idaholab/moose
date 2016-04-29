/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowTimeLimitedConstantPointSource.h"

template<>
InputParameters validParams<PorousFlowTimeLimitedConstantPointSource>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<Real>("mass_flux", "The mass flux at this point in kg/s (positive is flux in, negative is flux out)");
  params.addRequiredParam<Point>("point", "The x,y,z coordinates of the point");
  params.addParam<Real>("end_time", 1.0e30, "The time at which the source will end");
  return params;
}

PorousFlowTimeLimitedConstantPointSource::PorousFlowTimeLimitedConstantPointSource(const InputParameters & parameters) :
    DiracKernel(parameters),
    _mass_flux(getParam<Real>("mass_flux")),
    _p(getParam<Point>("point")),
    _end_time(getParam<Real>("end_time"))
{
}

void
PorousFlowTimeLimitedConstantPointSource::addPoints()
{
  addPoint(_p);
}

Real
PorousFlowTimeLimitedConstantPointSource::computeQpResidual()
{
  Real rate = 0.0;

  /**
   * If t - dt < end_time, then the Dirac point is still active.
   * In this case, if t <= end_time, then the rate is equal to mass_flux.
   * If t > end_time, then this time step is the one that exceeds
   * the specified end time. For this case, mass_flux is scaled so that then
   * total mass added (or removed) is correct (end_time * mass_flux).
   *
   * If t - dt > end_time, the Dirac point is made inactive by setting residual = 0
   */

  if (_t - _dt <= _end_time)
  {
    if (_t <= _end_time)
      rate = _mass_flux;
    else
      rate = _mass_flux * (1.0 - (_t - _end_time) / _dt);
  }

  return - _test[_i][_qp] * rate;
}
