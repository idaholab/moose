//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericConstantRealVectorValue.h"

registerMooseObject("MooseApp", GenericConstantRealVectorValue);
registerMooseObject("MooseApp", ADGenericConstantRealVectorValue);

template <bool is_ad>
InputParameters
GenericConstantRealVectorValueTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Object for declaring a constant 3-vector as a material property.");
  params.addRequiredParam<RealVectorValue>("vector_values", "Values defining the constant vector");
  params.addRequiredParam<MaterialPropertyName>(
      "vector_name", "Name of the vector material property to be created");
  params.set<MooseEnum>("constant_on") = "SUBDOMAIN";
  return params;
}

template <bool is_ad>
GenericConstantRealVectorValueTempl<is_ad>::GenericConstantRealVectorValueTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _vector(getParam<RealVectorValue>("vector_values")),
    _prop(declareGenericProperty<RealVectorValue, is_ad>(
        getParam<MaterialPropertyName>("vector_name")))
{
}

template <bool is_ad>
void
GenericConstantRealVectorValueTempl<is_ad>::initQpStatefulProperties()
{
  GenericConstantRealVectorValueTempl<is_ad>::computeQpProperties();
}

template <bool is_ad>
void
GenericConstantRealVectorValueTempl<is_ad>::computeQpProperties()
{
  _prop[_qp] = _vector;
}

template class GenericConstantRealVectorValueTempl<false>;
template class GenericConstantRealVectorValueTempl<true>;
