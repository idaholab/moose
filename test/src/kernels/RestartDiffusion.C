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
#include "RestartDiffusion.h"

template <>
InputParameters
validParams<RestartDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addCustomTypeParam("coef", 0.0, "CoefficientType", "The coefficient of diffusion");
  return params;
}

RestartDiffusion::RestartDiffusion(const InputParameters & parameters)
  : Kernel(parameters),
    _coef(getParam<Real>("coef")),
    _current_coef(declareRestartableData<Real>("current_coef", 1))
{
}

void
RestartDiffusion::timestepSetup()
{
  _current_coef += 1;
}

Real
RestartDiffusion::computeQpResidual()
{
  return _current_coef * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
RestartDiffusion::computeQpJacobian()
{
  return _current_coef * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
