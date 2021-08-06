//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FEFVCouplingMaterial.h"

registerMooseObject("MooseTestApp", FEFVCouplingMaterial);

InputParameters
FEFVCouplingMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params += SetupInterface::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_NONLINEAR};
  params.addParam<MooseFunctorName>("fe_var", 1., "A coupled finite element variable.");
  params.addParam<MooseFunctorName>("fv_var", 1., "A coupled finite volume variable.");
  params.addParam<MaterialPropertyName>("declared_prop_name", "The name of the declared property.");
  params.addParam<MooseFunctorName>("retrieved_prop_name", "The name of the retrieved property.");
  params.addParam<MaterialPropertyName>("fv_prop_name", "The name of the fv property");
  return params;
}

FEFVCouplingMaterial::FEFVCouplingMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _fe_var(getFunctor<ADReal>("fe_var")),
    _fv_var(getFunctor<ADReal>("fv_var")),
    _retrieved_prop(isParamValid("retrieved_prop_name") ? &getFunctor<ADReal>("retrieved_prop_name")
                                                        : nullptr)
{
  if (isParamValid("declared_prop_name"))
    addFunctorProperty<ADReal>(getParam<MaterialPropertyName>("declared_prop_name"),
                               [](const auto &, const auto &) -> ADReal { return 1.; },
                               {EXEC_TIMESTEP_BEGIN});

  if (isFunctor("fe_var"))
    addFunctorProperty<ADReal>(
        "fe_prop",
        [this](const auto & r, const auto & t) -> ADReal { return 1. + _fe_var(r, t); },
        std::set<ExecFlagType>(_execute_enum.begin(), _execute_enum.end()));

  if (isFunctor("fv_var"))
    addFunctorProperty<ADReal>(
        isParamValid("fv_prop_name") ? getParam<MaterialPropertyName>("fv_prop_name") : "fv_prop",
        [this](const auto & r, const auto & t) -> ADReal
        {
          auto ret = 1. + _fv_var(r, t);
          if (_retrieved_prop)
            ret *= (*_retrieved_prop)(r, t);
          return ret;
        },
        std::set<ExecFlagType>(_execute_enum.begin(), _execute_enum.end()));
}
