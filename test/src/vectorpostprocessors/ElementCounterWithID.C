//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementCounterWithID.h"

registerMooseObject("MooseTestApp", ElementCounterWithID);

InputParameters
ElementCounterWithID::validParams()
{
  InputParameters params = ElementVectorPostprocessor::validParams();
  params.addRequiredParam<std::vector<ExtraElementIDName>>("id_name", "The element integer name");
  params.addClassDescription(
      "Count number of elements on the subdomain where this VPP is defined based on their IDs");
  return params;
}

ElementCounterWithID::ElementCounterWithID(const InputParameters & parameters)
  : ElementVectorPostprocessor(parameters),
    _id_index(getElementIDIndex("id_name")),
    _current_id(getElementID("id_name")),
    _unique_ids(getElemIDsOnBlocks(_id_index, blockIDs())),
    _ids(declareVector("id")),
    _nelems(declareVector("nelem"))
{
  _ids.resize(_unique_ids.size());
  _nelems.resize(_unique_ids.size());
}

void
ElementCounterWithID::initialize()
{
  _counters.clear();
  for (auto & id : _unique_ids)
    _counters[id] = 0;
}

void
ElementCounterWithID::execute()
{
  if (_current_id != getElementID(_current_elem, _id_index))
    mooseError("Internal error on element ID handling");
  ++_counters[_current_id];
}

void
ElementCounterWithID::finalize()
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
ElementCounterWithID::threadJoin(const UserObject & y)
{
  const ElementCounterWithID & uo = static_cast<const ElementCounterWithID &>(y);
  for (auto & id : _unique_ids)
    _counters[id] += uo._counters.at(id);
}
