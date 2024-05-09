//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialRankTwoTensorAux.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", MaterialRankTwoTensorAux);
registerMooseObject("MooseApp", ADMaterialRankTwoTensorAux);
registerMooseObject("MooseApp", MaterialSymmetricRankFourTensorAux);
registerMooseObject("MooseApp", ADMaterialSymmetricRankFourTensorAux);

template <typename T, bool is_ad>
InputParameters
MaterialRankTwoTensorAuxTempl<T, is_ad>::validParams()
{
  InputParameters params = MaterialAuxBaseTempl<T, is_ad>::validParams();
  params.addClassDescription(
      "Access a component of a RankTwoTensor for automatic material property output");
  params.addRequiredParam<unsigned int>("i", "The index i of ij for the tensor to output");
  params.addRequiredParam<unsigned int>("j", "The index j of ij for the tensor to output");
  return params;
}

template <typename T, bool is_ad>
MaterialRankTwoTensorAuxTempl<T, is_ad>::MaterialRankTwoTensorAuxTempl(
    const InputParameters & parameters)
  : MaterialAuxBaseTempl<T, is_ad>(parameters),
    _i(this->template getParam<unsigned int>("i")),
    _j(this->template getParam<unsigned int>("j"))
{
  mooseAssert(_i < T::N, "i component out of range.");
  mooseAssert(_j < T::N, "j component out of range.");
}

template <typename T, bool is_ad>
Real
MaterialRankTwoTensorAuxTempl<T, is_ad>::getRealValue()
{
  return MetaPhysicL::raw_value(this->_full_value(_i, _j));
}

template class MaterialRankTwoTensorAuxTempl<RankTwoTensor, false>;
template class MaterialRankTwoTensorAuxTempl<RankTwoTensor, true>;
template class MaterialRankTwoTensorAuxTempl<SymmetricRankFourTensor, false>;
template class MaterialRankTwoTensorAuxTempl<SymmetricRankFourTensor, true>;
