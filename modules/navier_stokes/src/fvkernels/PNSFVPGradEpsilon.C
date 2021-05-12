//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PNSFVPGradEpsilon.h"
#include "NS.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", PNSFVPGradEpsilon);

InputParameters
PNSFVPGradEpsilon::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Introduces a -p * grad_eps term.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addParam<FunctionName>("epsilon_function", "A function describing epsilon");
  params.addCoupledVar("epsilon_var", "An auxiliary variable holding the porosity");
  return params;
}

PNSFVPGradEpsilon::PNSFVPGradEpsilon(const InputParameters & params)
  : FVElementalKernel(params),
    _pressure(getADMaterialProperty<Real>(NS::pressure)),
    _eps_function(isParamValid("epsilon_function") ? &getFunction("epsilon_function") : nullptr),
    _eps_var_grad(isCoupled("epsilon_var") ? &adCoupledGradient("epsilon_var") : nullptr),
    _index(getParam<MooseEnum>("momentum_component"))
{
  if (!_eps_function && !_eps_var_grad)
    mooseError("Either 'epsilon_function' or 'epsilon_var' must be provided for ", name());
  if (_eps_function && _eps_var_grad)
    mooseError("Either 'epsilon_function' or 'epsilon_var', but not both, must be provided for ",
               name());
}

ADReal
PNSFVPGradEpsilon::computeQpResidual()
{
  if (_eps_function)
    return -_pressure[_qp] * _eps_function->gradient(_t, _q_point[_qp])(_index);
  else
    return -_pressure[_qp] * (*_eps_var_grad)[_qp](_index);
}
