//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RBMPresetOldValuePD.h"

registerMooseObject("PeridynamicsApp", RBMPresetOldValuePD);

InputParameters
RBMPresetOldValuePD::validParams()
{
  InputParameters params = DirichletBCBase::validParams();
  params.addClassDescription("Class to apply a preset BC to nodes with rigid body motion (RBM).");

  // Forcefully preset the BC
  params.set<bool>("preset") = true;
  params.suppressParameter<bool>("preset");

  return params;
}

RBMPresetOldValuePD::RBMPresetOldValuePD(const InputParameters & parameters)
  : DirichletBCBase(parameters),
    _pdmesh(dynamic_cast<PeridynamicsMesh &>(_mesh)),
    _u_old(_var.dofValuesOld()),
    _bond_status_var(&_subproblem.getStandardVariable(_tid, "bond_status"))
{
}

Real
RBMPresetOldValuePD::computeQpValue()
{
  return _u_old[_qp];
}

bool
RBMPresetOldValuePD::shouldApply()
{
  // check whether the number of active bonds is less than number of problem dimension
  unsigned int active_bonds = 0;
  std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_node->id());

  for (unsigned int nb = 0; nb < bonds.size(); ++nb)
    if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[nb])) > 0.5)
      active_bonds++;

  return (active_bonds < _pdmesh.dimension() + 1);
}
