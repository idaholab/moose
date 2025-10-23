//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGCartesianLattice.h"

namespace CSG
{

CSGCartesianLattice::CSGCartesianLattice(
    const std::string & name,
    const Real pitch,
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes)
  : CSGLattice(name, MooseUtils::prettyCppType<CSGCartesianLattice>()),
    _nx0(universes.size()),
    _nx1(universes[0].size()),
    _pitch(pitch)
{
  if (!hasValidDimensions())
    mooseError("Lattice " + getName() + " of type " + getType() +
               " must have pitch and number of elements in both dimensions greater than 0.");
  initializeUniverseMap();
  setUniverses(universes);
}

CSGCartesianLattice::CSGCartesianLattice(const std::string & name,
                                         const int nx0,
                                         const int nx1,
                                         const Real pitch)
  : CSGLattice(name, MooseUtils::prettyCppType<CSGCartesianLattice>()),
    _nx0(nx0),
    _nx1(nx1),
    _pitch(pitch)
{
  if (!hasValidDimensions())
    mooseError("Lattice " + getName() + " of type " + getType() +
               " must have pitch and number of elements in both dimensions greater than 0.");
  initializeUniverseMap();
}

void
CSGCartesianLattice::initializeUniverseMap()
{
  size_t rows = _nx0;
  size_t cols = _nx1;
  _universe_map =
      std::vector<std::vector<std::optional<std::reference_wrapper<const CSGUniverse>>>>(
          rows,
          std::vector<std::optional<std::reference_wrapper<const CSGUniverse>>>(cols,
                                                                                std::nullopt));
}

bool
CSGCartesianLattice::hasValidDimensions() const
{
  return (_nx0 > 0 && _nx1 > 0 && _pitch > 0);
}

bool
CSGCartesianLattice::isValidUniverseMap(
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes) const
{
  // make sure universes exist at all and the right number of sublists are present (_nx0)
  if (universes.size() != static_cast<size_t>(_nx0))
    return false;

  // check that each sublist is same size and equal to _nx1
  for (auto univ_list : universes)
  {
    if (univ_list.size() != static_cast<size_t>(_nx1))
      return false;
  }

  return true;
}

void
CSGCartesianLattice::updateDimension(const std::string & dim_name, std::any dim_value)
{
  if (dim_name == "pitch")
  {
    auto val = *std::any_cast<Real>(&dim_value);
    if (val <= 0.0)
      mooseError("Updated pitch value for lattice " + getName() + " is not valid.");
    _pitch = val;
  }
  else if (dim_name == "nx0" || dim_name == "nx1")
  {
    // number of lattice elements can only be changed if _universe_map is not already set
    if (_universe_map.size() > 0)
      mooseError("Cannot update the dimension " + dim_name + " of the lattice " + getName() +
                 ". Universe map is already defined.");
    else
    {
      auto val = *std::any_cast<int>(&dim_value);
      if (val < 1)
        mooseError("Updated " + dim_name + " value for lattice " + getName() + " is not valid.");
      dim_name == "nx0" ? _nx0 = val : _nx1 = val;
    }
  }
  else
    mooseError("Dimension " + dim_name + " is not an allowable dimension for lattice type " +
               getType() + " allowable dimension types are: nx0, nx1, pitch.");
  if (!hasValidDimensions())
    mooseError("Lattice " + getName() + " does not have valid dimensions after updating " +
               dim_name);
}

std::unordered_map<std::string, std::any>
CSGCartesianLattice::getDimensions() const
{
  std::unordered_map<std::string, std::any> dims_map;
  dims_map["nx0"] = _nx0;
  dims_map["nx1"] = _nx1;
  dims_map["pitch"] = _pitch;
  return dims_map;
}

bool
CSGCartesianLattice::isValidIndex(const std::pair<int, int> index) const
{
  auto x0 = index.first;  // must be (0 <= x0 < _nx0)
  auto x1 = index.second; // must be (0 <= x1 < _nx1)
  if (x0 < 0 || x0 >= _nx0 || x1 < 0 || x1 >= _nx1)
    return false;
  return true;
}

bool
CSGCartesianLattice::compareDimensions(const CSGLattice & other) const
{
  if (other.getType() != this->getType())
    return false;

  auto this_dims = this->getDimensions();
  auto other_dims = other.getDimensions();

  if (std::any_cast<int>(this_dims["nx0"]) != std::any_cast<int>(other_dims["nx0"]))
    return false;
  if (std::any_cast<int>(this_dims["nx1"]) != std::any_cast<int>(other_dims["nx1"]))
    return false;
  if (std::any_cast<Real>(this_dims["pitch"]) != std::any_cast<Real>(other_dims["pitch"]))
    return false;
  return true;
}

} // namespace CSG
