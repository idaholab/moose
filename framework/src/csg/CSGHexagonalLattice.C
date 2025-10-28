//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGHexagonalLattice.h"

namespace CSG
{

CSGHexagonalLattice::CSGHexagonalLattice(
    const std::string & name,
    Real pitch,
    std::string orientation,
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes)
  : CSGLattice(name, MooseUtils::prettyCppType<CSGHexagonalLattice>()),
    _num_rings((universes.size() % 2 == 1) ? (universes.size() - 1) / 2 : -1),
    _pitch(pitch),
    _orientation(orientation)
{
  if (!hasValidDimensions())
    mooseError("Hex lattice \"" + name + "\" must have odd-size universes and pitch > 0.");
  if (!isValidUniverseMap(universes))
    mooseError("Universe map does not match hexagon pattern for lattice \"" + name + "\".");
  setUniverses(universes);
}

CSGHexagonalLattice::CSGHexagonalLattice(const std::string & name,
                                         int num_rings,
                                         Real pitch,
                                         std::string orientation)
  : CSGLattice(name, MooseUtils::prettyCppType<CSGHexagonalLattice>()),
    _num_rings(num_rings),
    _pitch(pitch),
    _orientation(orientation)
{
  if (!hasValidDimensions())
    mooseError("Hex lattice \"" + name + "\" must have num_rings > 0 and pitch > 0.");
}

bool
CSGHexagonalLattice::hasValidDimensions() const
{
  return (_num_rings > 0 && _pitch > 0.0);
}

std::unordered_map<std::string, std::any>
CSGHexagonalLattice::getDimensions() const
{
  return {{"num_rings", _num_rings}, {"pitch", _pitch}};
}

void
CSGHexagonalLattice::updateDimension(const std::string & dim_name, std::any dim_value)
{
  if (dim_name == "num_rings")
  {
    int v = std::any_cast<int>(dim_value);
    if (v < 1)
      mooseError("Updated num_rings for lattice \"" + getName() + "\" must be >= 1.");
    _num_rings = v;
  }
  else if (dim_name == "pitch")
  {
    Real v = std::any_cast<Real>(dim_value);
    if (v <= 0.0)
      mooseError("Updated pitch for lattice \"" + getName() + "\" must be > 0.");
    _pitch = v;
  }
  else
    mooseError("Invalid dimension \"" + dim_name + "\" for hex lattice \"" + getName() +
               "\". Valid names: num_rings, pitch.");

  if (!hasValidDimensions())
    mooseError("Dimensions invalid after update for lattice \"" + getName() + "\".");
}

bool
CSGHexagonalLattice::isValidIndex(const std::pair<int, int> index) const
{
  int N = 2 * _num_rings + 1;
  return (index.first >= 0 && index.first < N && index.second >= 0 && index.second < N);
}

bool
CSGHexagonalLattice::isValidUniverseMap(
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes) const
{
  int N = 2 * _num_rings + 1;
  if (universes.size() != static_cast<size_t>(N))
    return false;

  for (auto & row : universes)
    if (row.size() != static_cast<size_t>(N))
      return false;

  return true;
}

bool
CSGHexagonalLattice::compareDimensions(const CSGLattice & other) const
{
  if (other.getType() != this->getType())
    return false;

  auto dims = other.getDimensions();
  return _num_rings == std::any_cast<int>(dims.at("num_rings")) &&
         _pitch == std::any_cast<Real>(dims.at("pitch"));
}

} // namespace CSG