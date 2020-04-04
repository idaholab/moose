//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoefDiffusion.h"

registerMooseObject("MiscApp", CoefDiffusion);

InputParameters
CoefDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<Real>("coef", 0.0, "Diffusion coefficient");
  params.addParam<FunctionName>("function",
                                "If provided, the diffusion coefficient will be coef + "
                                "this function.  This is useful for temporally or "
                                "spatially varying diffusivities");
  params.addClassDescription("Kernel for diffusion with diffusivity = coef + function");
  return params;
}

CoefDiffusion::CoefDiffusion(const InputParameters & parameters)
  : Kernel(parameters),
    _coef(getParam<Real>("coef")),
    _func(parameters.isParamValid("function") ? &getFunction("function") : NULL)
{
}

Real
CoefDiffusion::computeQpResidual()
{
  Real diffusivity = _coef;

  if (_func)
    diffusivity += _func->value(_t, _q_point[_qp]);

  return diffusivity * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
CoefDiffusion::computeQpJacobian()
{
  Real diffusivity = _coef;

  if (_func)
    diffusivity += _func->value(_t, _q_point[_qp]);

  return diffusivity * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
