//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalRankTwoUserObjectBasePD.h"
#include "RankTwoTensor.h"
#include "AuxiliarySystem.h"

InputParameters
NodalRankTwoUserObjectBasePD::validParams()
{
  InputParameters params = NodalAuxVariableUserObjectBasePD::validParams();
  params.addClassDescription(
      "Base class for calculating components and scalar type quantities of nodal rank-two stress "
      "and strain tensors from material properties (stress and strain) for edge elements "
      "(i.e., bonds) connected at that node. NOTE: This UserObject only applies to the NOSPD "
      "model.");

  params.addRequiredParam<MaterialPropertyName>(
      "rank_two_tensor", "Name of the nodal rank two tensors (stress/strains)");

  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

  return params;
}

NodalRankTwoUserObjectBasePD::NodalRankTwoUserObjectBasePD(const InputParameters & parameters)
  : NodalAuxVariableUserObjectBasePD(parameters),
    _tensor(getMaterialProperty<RankTwoTensor>("rank_two_tensor"))
{
}

void
NodalRankTwoUserObjectBasePD::computeValue(unsigned int id, dof_id_type dof)
{
  dof_id_type id_j_in_i =
      _pdmesh.getNeighborIndex(_current_elem->node_id(id), _current_elem->node_id(1 - id));
  Real dg_vol_frac = _pdmesh.getHorizonSubsetVolumeFraction(_current_elem->node_id(id), id_j_in_i);

  // gather volume weighted contribution only if the bond is active
  if (_bond_status_var->getElementalValue(_current_elem) > 0.5)
    gatherWeightedValue(id, dof, dg_vol_frac);
}
