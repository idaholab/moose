//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGLattice.h"

namespace CSG
{

CSGLattice::CSGLattice(const std::string & name, const std::string & lattice_type)
  : _name(name), _lattice_type(lattice_type)
{
}

void
CSGLattice::setUniverses(
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes)
{
  if (!isValidUniverseMap(universes))
    mooseError("Cannot set lattice " + getName() +
               " with universes. Does not have valid dimensions for lattice type " + getType());
  _universe_map = universes;
}

void
CSGLattice::setUniverseAtIndex(std::reference_wrapper<const CSGUniverse> universe,
                               const std::pair<unsigned int, unsigned int> index)
{
  std::string base_msg = "Cannot set universe at location (" + std::to_string(index.first) + ", " +
                         std::to_string(index.second) + ") for lattice " + getName() + ". ";
  if (!isValidIndex(index))
    mooseError(base_msg + "Not valid location.");
  if (_universe_map.size() == 0)
    mooseError(base_msg + "Universe map has not been initialized.");
  _universe_map[index.first][index.second] = universe;
}

bool
CSGLattice::hasUniverse(const std::string & name) const
{
  for (auto list_univ : _universe_map)
  {
    for (const CSGUniverse & univ : list_univ)
    {
      if (univ.getName() == name)
        return true;
    }
  }
  return false; // no universe with matching name was found
}

void
CSGLattice::checkDimensions() const
{
  if (!hasValidDimensions())
    mooseError("Lattice " + getName() + " does not have valid dimensions for lattice type " +
               getType());
}

std::reference_wrapper<const CSGUniverse>
CSGLattice::getUniverseAtIndex(const std::pair<unsigned int, unsigned int> index)
{
  if (!isValidIndex(index))
    mooseError("Index ()" + std::to_string(index.first) + ", " + std::to_string(index.second) +
               ") is not a valid index for lattice " + getName());
  else
    return _universe_map[index.first][index.second];
}

const std::vector<std::pair<unsigned int, unsigned int>>
CSGLattice::getUniverseIndices(const std::string & univ_name) const
{
  if (!hasUniverse(univ_name))
    mooseError("Universe " + univ_name + " does not exist in lattice " + getName());

  std::vector<std::pair<unsigned int, unsigned int>> indices;
  for (auto i : make_range(_universe_map.size()))
  {
    for (auto j : make_range(_universe_map[i].size()))
    {
      const CSGUniverse & univ = _universe_map[i][j];
      if (univ.getName() == _name)
        indices.push_back(std::make_pair(i, j));
    }
  }
  return indices;
}

// bool
// CSGLattice::operator==(const CSGLattice & other) const
// {
//   return (getName() == other.getName()) &&
//          (getDimensions() == other.getDimensions()) &&
//          (getType() == other.getType());
// }

// bool
// CSGLattice::operator!=(const CSGLattice & other) const
// {
//   return !(*this == other);
// }

} // namespace CSG
