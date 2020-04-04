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
  bool should_apply = true;

  // check whether the node shape tensor is singular
  unsigned int dim = _pdmesh.dimension();
  Real horiz_size = _pdmesh.getHorizon(_current_node->id());
  std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_node->id());
  std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_node->id());

  RankTwoTensor shape;
  if (dim == 2)
    shape(2, 2) = 1.0;

  Real vol_nb;
  RealGradient origin_vec;

  for (unsigned int nb = 0; nb < neighbors.size(); ++nb)
    if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[nb])) > 0.5)
    {
      vol_nb = _pdmesh.getPDNodeVolume(neighbors[nb]);
      origin_vec = *(_pdmesh.nodePtr(neighbors[nb])) - *_current_node;

      for (unsigned int k = 0; k < dim; ++k)
        for (unsigned int l = 0; l < dim; ++l)
          shape(k, l) += horiz_size / origin_vec.norm() * origin_vec(k) * origin_vec(l) * vol_nb;
    }

  if (!MooseUtils::absoluteFuzzyEqual(shape.det(), 0.0))
    should_apply = false;

  return should_apply;
}
