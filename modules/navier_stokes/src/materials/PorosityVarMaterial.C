//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "PorosityVarMaterial.h"
#include "NS.h"
#include "MooseVariableFieldBase.h"
#include "Function.h"
#include "metaphysicl/raw_type.h"

registerMooseObject("NavierStokesApp", PorosityVarMaterial);

InputParameters
PorosityVarMaterial::validParams()
{
  auto params = Material::validParams();
  params.addCoupledVar(NS::porosity + "_var",
                       "the porosity varaible. This should be an *auxiliary* variable!");
  params.addParam<FunctionName>(NS::porosity + "_function", "A porosity function");
  return params;
}

PorosityVarMaterial::PorosityVarMaterial(const InputParameters & params)
  : Material(params),
    _eps_function(isParamValid(NS::porosity + "_function")
                      ? &getFunction(NS::porosity + "_function")
                      : nullptr),
    _eps_var_value(isCoupled(NS::porosity + "_var") ? &adCoupledValue(NS::porosity + "_var")
                                                    : nullptr),
    _eps_var_grad(isCoupled(NS::porosity + "_var") ? &adCoupledGradient(NS::porosity + "_var")
                                                   : nullptr),
    _eps(declareProperty<Real>(NS::porosity)),
    _eps_grad(declareProperty<RealVectorValue>(NS::grad(NS::porosity)))
{
  if (!_eps_function && !_eps_var_value)
    mooseError("Either '" + NS::porosity + "_function' or '" + NS::porosity +
                   "_var' must be provided for ",
               name());
  if (_eps_function && _eps_var_value)
    mooseError("Either '" + NS::porosity + "_function' or '" + NS::porosity +
                   "_var', but not both, must be provided for ",
               name());

  if (_eps_var_value)
  {
    const auto & eps_var = *getFieldVar(NS::porosity + "_var", 0);
    if (eps_var.kind() != Moose::VarKindType::VAR_AUXILIARY)
      paramError(NS::porosity + "_var", "The porosity variable must be an auxiliary variable");
  }
}

void
PorosityVarMaterial::computeQpProperties()
{
  if (_eps_var_value)
  {
    _eps[_qp] = MetaPhysicL::raw_value((*_eps_var_value)[_qp]);
    _eps_grad[_qp] = MetaPhysicL::raw_value((*_eps_var_grad)[_qp]);
  }
  else
  {
    _eps[_qp] = _eps_function->value(_t, _q_point[_qp]);
    _eps_grad[_qp] = _eps_function->gradient(_t, _q_point[_qp]);
  }
}
