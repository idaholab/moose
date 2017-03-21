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

#include "ODETimeDerivative.h"

template <>
InputParameters
validParams<ODETimeDerivative>()
{
  InputParameters params = validParams<ODETimeKernel>();
  return params;
}

ODETimeDerivative::ODETimeDerivative(const InputParameters & parameters) : ODETimeKernel(parameters)
{
}

Real
ODETimeDerivative::computeQpResidual()
{
  return _u_dot[_i];
}

Real
ODETimeDerivative::computeQpJacobian()
{
  if (_i == _j)
    return _du_dot_du[_i];
  else
    return 0;
}
