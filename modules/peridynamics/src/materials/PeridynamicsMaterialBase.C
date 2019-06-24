//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeridynamicsMaterialBase.h"

template <>
InputParameters
validParams<PeridynamicsMaterialBase>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Base class for peridynamics material models");

  return params;
}

PeridynamicsMaterialBase::PeridynamicsMaterialBase(const InputParameters & parameters)
  : Material(parameters),
    _pdmesh(dynamic_cast<PeridynamicsMesh &>(_mesh)),
    _dim(_pdmesh.dimension()),
    _nnodes(2),
    _horiz_rad(_nnodes),
    _node_vol(_nnodes),
    _horiz_vol(_nnodes)
{
}

void
PeridynamicsMaterialBase::setupMeshRelatedData()
{
  for (unsigned int i = 0; i < _nnodes; ++i)
  {
    _horiz_rad[i] = _pdmesh.getHorizon(_current_elem->node_id(i));
    _node_vol[i] = _pdmesh.getPDNodeVolume(_current_elem->node_id(i));
    _horiz_vol[i] = _pdmesh.getHorizVolume(_current_elem->node_id(i));
  }

  _origin_vec = _pdmesh.getPDNodeCoord(_current_elem->node_id(1)) -
                _pdmesh.getPDNodeCoord(_current_elem->node_id(0));
  _origin_length = _origin_vec.norm();
}
