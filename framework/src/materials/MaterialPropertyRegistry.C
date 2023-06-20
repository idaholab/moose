//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialPropertyRegistry.h"

#include "MooseError.h"

unsigned int
MaterialPropertyRegistry::addOrGetID(const std::string & name,
                                     const MaterialPropertyRegistry::WriteKey)
{
  const auto it = _name_to_id.find(name);
  if (it != _name_to_id.end())
    return it->second;

  const auto id = _id_to_name.size();
  _name_to_id.emplace(name, id);
  _id_to_name.push_back(name);
  return id;
}

unsigned int
MaterialPropertyRegistry::getID(const std::string & name) const
{
  const auto id = queryID(name);
  if (!id)
    mooseError("MaterialPropertyRegistry: Property '" + name + "' is not declared");
  return *id;
}

std::optional<unsigned int>
MaterialPropertyRegistry::queryID(const std::string & name) const
{
  const auto it = _name_to_id.find(name);
  if (it == _name_to_id.end())
    return {};
  return it->second;
}

const std::string &
MaterialPropertyRegistry::getName(const unsigned int id) const
{
  if (!hasProperty(id))
    mooseError("MaterialPropertyRegistry: Property with ID ", id, " is not declared");
  return _id_to_name[id];
}
