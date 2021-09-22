//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialTensorIntegral.h"
#include "RankTwoScalarTools.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("TensorMechanicsApp", MaterialTensorIntegral);
registerMooseObject("TensorMechanicsApp", ADMaterialTensorIntegral);

template <bool is_ad>
InputParameters
MaterialTensorIntegralTempl<is_ad>::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addClassDescription("This postprocessor computes an element integral of "
                             "a component of a material tensor as specified by "
                             "the user-supplied indices");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material property tensor name");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_i",
      "index_i >= 0 & index_i <= 2",
      "The index i of ij for the tensor to output (0, 1, 2)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_j",
      "index_j >= 0 & index_j <= 2",
      "The index j of ij for the tensor to output (0, 1, 2)");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

template <bool is_ad>
MaterialTensorIntegralTempl<is_ad>::MaterialTensorIntegralTempl(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _tensor(getGenericMaterialProperty<RankTwoTensor, is_ad>("rank_two_tensor")),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j"))
{
}

template <bool is_ad>
Real
MaterialTensorIntegralTempl<is_ad>::computeQpIntegral()
{
  return RankTwoScalarTools::component(MetaPhysicL::raw_value(_tensor[_qp]), _i, _j);
}

template class MaterialTensorIntegralTempl<false>;
template class MaterialTensorIntegralTempl<true>;
