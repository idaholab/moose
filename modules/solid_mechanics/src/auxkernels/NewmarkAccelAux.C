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

#include "NewmarkAccelAux.h"

template<>
InputParameters validParams<NewmarkAccelAux>()
{
  InputParameters params = validParams<AuxKernel>();
    params.addRequiredCoupledVar("displacement","displacement variable");
    params.addRequiredCoupledVar("velocity","velocity variable");
    params.addRequiredParam<Real>("beta","beta parameter");
    //params.addRequiredParam<Real>("gamma","gamma parameter");
  return params;
}

NewmarkAccelAux::NewmarkAccelAux(const std::string & name, InputParameters parameters) :
  AuxKernel(name, parameters),
   _disp_old(coupledValueOld("displacement")),
   _disp(coupledValue("displacement")),
   _vel_old(coupledValueOld("velocity")),
   _beta(getParam<Real>("beta"))
{
}

Real
NewmarkAccelAux::computeValue()
{
  Real accel_old = _u_old[_qp];
  if (!isNodal())
    mooseError("must run on a nodal variable");
  return 1/_beta*(((_disp[_qp]-_disp_old[_qp])/(_dt*_dt)) - _vel_old[_qp]/_dt - accel_old*(0.5-_beta));
}
