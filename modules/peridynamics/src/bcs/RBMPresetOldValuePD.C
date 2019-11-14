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

template <>
InputParameters
validParams<RBMPresetOldValuePD>()
{
  InputParameters params = validParams<NodalBC>();
  params.addClassDescription("Class to apply a preset BC to nodes with rigid body motion (RBM).");

  return params;
}

RBMPresetOldValuePD::RBMPresetOldValuePD(const InputParameters & parameters)
  : PresetNodalBC(parameters),
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
  bool should_apply = true;

  // check whether the node shape tensor is singular
  unsigned int dim = _pdmesh.dimension();
  Real horiz_size = _pdmesh.getHorizon(_current_node->id());
  std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_node->id());
  std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_node->id());

  RankTwoTensor shape_tensor;
  if (dim == 2)
    shape_tensor(2, 2) = 1.0;

  RealGradient ori_vec(dim);
  for (unsigned int i = 0; i < neighbors.size(); ++i)
    if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[i])) > 0.5)
    {
      Real vol_i = _pdmesh.getPDNodeVolume(neighbors[i]);
      ori_vec = *(_pdmesh.nodePtr(neighbors[i])) - *_current_node;
      for (unsigned int j = 0; j < dim; ++j)
        for (unsigned int k = 0; k < dim; ++k)
          shape_tensor(j, k) += horiz_size / ori_vec.norm() * ori_vec(j) * ori_vec(k) * vol_i;
    }

  if (!MooseUtils::absoluteFuzzyEqual(shape_tensor.det(), 0.0))
    should_apply = false;

  return should_apply;
}
