//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalSideElementCounterWithID.h"

registerMooseObject("MooseTestApp", InternalSideElementCounterWithID);

InputParameters
InternalSideElementCounterWithID::validParams()
{
  InputParameters params = InternalSideVectorPostprocessor::validParams();
  params.addRequiredParam<std::vector<ExtraElementIDName>>("id_name", "The element integer name");
  params.addClassDescription("Count number of elements whose neighbor has a different ID (same "
                             "element can be counted multiple times with all its neighbors)");
  return params;
}

InternalSideElementCounterWithID::InternalSideElementCounterWithID(
    const InputParameters & parameters)
  : InternalSideVectorPostprocessor(parameters),
    _id_index(getElementIDIndex("id_name")),
    _current_id(getElementID("id_name")),
    _neighbor_id(getElementIDNeighbor("id_name")),
    _unique_ids(getAllElemIDs(_id_index)),
    _ids(declareVector("id")),
    _nelems(declareVector("nelem"))
{
  _ids.resize(_unique_ids.size());
  _nelems.resize(_unique_ids.size());
}

void
InternalSideElementCounterWithID::initialize()
{
  _counters.clear();
  for (auto & id : _unique_ids)
    _counters[id] = 0;
}

void
InternalSideElementCounterWithID::execute()
{
  if (_current_id != _neighbor_id)
  {
    ++_counters[_current_id];
    ++_counters[_neighbor_id];
  }
}

void
InternalSideElementCounterWithID::finalize()
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
InternalSideElementCounterWithID::threadJoin(const UserObject & y)
{
  const InternalSideElementCounterWithID & uo =
      static_cast<const InternalSideElementCounterWithID &>(y);
  for (auto & id : _unique_ids)
    _counters[id] += uo._counters.at(id);
}
