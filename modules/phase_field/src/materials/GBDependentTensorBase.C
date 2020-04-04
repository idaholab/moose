//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GBDependentTensorBase.h"

InputParameters
GBDependentTensorBase::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("gb", "GB variable");
  params.addParam<Real>("bulk_parameter", 0.0, "Parameter value of bulk material");
  params.addParam<Real>("gb_parameter", 0.0, "Parameter value at GB");
  params.addParam<MaterialPropertyName>("gb_normal_tensor_name",
                                        "Name of GB normal tensor property");
  params.addParam<MaterialPropertyName>("gb_tensor_prop_name", "Name of GB tensor property");
  return params;
}

GBDependentTensorBase::GBDependentTensorBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _gb(coupledValue("gb")),
    _bulk_parameter(getParam<Real>("bulk_parameter")),
    _gb_parameter(getParam<Real>("gb_parameter")),
    _gb_normal_tensor(getMaterialProperty<RankTwoTensor>("gb_normal_tensor_name")),
    _gb_dependent_tensor(
        declareProperty<RealTensorValue>(getParam<MaterialPropertyName>("gb_tensor_prop_name")))
{
}
