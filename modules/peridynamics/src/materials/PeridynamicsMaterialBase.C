//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeridynamicsMaterialBase.h"

InputParameters
PeridynamicsMaterialBase::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Base class for peridynamics material models");

  return params;
}

PeridynamicsMaterialBase::PeridynamicsMaterialBase(const InputParameters & parameters)
  : Material(parameters),
    _pdmesh(dynamic_cast<PeridynamicsMesh &>(_mesh)),
    _dim(_pdmesh.dimension()),
    _nnodes(2),
    _horizon_radius(_nnodes),
    _node_vol(_nnodes),
    _horizon_vol(_nnodes)
{
}

void
PeridynamicsMaterialBase::setupMeshRelatedData()
{
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    _horizon_radius[nd] = _pdmesh.getHorizon(_current_elem->node_id(nd));
    _node_vol[nd] = _pdmesh.getNodeVolume(_current_elem->node_id(nd));
    _horizon_vol[nd] = _pdmesh.getHorizonVolume(_current_elem->node_id(nd));
  }

  _origin_vec = _pdmesh.getNodeCoord(_current_elem->node_id(1)) -
                _pdmesh.getNodeCoord(_current_elem->node_id(0));
}
