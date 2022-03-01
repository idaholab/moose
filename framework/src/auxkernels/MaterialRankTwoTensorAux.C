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

template <bool is_ad>
InputParameters
MaterialRankTwoTensorAuxTempl<is_ad>::validParams()
{
  InputParameters params = MaterialAuxBaseTempl<RankTwoTensor, is_ad>::validParams();
  params.addClassDescription(
      "Access a component of a RankTwoTensor for automatic material property output");
  params.addRequiredParam<unsigned int>("i", "The index i of ij for the tensor to output");
  params.addRequiredParam<unsigned int>("j", "The index j of ij for the tensor to output");
  return params;
}

template <bool is_ad>
MaterialRankTwoTensorAuxTempl<is_ad>::MaterialRankTwoTensorAuxTempl(
    const InputParameters & parameters)
  : MaterialAuxBaseTempl<RankTwoTensor, is_ad>(parameters),
    _i(this->template getParam<unsigned int>("i")),
    _j(this->template getParam<unsigned int>("j"))
{
  mooseAssert(_i < LIBMESH_DIM, "i component out of range for current LIBMESH_DIM");
  mooseAssert(_j < LIBMESH_DIM, "j component out of range for current LIBMESH_DIM");
}

template <bool is_ad>
Real
MaterialRankTwoTensorAuxTempl<is_ad>::getRealValue()
{
  return MetaPhysicL::raw_value(this->_prop[this->_qp](_i, _j));
}
