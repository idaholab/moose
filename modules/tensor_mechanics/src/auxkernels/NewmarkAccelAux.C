/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NewmarkAccelAux.h"

template <>
InputParameters
validParams<NewmarkAccelAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("displacement", "displacement variable");
  params.addRequiredCoupledVar("velocity", "velocity variable");
  params.addRequiredParam<Real>("beta", "beta parameter");
  return params;
}

NewmarkAccelAux::NewmarkAccelAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _disp_old(coupledValueOld("displacement")),
    _disp(coupledValue("displacement")),
    _vel_old(coupledValueOld("velocity")),
    _beta(getParam<Real>("beta"))
{
}

Real
NewmarkAccelAux::computeValue()
{
  if (!isNodal())
    mooseError("must run on a nodal variable");

  Real accel_old = _u_old[_qp];
  if (_dt == 0)
    return accel_old;

  // Calculates acceeleration using Newmark time integration method
  return 1.0 / _beta * ((_disp[_qp] - _disp_old[_qp]) / (_dt * _dt) - _vel_old[_qp] / _dt -
                        accel_old * (0.5 - _beta));
}
