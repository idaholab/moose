//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartDiffusion.h"

registerMooseObject("MooseTestApp", RestartDiffusion);

InputParameters
RestartDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addCustomTypeParam("coef", 0.0, "CoefficientType", "The coefficient of diffusion");
  return params;
}

RestartDiffusion::RestartDiffusion(const InputParameters & parameters)
  : Kernel(parameters),
    _coef(getParam<Real>("coef")),
    _current_coef(declareRestartableData<Real>("current_coef", 1)),
    _last_t_step(declareRecoverableData<int>("last_t_step", -1))
{
}

void
RestartDiffusion::timestepSetup()
{
  if (_last_t_step == _t_step)
    return;

  _current_coef += 1;

  _last_t_step = _t_step;
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
