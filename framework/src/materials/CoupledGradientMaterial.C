//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledGradientMaterial.h"

registerMooseObject("MooseApp", CoupledGradientMaterial);
registerMooseObject("MooseApp", ADCoupledGradientMaterial);

template <bool is_ad>
InputParameters
CoupledGradientMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<MaterialPropertyName>(
      "mat_prop",
      "If provided, this will create a material property equal to the coupled variable.");
  params.addParam<MaterialPropertyName>("grad_mat_prop",
                                        "If provided, this will create a material property "
                                        "equal to the gradient of the coupled variable.");
  params.addParam<MaterialPropertyName>(
      "scalar_property",
      1.0,
      "Scalar material property multiplied by the coupled variable value and gradient.");
  params.addRequiredCoupledVar("u", "The coupled variable");
  return params;
}

template <bool is_ad>
CoupledGradientMaterialTempl<is_ad>::CoupledGradientMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _mat_prop(isParamValid("mat_prop") ? &declareGenericProperty<Real, is_ad>("mat_prop")
                                       : nullptr),
    _grad_mat_prop(isParamValid("grad_mat_prop")
                       ? &declareGenericProperty<RealVectorValue, is_ad>("grad_mat_prop")
                       : nullptr),
    _scalar_property(getGenericMaterialProperty<Real, is_ad>("scalar_property")),
    _u(coupledGenericValue<is_ad>("u")),
    _grad_u(coupledGenericGradient<is_ad>("u"))
{
}

template <bool is_ad>
void
CoupledGradientMaterialTempl<is_ad>::computeQpProperties()
{
  if (_mat_prop)
    (*_mat_prop)[_qp] = _u[_qp] * _scalar_property[_qp];
  if (_grad_mat_prop)
    (*_grad_mat_prop)[_qp] = _grad_u[_qp] * _scalar_property[_qp];
}
