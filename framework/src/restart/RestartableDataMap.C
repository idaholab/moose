//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartableDataMap.h"

#include "DependencyResolver.h"

RestartableDataMap::RestartableDataMap() {}

RestartableDataValue &
RestartableDataMap::addData(std::unique_ptr<RestartableDataValue> data)
{
  mooseAssert(data, "Not set");
  mooseAssert(!hasData(data->name()), "Name is already added");

  const auto & name = data->name();
  auto & inserted_data = _data.addPointer(std::move(data), {});
  _name_to_data_index.emplace(name, _data.size() - 1);

  mooseAssert(hasData(name), "Doesn't have data");

  return inserted_data;
}

const RestartableDataValue *
RestartableDataMap::findData(const std::string & name) const
{
  auto find_index = _name_to_data_index.find(name);

#ifndef NDEBUG
  auto find_it = std::find_if(
      _data.begin(), _data.end(), [&name](const auto & data) { return data.name() == name; });
#endif

  if (find_index == _name_to_data_index.end())
  {
    mooseAssert(find_it == _data.end(), "Inconsistent map");
    return nullptr;
  }

  const auto index = find_index->second;
  mooseAssert(index == (std::size_t)std::distance(_data.begin(), find_it), "Inconsistent map");
  mooseAssert(_data.size() > index, "Invalid index");

  auto & data = _data[index];
  mooseAssert(data.name() == name, "Inconsistent name");

  return &data;
}

RestartableDataValue *
RestartableDataMap::findData(const std::string & name)
{
  return const_cast<RestartableDataValue *>(
      const_cast<const RestartableDataMap *>(this)->findData(name));
}

RestartableDataValue &
RestartableDataMap::data(const std::string & name)
{
  auto find_data = findData(name);
  if (!find_data)
    mooseError("Restartable data with the name ", name, " is not registered");
  return *find_data;
}

bool
RestartableDataMap::hasData(const std::string & name) const
{
  return findData(name) != nullptr;
}
