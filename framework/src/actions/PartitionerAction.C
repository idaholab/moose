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

registerMooseAction("MooseApp", PartitionerAction, "add_partitioner");

InputParameters
PartitionerAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Partitioner object to the simulation.");
  return params;
}

PartitionerAction::PartitionerAction(const InputParameters & params) : MooseObjectAction(params) {}

void
PartitionerAction::act()
{
  _mesh->setIsCustomPartitionerRequested(true);
  _moose_object_pars.set<MooseMesh *>("mesh") = _mesh.get();
  std::shared_ptr<MoosePartitioner> mp =
      _factory.create<MoosePartitioner>(_type, _name, _moose_object_pars);
  _mesh->setCustomPartitioner(mp.get());
}
