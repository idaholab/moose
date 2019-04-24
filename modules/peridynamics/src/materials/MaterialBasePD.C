//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialBasePD.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<MaterialBasePD>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Base class for material classes for Peridynamic models");

  return params;
}

MaterialBasePD::MaterialBasePD(const InputParameters & parameters)
  : Material(parameters),
    _pdmesh(dynamic_cast<PeridynamicsMesh &>(_mesh)),
    _dim(_pdmesh.dimension()),
    _nnodes(2),
    _horizon(_nnodes),
    _nv(_nnodes),
    _nvsum(_nnodes)
{
}

void
MaterialBasePD::computeProperties()
{
  if (_qrule->n_points() != _nnodes)
    mooseError("In current implementation, the number of quadrature points should be the same as "
               "number of element nodes (i.e., equal to 2)!");

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _horizon[_qp] = _pdmesh.getHorizon(_current_elem->node_id(_qp));
    _nv[_qp] = _pdmesh.getVolume(_current_elem->node_id(_qp));
    _nvsum[_qp] = _pdmesh.getHorizonVolume(_current_elem->node_id(_qp));
  }

  _origin_vec = *_current_elem->node_ptr(1) - *_current_elem->node_ptr(0);
  _origin_length = _origin_vec.norm();
}
