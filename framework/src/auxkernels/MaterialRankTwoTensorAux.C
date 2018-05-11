//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialRankTwoTensorAux.h"

registerMooseObject("MooseApp", MaterialRankTwoTensorAux);

template <>
InputParameters
validParams<MaterialRankTwoTensorAux>()
{
  InputParameters params = validParams<MaterialAuxBase<>>();
  params.addClassDescription(
      "Access a component of a RankTwoTensor for automatic material property output");
  params.addRequiredParam<unsigned int>("i", "The index i of ij for the tensor to output");
  params.addRequiredParam<unsigned int>("j", "The index j of ij for the tensor to output");
  return params;
}

MaterialRankTwoTensorAux::MaterialRankTwoTensorAux(const InputParameters & parameters)
  : MaterialAuxBase<RankTwoTensor>(parameters),
    _i(getParam<unsigned int>("i")),
    _j(getParam<unsigned int>("j"))
{
  mooseAssert(_i < LIBMESH_DIM, "i component out of range for current LIBMESH_DIM");
  mooseAssert(_j < LIBMESH_DIM, "j component out of range for current LIBMESH_DIM");
}

Real
MaterialRankTwoTensorAux::getRealValue()
{
  return _prop[_qp](_i, _j);
}
