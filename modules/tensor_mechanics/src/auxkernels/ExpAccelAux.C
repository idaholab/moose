/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ExpAccelAux.h"

template <>
InputParameters
validParams<ExpAccelAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("displacement", "displacement variable");
  return params;
}

ExpAccelAux::ExpAccelAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _disp_older(coupledValueOlder("displacement")),
    _disp_old(coupledValueOld("displacement")),
    _disp(coupledValue("displacement"))
{
}

Real
ExpAccelAux::computeValue()
{
  if (!isNodal())
    mooseError("must run on a nodal variable");

  // Calculates acceeleration using Newmark time integration method
  return (_disp[_qp] - _disp_old[_qp] * 2.0 + _disp_older[_qp]) / (_dt * _dt);
}
