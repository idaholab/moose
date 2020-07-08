//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceElementCounterWithID.h"

registerMooseObject("MooseTestApp", InterfaceElementCounterWithID);

InputParameters
InterfaceElementCounterWithID::validParams()
{
  InputParameters params = InterfaceVectorPostprocessor::validParams();
  params.addRequiredParam<std::vector<ExtraElementIDName>>("id_name", "The element integer name");
  params.addClassDescription("Count number of elements attached to the side sets on both sides "
                             "where this VPP is defined based on their IDs");
  return params;
}

InterfaceElementCounterWithID::InterfaceElementCounterWithID(const InputParameters & parameters)
  : InterfaceVectorPostprocessor(parameters),
    _id_index(getElementIDIndex("id_name")),
    _current_id(getElementID("id_name")),
    _neighbor_id(getElementIDNeighbor("id_name")),
    _unique_ids(getAllElemIDs(_id_index)),
    _ids(declareVector("id")),
    _nelems(declareVector("nelem")),
    _neighbor_nelems(declareVector("neighbor_nelem"))
{
  _ids.resize(_unique_ids.size());
  _nelems.resize(_unique_ids.size());
  _neighbor_nelems.resize(_unique_ids.size());
}

void
InterfaceElementCounterWithID::initialize()
{
  _counters.clear();
  for (auto & id : _unique_ids)
  {
    _counters[id].first = 0;
    _counters[id].second = 0;
  }
}

void
InterfaceElementCounterWithID::execute()
{
  ++_counters[_current_id].first;
  ++_counters[_neighbor_id].second;
}

void
InterfaceElementCounterWithID::finalize()
{
  for (auto & pair : _counters)
  {
    gatherSum(pair.second.first);
    gatherSum(pair.second.second);
  }

  std::size_t i = 0;
  for (auto & id : _unique_ids)
  {
    _ids[i] = id;
    _nelems[i] = _counters[id].first;
    _neighbor_nelems[i] = _counters[id].second;
    ++i;
  }
}

void
InterfaceElementCounterWithID::threadJoin(const UserObject & y)
{
  const InterfaceElementCounterWithID & uo = static_cast<const InterfaceElementCounterWithID &>(y);
  for (auto & id : _unique_ids)
  {
    _counters[id].first += uo._counters.at(id).first;
    _counters[id].second += uo._counters.at(id).second;
  }
}
