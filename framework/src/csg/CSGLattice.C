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

CSGLattice::CSGLattice(
    const std::string & name,
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes,
    const std::string & lattice_type)
  : _name(name), _lattice_type(lattice_type), _universe_map(universes)
{
  if (!hasValidDimensions())
    mooseError("Cannot construct lattice " + getName() + " of type " + getType() +
               " with provided universes (invalid dimensions).")
}

void CSGLattice::setUniverses(
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes);
{
  _universe_map = universes;
  if (!hasValidDimensions())
    mooseError("Cannot set lattice " + getName() +
               " with universes. Does not have valid dimensions for lattice type " + getType());
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
CSGLattice::getUniverseAtIndex(const std::pair<int, int> index)
{
  if (!isValidIndex(index))
    mooseError("Index ()" + std::to_string(index.first) + ", " + std::to_string(index.second) +
               ") is not a valid index for lattice " + getName());
  else
    return _universe_map[index.first][index.second];
}

const std::vector<std::pair<int, int>>
CSGLattice::getUniverseIndices(const std::string & univ_name) const
{
  if (!hasUniverse(univ_name))
    mooseError("Universe " + univ_name + " does not exist in lattice " + getName());

  std::vector<std::pair<int, int>> indices;
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

std::pair<int, int>
CSGLattice::getIndexAtPoint(Point point)
{
  // always check if point is within the lattice bounds
  if (!isPointInLattice(point))
    mooseError("Point not within bounds of lattice " + getName());
  return getIndex(point);
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
