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
  params.addClassDescription("Creates a gradient material equal to the gradient of the coupled "
                             "variable gradient times a scalar material property.");
  params.addRequiredParam<MaterialPropertyName>(
      "grad_mat_prop",
      "Name of gradient material property equal to the gradient of the coupled variable gradient "
      "times the scalar.");
  params.deprecateParam("grad_mat_prop", "gradient_material_name", "12/12/25");
  params.addParam<MaterialPropertyName>(
      "scalar_property_factor",
      1.0,
      "Scalar material property acting as a factor in the output gradient material property.");
  params.addRequiredCoupledVar("u", "The coupled variable to take the gradient of");
  params.deprecateCoupledVar("u", "coupled_variable", "12/12/25");
  return params;
}

template <bool is_ad>
CoupledGradientMaterialTempl<is_ad>::CoupledGradientMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _grad_mat_prop(declareGenericProperty<RealVectorValue, is_ad>("grad_mat_prop")),
    _scalar_property_factor(getGenericMaterialProperty<Real, is_ad>("scalar_property_factor")),
    _grad_u(coupledGenericGradient<is_ad>("u"))
{
}

template <bool is_ad>
void
CoupledGradientMaterialTempl<is_ad>::computeQpProperties()
{
  _grad_mat_prop[_qp] = _grad_u[_qp] * _scalar_property_factor[_qp];
}
