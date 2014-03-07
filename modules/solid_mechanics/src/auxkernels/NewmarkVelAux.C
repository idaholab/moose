/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
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
