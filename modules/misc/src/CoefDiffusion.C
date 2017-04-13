/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CoefDiffusion.h"

template <>
InputParameters
validParams<CoefDiffusion>()
{
  InputParameters params = validParams<Kernel>();
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
