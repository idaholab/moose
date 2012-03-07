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

template<>
InputParameters validParams<ODETimeDerivative>()
{
  InputParameters params = validParams<ODEKernel>();
  return params;
}

ODETimeDerivative::ODETimeDerivative(const std::string & name, InputParameters parameters) :
    ODEKernel(name, parameters)
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
  return _du_dot_du[_i];
}
