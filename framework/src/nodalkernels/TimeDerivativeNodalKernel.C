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

#include "TimeDerivativeNodalKernel.h"

template<>
InputParameters validParams<TimeDerivativeNodalKernel>()
{
  InputParameters params = validParams<NodalKernel>();
  return params;
}

TimeDerivativeNodalKernel::TimeDerivativeNodalKernel(const InputParameters & parameters) :
    NodalKernel(parameters)
{
}

Real
TimeDerivativeNodalKernel::computeQpResidual()
{
  return _u_dot[_qp];
}

Real
TimeDerivativeNodalKernel::computeQpJacobian()
{
  return _du_dot_du[_qp];
}
