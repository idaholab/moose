/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GBDependentTensorBase.h"

template <>
InputParameters
validParams<GBDependentTensorBase>()
{
  InputParameters params = validParams<Material>();
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
