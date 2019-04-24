//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalRankTwoTensorUserObjectBasePD.h"
#include "RankTwoTensor.h"
#include "AuxiliarySystem.h"

template <>
InputParameters
validParams<NodalRankTwoTensorUserObjectBasePD>()
{
  InputParameters params = validParams<NodalAuxVariableUserObjectBasePD>();
  params.addClassDescription(
      "Base class for calculating components and scalar type quantities of nodal rank-two stress "
      "and strain tensors from material properties (stress and strain) for edge elements "
      "(i.e., bonds) connected at that node. NOTE: This UserObject only applies to SNOSPD model.");

  params.addRequiredParam<MaterialPropertyName>(
      "rank_two_tensor", "Name of the nodal rank two tensors (stress/strains)");

  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

  return params;
}

NodalRankTwoTensorUserObjectBasePD::NodalRankTwoTensorUserObjectBasePD(
    const InputParameters & parameters)
  : NodalAuxVariableUserObjectBasePD(parameters),
    _tensor(getMaterialProperty<RankTwoTensor>("rank_two_tensor"))
{
}

void
NodalRankTwoTensorUserObjectBasePD::computeValue(unsigned int id, dof_id_type dof)
{
  unsigned int id_j_in_i =
      _pdmesh.getNeighborID(_current_elem->node_id(id), _current_elem->node_id(1 - id));
  Real dgb_vol_sum = _pdmesh.getBondAssocHorizonVolume(_current_elem->node_id(id), id_j_in_i);
  Real dgn_vol_sum = _pdmesh.getBondAssocHorizonVolumeSum(_current_elem->node_id(id));

  // gather volume weighted contribution
  if (_bond_status_var.getElementalValue(_current_elem) > 0.5)
    gatherWeightedValue(id, dof, dgb_vol_sum, dgn_vol_sum);
}
