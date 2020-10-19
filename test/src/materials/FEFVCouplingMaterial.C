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
  InputParameters params = Material::validParams();
  params.addCoupledVar("fe_var", 1., "A coupled finite element variable.");
  params.addCoupledVar("fv_var", 1., "A coupled finite volume variable.");
  params.addParam<MaterialPropertyName>("declared_prop_name", "The name of the declared property.");
  params.addParam<MaterialPropertyName>("retrieved_prop_name",
                                        "The name of the retrieved property.");
  return params;
}

FEFVCouplingMaterial::FEFVCouplingMaterial(const InputParameters & parameters)
  : Material(parameters),
    _fe_var(adCoupledValue("fe_var")),
    _fv_var(adCoupledValue("fv_var")),
    _fe_prop(isCoupled("fe_var") ? &declareADProperty<Real>("fe_prop") : nullptr),
    _fv_prop(isCoupled("fv_var") ? &declareADProperty<Real>("fv_prop") : nullptr),
    _declared_prop(
        isParamValid("declared_prop_name")
            ? &declareADProperty<Real>(getParam<MaterialPropertyName>("declared_prop_name"))
            : nullptr),
    _retrieved_prop(isParamValid("retrieved_prop_name")
                        ? &getADMaterialProperty<Real>("retrieved_prop_name")
                        : nullptr)
{
}

void
FEFVCouplingMaterial::computeQpProperties()
{
  if (_declared_prop)
    (*_declared_prop)[_qp] = 1.;
  if (_fe_prop)
    (*_fe_prop)[_qp] = 1. + _fe_var[_qp];
  if (_fv_prop)
  {
    (*_fv_prop)[_qp] = 1. + _fv_var[_qp];
    if (_retrieved_prop)
      (*_fv_prop)[_qp] *= (*_retrieved_prop)[_qp];
  }
}
