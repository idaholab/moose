/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ExpVelAux.h"

template <>
InputParameters
validParams<ExpVelAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("displacement", "displacement variable");
  return params;
}

ExpVelAux::ExpVelAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _disp_older(coupledValueOlder("displacement")),
    _disp_old(coupledValueOld("displacement")),
    _disp(coupledValue("displacement"))
{
}

Real
ExpVelAux::computeValue()
{
  if (!isNodal())
    mooseError("must run on a nodal variable");
  return 0.50 / _dt * (_disp[_qp] - _disp_older[_qp]);
}
