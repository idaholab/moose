//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddDefaultSubchannelPartitioner.h"
#include "MooseMesh.h"
#include "Factory.h"
#include "MoosePartitioner.h"

registerMooseAction("SubChannelApp", AddDefaultSubchannelPartitioner, "add_partitioner");

InputParameters
AddDefaultSubchannelPartitioner::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Adds a default partitioner to subchannel simulation. Currently, a SingleRankPartitioner");
  return params;
}

AddDefaultSubchannelPartitioner::AddDefaultSubchannelPartitioner(const InputParameters & params)
  : Action(params)
{
}

void
AddDefaultSubchannelPartitioner::act()
{
  if (_current_task == "add_partitioner")
  {
    _mesh->setIsCustomPartitionerRequested(true);
    auto pars = _factory.getValidParams("SingleRankPartitioner");
    pars.set<MooseMesh *>("mesh") = _mesh.get();
    std::shared_ptr<MoosePartitioner> mp =
        _factory.create<MoosePartitioner>("SingleRankPartitioner", "SCM_partitioner", pars);
    _mesh->setCustomPartitioner(mp.get());
  }
}
