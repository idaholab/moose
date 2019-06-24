//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IntactBondsPresetBCPD.h"

registerMooseObject("PeridynamicsApp", IntactBondsPresetBCPD);

template <>
InputParameters
validParams<IntactBondsPresetBCPD>()
{
  InputParameters params = validParams<NodalBC>();
  params.addClassDescription(
      "Class to selectively apply a Dirichlet BC based on the number of intact "
      "bonds associated with each material point. Used to stabilize nodes without "
      "a sufficient number of connections to other material points.");

  return params;
}

IntactBondsPresetBCPD::IntactBondsPresetBCPD(const InputParameters & parameters)
  : PresetNodalBC(parameters),
    _pdmesh(dynamic_cast<PeridynamicsMesh &>(_mesh)),
    _u_old(_var.dofValuesOld()),
    _bond_status_var(_subproblem.getStandardVariable(_tid, "bond_status")),
    _max_intact_bonds(_pdmesh.dimension() - 1)
{
}

Real
IntactBondsPresetBCPD::computeQpValue()
{
  return _u_old[_qp];
}

bool
IntactBondsPresetBCPD::shouldApply()
{
  bool should_apply = false;

  unsigned int intact_bonds = 0;
  std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_node->id());
  for (unsigned int i = 0; i < bonds.size(); ++i)
    if (_bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[i])) > 0.5)
      intact_bonds++;

  if (intact_bonds <= _max_intact_bonds)
    should_apply = true;

  return should_apply;
}
