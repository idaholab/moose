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
  _root_universe = std::make_shared<CSGUniverse>(root_name, true); // is_root = true
  _universes.insert(std::make_pair(root_name, _root_universe));
}

void
CSGUniverseList::checkUniverseName(const std::string name)
{
  if (_universes.find(name) != _universes.end())
    mooseError("Universe with name " + name + " already exists in geoemetry.");
}

const std::shared_ptr<CSGUniverse> &
CSGUniverseList::getUniverse(const std::string name)
{
  auto univ = _universes.find(name);
  if (univ == _universes.end())
    mooseError("No universe by name " + name + " exists in the geometry.");
  else
    return univ->second;
}

std::shared_ptr<CSGUniverse>
CSGUniverseList::addUniverse(const std::string name)
{
  checkUniverseName(name);
  _universes.insert(std::make_pair(name, std::make_shared<CSGUniverse>(name)));
  return _universes[name];
}

std::shared_ptr<CSGUniverse>
CSGUniverseList::addUniverse(const std::string name, std::vector<std::shared_ptr<CSGCell>> cells)
{
  checkUniverseName(name);
  _universes.insert(std::make_pair(name, std::make_shared<CSGUniverse>(name, cells)));
  return _universes[name];
}

void
CSGUniverseList::addUniverse(const std::pair<std::string, std::shared_ptr<CSGUniverse>> universe)
{
  checkUniverseName(universe.first);
  _universes.insert(universe);
}

void
CSGUniverseList::renameRoot(const std::string name)
{
  auto prev_root_name = _root_universe->getName();
  _root_universe->setName(name);
  _universes.erase(prev_root_name);
  _universes.insert(std::make_pair(name, _root_universe));
}

} // namespace CSG
