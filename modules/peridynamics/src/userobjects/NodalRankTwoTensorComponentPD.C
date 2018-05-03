//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalRankTwoTensorComponentPD.h"
#include "RankTwoTensor.h"

registerMooseObject("PeridynamicsApp", NodalRankTwoTensorComponentPD);

template <>
InputParameters
validParams<NodalRankTwoTensorComponentPD>()
{
  InputParameters params = validParams<NodalRankTwoTensorUserObjectBasePD>();
  params.addClassDescription(
      "Class for calculating components of nodal rank-two stress and strain tensors "
      "from material properties (stress and strain) for edge elements (i.e., "
      "bonds) connected at that node. NOTE: This UserObject only applies to SNOSPD model.");

  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_i",
      "index_i >= 0 & index_i <= 2",
      "The index i of ij for the tensor to output (0, 1, 2)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_j",
      "index_j >= 0 & index_j <= 2",
      "The index j of ij for the tensor to output (0, 1, 2)");

  return params;
}

NodalRankTwoTensorComponentPD::NodalRankTwoTensorComponentPD(const InputParameters & parameters)
  : NodalRankTwoTensorUserObjectBasePD(parameters),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j"))
{
}

void
NodalRankTwoTensorComponentPD::gatherWeightedValue(unsigned int id,
                                                   dof_id_type dof,
                                                   Real dgb_vol_sum,
                                                   Real dgn_vol_sum)
{
  _aux_sln.add(dof, _tensor[id](_i, _j) * dgb_vol_sum / dgn_vol_sum);
}
