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

#include "TimeDerivative.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<TimeDerivative>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

TimeDerivative::TimeDerivative(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _time_weight(_moose_system._time_weight)
{}

Real
TimeDerivative::computeQpResidual()
{
  return _test[_i][_qp]*_u_dot[_qp];
}

Real
TimeDerivative::computeQpJacobian()
{
  return _test[_i][_qp]*_time_weight[0]*_phi[_j][_qp]/_dt;
}
