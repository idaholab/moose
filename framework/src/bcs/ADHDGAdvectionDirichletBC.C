//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHDGAdvectionDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", ADHDGAdvectionDirichletBC);

InputParameters
ADHDGAdvectionDirichletBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addParam<Real>(
      "coeff", 1, "A constant coefficient. This could be something like a density");
  params.addParam<MaterialPropertyName>("velocity", "Velocity vector");
  params.addParam<FunctionName>("velocity_function", "Velocity vector function");
  params.addParam<FunctionName>("exact_soln", "The exact solution.");
  params.addParam<bool>("self_advection",
                        true,
                        "Whether this kernel should advect itself, e.g. it's "
                        "variable/side_variable pair. If false, we will advect "
                        "unity (possibly multiplied by the 'coeff' parameter");
  return params;
}

ADHDGAdvectionDirichletBC::ADHDGAdvectionDirichletBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _velocity(isParamValid("velocity") ? &getADMaterialProperty<RealVectorValue>("velocity")
                                       : nullptr),
    _velocity_func(isParamValid("velocity_function") ? &getFunction("velocity_function") : nullptr),
    _coeff(getParam<Real>("coeff")),
    _func(isParamValid("exact_soln") ? &getFunction("exact_soln") : nullptr)
{
  if (_func && !getParam<bool>("self_advection"))
    paramError("exact_soln",
               "If not advecting the variable/exact_soln pair as indicated by "
               "'self_advection=false', then 'exact_soln' should not supplied");
  else if (!_func && getParam<bool>("self_advection"))
    paramError("exact_soln",
               "When advecting a variable/exact_soln pair as indicated by 'self_advection=true' "
               "(the default), then 'exact_soln' should be supplied");
  if ((bool(_velocity) + bool(_velocity_func)) != 1)
    mooseError("Exactly one of 'velocity' and 'velocity_function' must be provided");
}

ADReal
ADHDGAdvectionDirichletBC::computeQpResidual()
{
  ADReal r = 0;
  const auto vel = _velocity ? (*_velocity)[_qp]
                             : ADRealVectorValue(_velocity_func->vectorValue(_t, _q_point[_qp]));
  const auto vdotn = vel * _normals[_qp];
  if (_func)
  {
    if (MetaPhysicL::raw_value(vdotn) >= 0)
      // outflow
      r = _u[_qp];
    else
      // inflow
      r = _func->value(_t, _q_point[_qp]);
  }
  else
    r = 1;

  r *= _coeff * _test[_i][_qp] * vdotn;
  return r;
}
