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
  params.addCoupledVar("fe_var", 1., "A coupled finite element variable.");
  params.addCoupledVar("fv_var", 1., "A coupled finite volume variable.");
  params.addParam<MaterialPropertyName>("declared_prop_name", "The name of the declared property.");
  params.addParam<MaterialPropertyName>("retrieved_prop_name",
                                        "The name of the retrieved property.");
  params.addParam<MaterialPropertyName>("fv_prop_name", "The name of the fv property");
  return params;
}

FEFVCouplingMaterial::FEFVCouplingMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _fe_var(getFunctor<MooseVariableFE<Real>>("fe_var", 0)),
    _fv_var(getFunctor<MooseVariableFV<Real>>("fv_var", 0)),
    _fe_prop(isCoupled("fe_var") ? &declareFunctorProperty<ADReal>("fe_prop") : nullptr),
    _fv_prop(isCoupled("fv_var")
                 ? &declareFunctorProperty<ADReal>(
                       isParamValid("fv_prop_name") ? getParam<MaterialPropertyName>("fv_prop_name")
                                                    : "fv_prop")
                 : nullptr),
    _declared_prop(
        isParamValid("declared_prop_name")
            ? &declareFunctorProperty<ADReal>(getParam<MaterialPropertyName>("declared_prop_name"))
            : nullptr),
    _retrieved_prop(isParamValid("retrieved_prop_name")
                        ? &getFunctorMaterialProperty<ADReal>("retrieved_prop_name")
                        : nullptr)
{
  if (_declared_prop)
    _declared_prop->setFunctor(
        _mesh, blockIDs(), [](const auto &, const auto &) -> ADReal { return 1.; });
  if (_fe_prop)
  {
    _fe_prop->setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
      return 1. + _fe_var(r, t);
    });
    _fe_prop->setCacheClearanceSchedule(
        std::set<ExecFlagType>(_execute_enum.begin(), _execute_enum.end()));
  }
  if (_fv_prop)
  {
    _fv_prop->setFunctor(_mesh, blockIDs(), [this](const auto & r, const auto & t) -> ADReal {
      auto ret = 1. + _fv_var(r, t);
      if (_retrieved_prop)
        ret *= (*_retrieved_prop)(r, t);
      return ret;
    });
    _fv_prop->setCacheClearanceSchedule(
        std::set<ExecFlagType>(_execute_enum.begin(), _execute_enum.end()));
  }
}
