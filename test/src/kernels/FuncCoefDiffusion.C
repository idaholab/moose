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
#include "FuncCoefDiffusion.h"

template <>
InputParameters
validParams<FuncCoefDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<FunctionName>("coef", "0.5*x+0.5*y", "The function for conductivity");
  return params;
}

FuncCoefDiffusion::FuncCoefDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _function(getFunction("coef"))
{
}

Real
FuncCoefDiffusion::computeQpResidual()
{
  Real k = _function.value(_t, _qp);
  return k * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
FuncCoefDiffusion::computeQpJacobian()
{
  Real k = _function.value(_t, _qp);
  return k * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
