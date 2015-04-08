/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "NewmarkVelAux.h"

template<>
InputParameters validParams<NewmarkVelAux>()
{
  InputParameters params = validParams<AuxKernel>();
    params.addRequiredCoupledVar("acceleration","acceleration variable");
    //params.addRequiredParam<Real>("beta","beta parameter");
    params.addRequiredParam<Real>("gamma","gamma parameter");
  return params;
}

NewmarkVelAux::NewmarkVelAux(const std::string & name, InputParameters parameters) :
  AuxKernel(name, parameters),
   _accel_old(coupledValueOld("acceleration")),
   _accel(coupledValue("acceleration")),
   _gamma(getParam<Real>("gamma"))
{
}

Real
NewmarkVelAux::computeValue()
{
  Real vel_old = _u_old[_qp];
  if (!isNodal())
    mooseError("must run on a nodal variable");
  return vel_old + (_dt*(1-_gamma))*_accel_old[_qp] + _gamma*_dt*_accel[_qp];
}
