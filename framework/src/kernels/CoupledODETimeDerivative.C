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

#include "CoupledODETimeDerivative.h"

template <>
InputParameters
validParams<CoupledODETimeDerivative>()
{
  InputParameters params = validParams<ODETimeKernel>();
  params.addRequiredCoupledVar("v", "Coupled variable.");
  return params;
}

CoupledODETimeDerivative::CoupledODETimeDerivative(const InputParameters & parameters)
  : ODETimeKernel(parameters), _v_dot(coupledScalarDot("v")), _dv_dot_dv(coupledScalarDotDu("v"))
{
}

Real
CoupledODETimeDerivative::computeQpResidual()
{
  return _v_dot[_i];
}

Real
CoupledODETimeDerivative::computeQpJacobian()
{
  if (_i == _j)
    return _dv_dot_dv[_i];
  else
    return 0;
}
