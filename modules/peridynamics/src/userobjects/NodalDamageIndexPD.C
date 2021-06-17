//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalDamageIndexPD.h"
#include "PeridynamicsMesh.h"
#include "AuxiliarySystem.h"

registerMooseObject("PeridynamicsApp", NodalDamageIndexPD);

InputParameters
NodalDamageIndexPD::validParams()
{
  InputParameters params = NodalAuxVariableUserObjectBasePD::validParams();
  params.addClassDescription("Class for computing damage index for each material point in "
                             "peridynamic fracture modeling and simulation");

  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};

  return params;
}

NodalDamageIndexPD::NodalDamageIndexPD(const InputParameters & parameters)
  : NodalAuxVariableUserObjectBasePD(parameters)
{
}

void
NodalDamageIndexPD::computeValue(unsigned int id, dof_id_type dof)
{
  Real neighbor_vol = _pdmesh.getNodeVolume(_current_elem->node_id(1 - id));
  Real node_vol_sum = _pdmesh.getHorizonVolume(_current_elem->node_id(id));

  if (_bond_status_var->getElementalValue(_current_elem) < 0.5)
    _aux.solution().add(dof, neighbor_vol / node_vol_sum);
}
