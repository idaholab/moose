//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "PartitionerAction.h"
#include "MoosePartitioner.h"
#include "FEProblem.h"
#include "MooseEnum.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<PartitionerAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

PartitionerAction::PartitionerAction(InputParameters params) : MooseObjectAction(params) {}

void
PartitionerAction::act()
{
  _mesh->setIsCustomPartitionerRequested(true);
  _moose_object_pars.set<MooseMesh *>("mesh") = _mesh.get();
  std::shared_ptr<MoosePartitioner> mp =
      _factory.create<MoosePartitioner>(_type, _name, _moose_object_pars);
  _mesh->setCustomPartitioner(mp.get());
  if (_displaced_mesh)
  {
    _displaced_mesh->setIsCustomPartitionerRequested(true);
    _displaced_mesh->setCustomPartitioner(mp.get());
  }
}
