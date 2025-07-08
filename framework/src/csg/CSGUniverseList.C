//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGUniverseList.h"

namespace CSG
{

CSGUniverseList::CSGUniverseList()
{
  // create root universe by default when CSG object is initialized
  std::string root_name = "ROOT_UNIVERSE";
  _root_universe = std::make_shared<CSGUniverse>(root_name, /* is_root = */ true);
  _universes.insert(std::make_pair(root_name, _root_universe));
}

void
CSGUniverseList::checkUniverseName(const std::string name) const
{
  if (_universes.find(name) != _universes.end())
    mooseError("Universe with name " + name + " already exists in geoemetry.");
}

CSGUniverse &
CSGUniverseList::getUniverse(const std::string name)
{
  auto univ = _universes.find(name);
  if (univ == _universes.end())
    mooseError("No universe by name " + name + " exists in the geometry.");
  else
    return *(univ->second);
}

std::vector<CSGUniverse *>
CSGUniverseList::getAllUniverses() const
{
  std::vector<CSGUniverse *> univs;
  for (auto it = _universes.begin(); it != _universes.end(); ++it)
    univs.push_back(it->second.get());
  return univs;
}

CSGUniverse &
CSGUniverseList::addUniverse(const std::string name)
{
  checkUniverseName(name);
  _universes.insert(std::make_pair(name, std::make_shared<CSGUniverse>(name)));
  return *_universes[name];
}

CSGUniverse &
CSGUniverseList::addUniverse(const std::string name, std::vector<CSGCell *> & cells)
{
  checkUniverseName(name);
  _universes.insert(std::make_pair(name, std::make_shared<CSGUniverse>(name, cells)));
  return *_universes[name];
}

void
CSGUniverseList::addUniverse(std::shared_ptr<CSGUniverse> & universe)
{
  auto name = universe->getName();
  checkUniverseName(name);
  _universes.insert(std::make_pair(name, universe));
}

void
CSGUniverseList::renameUniverse(CSGUniverse & universe, const std::string name)
{
  // check that this universe passed in is actually in the same universe that is in the universe
  // list
  auto prev_name = universe.getName();
  auto existing_univ = _universes.find(prev_name)->second;
  if (*existing_univ != universe)
    mooseError("Universe " + prev_name + " cannot be renamed to " + name +
               " as it does not exist in this CSGBase instance.");

  checkUniverseName(name);
  existing_univ->setName(name);
  _universes.erase(prev_name);
  _universes.insert(std::make_pair(name, existing_univ));
}

} // namespace CSG
