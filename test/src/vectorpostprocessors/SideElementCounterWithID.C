//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideElementCounterWithID.h"

registerMooseObject("MooseTestApp", SideElementCounterWithID);

InputParameters
SideElementCounterWithID::validParams()
{
  InputParameters params = SideVectorPostprocessor::validParams();
  params.addRequiredParam<std::vector<ExtraElementIDName>>("id_name", "The element integer name");
  params.addClassDescription("Count number of elements attached to the side sets where this VPP is "
                             "defined based on their IDs");
  return params;
}

SideElementCounterWithID::SideElementCounterWithID(const InputParameters & parameters)
  : SideVectorPostprocessor(parameters),
    _id_index(getElementIDIndex("id_name")),
    _current_id(getElementID("id_name")),
    _unique_ids(getAllElemIDs(_id_index)),
    _ids(declareVector("id")),
    _nelems(declareVector("nelem"))
{
  _ids.resize(_unique_ids.size());
  _nelems.resize(_unique_ids.size());
}

void
SideElementCounterWithID::initialize()
{
  _counters.clear();
  for (auto & id : _unique_ids)
    _counters[id] = 0;
}

void
SideElementCounterWithID::execute()
{
  ++_counters[_current_id];
}

void
SideElementCounterWithID::finalize()
{
  gatherSum(_counters);
  std::size_t i = 0;
  for (auto & id : _unique_ids)
  {
    _ids[i] = id;
    _nelems[i] = _counters[id];
    ++i;
  }
}

void
SideElementCounterWithID::threadJoin(const UserObject & y)
{
  const SideElementCounterWithID & uo = static_cast<const SideElementCounterWithID &>(y);
  for (auto & id : _unique_ids)
    _counters[id] += uo._counters.at(id);
}
