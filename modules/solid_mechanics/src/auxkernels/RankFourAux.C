//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankFourAux.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("TensorMechanicsApp", RankFourAux);
registerMooseObject("TensorMechanicsApp", ADRankFourAux);

template <bool is_ad>
InputParameters
RankFourAuxTempl<is_ad>::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Access a component of a RankFourTensor");

  // add stuff here
  params.addRequiredParam<MaterialPropertyName>("rank_four_tensor",
                                                "The rank four material tensor name");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_i",
      "index_i >= 0 & index_i <= 2",
      "The index i of ijkl for the tensor to output (0, 1, 2)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_j",
      "index_j >= 0 & index_j <= 2",
      "The index j of ijkl for the tensor to output (0, 1, 2)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_k",
      "index_k >= 0 & index_k <= 2",
      "The index k of ijkl for the tensor to output (0, 1, 2)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_l",
      "index_l >= 0 & index_l <= 2",
      "The index l of ijkl for the tensor to output (0, 1, 2)");

  return params;
}

template <bool is_ad>
RankFourAuxTempl<is_ad>::RankFourAuxTempl(const InputParameters & parameters)
  : AuxKernel(parameters),
    _tensor(getGenericMaterialProperty<RankFourTensor, is_ad>("rank_four_tensor")),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j")),
    _k(getParam<unsigned int>("index_k")),
    _l(getParam<unsigned int>("index_l"))
{
}

template <bool is_ad>
Real
RankFourAuxTempl<is_ad>::computeValue()
{
  return MetaPhysicL::raw_value(_tensor[_qp](_i, _j, _k, _l));
}

template class RankFourAuxTempl<false>;
template class RankFourAuxTempl<true>;
