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
  auto root_universe = std::make_unique<CSGUniverse>(root_name, /* is_root = */ true);
  _root_universe = root_universe.get();
  _universes.insert(std::make_pair(root_name, std::move(root_universe)));
}

CSGUniverse &
CSGUniverseList::getUniverse(const std::string & name) const
{
  auto univ = _universes.find(name);
  if (univ == _universes.end())
    mooseError("No universe by name " + name + " exists in the geometry.");
  else
    return *(univ->second);
}

std::vector<std::reference_wrapper<const CSGUniverse>>
CSGUniverseList::getAllUniverses() const
{
  std::vector<std::reference_wrapper<const CSGUniverse>> univs;
  for (auto it = _universes.begin(); it != _universes.end(); ++it)
    univs.push_back(*(it->second.get()));
  return univs;
}

CSGUniverse &
CSGUniverseList::addUniverse(const std::string & name)
{
  return addUniverse(std::make_unique<CSGUniverse>(name));
}

CSGUniverse &
CSGUniverseList::addUniverse(std::unique_ptr<CSGUniverse> universe,
                             const bool ignore_identical_universe)
{
  auto univ_name = universe->getName();
  if (ignore_identical_universe)
    // Check that universe already defined in _universes and if so, confirm it matches with input
    // univ
    if (auto it = _universes.find(univ_name); it != _universes.end())
    {
      if (*universe == *it->second)
        return *it->second;
      else
        mooseError("Universe with name ",
                   univ_name,
                   " has the same name as an existing universe in CSGBase instance but cannot be "
                   "discarded as it is not an identical universe.");
    }

  // Otherwise, add the universe to the universe list. At this point, we don't expect the universe
  // to already be defined in the universe list
  auto [it, inserted] = _universes.emplace(univ_name, std::move(universe));
  if (!inserted)
    mooseError("Universe with name " + univ_name + " already exists in geometry.");
  return *it->second;
}

void
CSGUniverseList::renameUniverse(const CSGUniverse & universe, const std::string & name)
{
  // check that this universe passed in is actually in the same universe that is in the universe
  // list
  auto prev_name = universe.getName();
  auto it = _universes.find(prev_name);
  if (it == _universes.end() || it->second.get() != &universe)
    mooseError("Universe " + prev_name + " cannot be renamed to " + name +
               " as it does not exist in this CSGBase instance.");

  auto existing_univ = std::move(it->second);
  existing_univ->setName(name);
  _universes.erase(prev_name);
  addUniverse(std::move(existing_univ));
}

bool
CSGUniverseList::operator==(const CSGUniverseList & other) const
{
  const auto all_univs = this->getAllUniverses();
  const auto other_univs = other.getAllUniverses();

  // Check that same number of universes are defined in both lists
  if (all_univs.size() != other_univs.size())
    return false;

  // Iterate through each CSGUniverse in list and check equality of universe
  // with other list
  for (const auto & univ : all_univs)
  {
    const auto & univ_name = univ.get().getName();
    if (!other.hasUniverse(univ_name))
      return false;
    const auto & other_univ = other.getUniverse(univ_name);
    if (univ.get() != other_univ)
      return false;
  }
  return true;
}

bool
CSGUniverseList::operator!=(const CSGUniverseList & other) const
{
  return !(*this == other);
}

} // namespace CSG
