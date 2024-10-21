//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialRankFourTensorAux.h"

registerMooseObject("MooseApp", MaterialRankFourTensorAux);
registerMooseObject("MooseApp", ADMaterialRankFourTensorAux);

template <bool is_ad>
InputParameters
MaterialRankFourTensorAuxTempl<is_ad>::validParams()
{
  InputParameters params = MaterialAuxBaseTempl<RankFourTensor, is_ad>::validParams();
  params.addClassDescription(
      "Access a component of a RankFourTensor for automatic material property output");
  params.addRequiredParam<unsigned int>("i", "The index i of ijkl for the tensor to output");
  params.addRequiredParam<unsigned int>("j", "The index j of ijkl for the tensor to output");
  params.addRequiredParam<unsigned int>("k", "The index k of ijkl for the tensor to output");
  params.addRequiredParam<unsigned int>("l", "The index l of ijkl for the tensor to output");
  return params;
}

template <bool is_ad>
MaterialRankFourTensorAuxTempl<is_ad>::MaterialRankFourTensorAuxTempl(
    const InputParameters & parameters)
  : MaterialAuxBaseTempl<RankFourTensor, is_ad>(parameters),
    _i(this->template getParam<unsigned int>("i")),
    _j(this->template getParam<unsigned int>("j")),
    _k(this->template getParam<unsigned int>("k")),
    _l(this->template getParam<unsigned int>("l"))
{
  mooseAssert(_i < LIBMESH_DIM, "i component out of range for current LIBMESH_DIM");
  mooseAssert(_j < LIBMESH_DIM, "j component out of range for current LIBMESH_DIM");
  mooseAssert(_k < LIBMESH_DIM, "k component out of range for current LIBMESH_DIM");
  mooseAssert(_l < LIBMESH_DIM, "l component out of range for current LIBMESH_DIM");
}

template <bool is_ad>
Real
MaterialRankFourTensorAuxTempl<is_ad>::getRealValue()
{
  return MetaPhysicL::raw_value(this->_full_value(_i, _j, _k, _l));
}
