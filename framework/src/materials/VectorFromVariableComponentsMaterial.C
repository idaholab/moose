//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorFromVariableComponentsMaterial.h"

registerMooseObject("MooseApp", VectorFromVariableComponentsMaterial);
registerMooseObject("MooseApp", ADVectorFromVariableComponentsMaterial);

template <bool is_ad>
InputParameters
VectorFromVariableComponentsMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes a vector material property from coupled variables");
  params.addRequiredParam<MaterialPropertyName>(
      "vector_prop_name", "The name to give the declared vector material property");
  params.addRequiredCoupledVar("u", "x-component");
  params.addCoupledVar("v", 0, "y-component");
  params.addCoupledVar("w", 0, "z-component");
  return params;
}

template <bool is_ad>
VectorFromVariableComponentsMaterialTempl<is_ad>::VectorFromVariableComponentsMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _vector(declareGenericProperty<RealVectorValue, is_ad>("vector_prop_name")),
    _u(coupledGenericValue<is_ad>("u")),
    _v(isCoupled("v") ? coupledGenericValue<is_ad>("v") : genericZeroValue<is_ad>()),
    _w(isCoupled("w") ? coupledGenericValue<is_ad>("w") : genericZeroValue<is_ad>())
{
}

template <bool is_ad>
void
VectorFromVariableComponentsMaterialTempl<is_ad>::computeQpProperties()
{
  _vector[_qp] = GenericRealVectorValue<is_ad>{_u[_qp], _v[_qp], _w[_qp]};
}

template class VectorFromVariableComponentsMaterialTempl<false>;
template class VectorFromVariableComponentsMaterialTempl<true>;
