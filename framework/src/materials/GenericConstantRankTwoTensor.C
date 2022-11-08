//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericConstantRankTwoTensor.h"

registerMooseObject("MooseApp", GenericConstantRankTwoTensor);
registerMooseObject("MooseApp", ADGenericConstantRankTwoTensor);

template <bool is_ad>
InputParameters
GenericConstantRankTwoTensorTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Object for declaring a constant rank two tensor as a material property.");
  params.addRequiredParam<std::vector<Real>>(
      "tensor_values", "Vector of values defining the constant rank two tensor");
  params.addRequiredParam<MaterialPropertyName>(
      "tensor_name", "Name of the tensor material property to be created");
  params.set<MooseEnum>("constant_on") = "SUBDOMAIN";
  return params;
}

template <bool is_ad>
GenericConstantRankTwoTensorTempl<is_ad>::GenericConstantRankTwoTensorTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _prop(
        declareGenericProperty<RankTwoTensor, is_ad>(getParam<MaterialPropertyName>("tensor_name")))
{
  _tensor.fillFromInputVector(getParam<std::vector<Real>>("tensor_values"));
}

template <bool is_ad>
void
GenericConstantRankTwoTensorTempl<is_ad>::initQpStatefulProperties()
{
  GenericConstantRankTwoTensorTempl<is_ad>::computeQpProperties();
}

template <bool is_ad>
void
GenericConstantRankTwoTensorTempl<is_ad>::computeQpProperties()
{
  _prop[_qp] = _tensor;
}

template class GenericConstantRankTwoTensorTempl<false>;
template class GenericConstantRankTwoTensorTempl<true>;
