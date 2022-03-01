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

InputParameters
MaterialRankFourTensorAux::validParams()
{
  InputParameters params = MaterialAuxBase<>::validParams();
  params.addClassDescription(
      "Access a component of a RankFourTensor for automatic material property output");
  params.addRequiredParam<unsigned int>("i", "The index i of ijkl for the tensor to output");
  params.addRequiredParam<unsigned int>("j", "The index j of ijkl for the tensor to output");
  params.addRequiredParam<unsigned int>("k", "The index k of ijkl for the tensor to output");
  params.addRequiredParam<unsigned int>("l", "The index l of ijkl for the tensor to output");
  return params;
}

MaterialRankFourTensorAux::MaterialRankFourTensorAux(const InputParameters & parameters)
  : MaterialAuxBase<RankFourTensor>(parameters),
    _i(getParam<unsigned int>("i")),
    _j(getParam<unsigned int>("j")),
    _k(getParam<unsigned int>("k")),
    _l(getParam<unsigned int>("l"))
{
  mooseAssert(_i < LIBMESH_DIM, "i component out of range for current LIBMESH_DIM");
  mooseAssert(_j < LIBMESH_DIM, "j component out of range for current LIBMESH_DIM");
  mooseAssert(_k < LIBMESH_DIM, "k component out of range for current LIBMESH_DIM");
  mooseAssert(_l < LIBMESH_DIM, "l component out of range for current LIBMESH_DIM");
}

Real
MaterialRankFourTensorAux::getRealValue()
{
  return _prop[_qp](_i, _j, _k, _l);
}
