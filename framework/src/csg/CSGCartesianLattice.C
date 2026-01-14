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
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes,
    const std::optional<OuterVariant> & outer)
  : CSGLattice(name, MooseUtils::prettyCppType<CSGCartesianLattice>(), outer), _pitch(pitch)
{
  setUniverses(universes); // this will set _nrow and _ncol
  if (_pitch < 0)
    mooseError("Lattice " + getName() + " must have pitch greater than 0.");
}

CSGCartesianLattice::CSGCartesianLattice(const std::string & name,
                                         const Real pitch,
                                         const std::optional<OuterVariant> & outer)
  : CSGLattice(name, MooseUtils::prettyCppType<CSGCartesianLattice>(), outer),
    _pitch(pitch),
    _nrow(0),
    _ncol(0)
{
  if (_pitch < 0)
    mooseError("Lattice " + getName() + " must have pitch greater than 0.");
}

void
CSGCartesianLattice::setPitch(const Real pitch)
{
  if (pitch < 0)
    mooseError("Lattice " + getName() + " must have pitch greater than 0.");
  _pitch = pitch;
}

bool
CSGCartesianLattice::isValidUniverseMap(
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes) const
{
  // need at least one row
  if (universes.size() < 1)
    return false;

  // check that each row is same size
  auto row_size = universes[0].size();
  for (auto univ_list : universes)
  {
    if (univ_list.size() != row_size)
      return false;
  }
  return true;
}

void
CSGCartesianLattice::setUniverses(
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes)
{
  // check for valid map arrangment
  if (!isValidUniverseMap(universes))
    mooseError("Cannot set lattice " + getName() +
               " with universes. Does not have valid dimensions for lattice type " + getType());
  // set attributes based on universe map
  _nrow = universes.size();
  _ncol = universes[0].size();
  _universe_map = universes;
}

bool
CSGCartesianLattice::isValidIndex(const std::pair<int, int> index) const
{
  auto row = index.first;  // must be (0 <= row < _nrow); rows
  auto col = index.second; // must be (0 <= col < _ncol); cols
  return ((0 <= row && row < (int)_nrow) && (0 <= col && col < (int)_ncol));
}

bool
CSGCartesianLattice::compareAttributes(const CSGLattice & other) const
{
  if (other.getType() != this->getType())
    return false;

  auto this_dims = this->getAttributes();
  auto other_dims = other.getAttributes();
  if (std::get<unsigned int>(this_dims["nrow"]) != std::get<unsigned int>(other_dims["nrow"]))
    return false;
  if (std::get<unsigned int>(this_dims["ncol"]) != std::get<unsigned int>(other_dims["ncol"]))
    return false;
  if (std::get<Real>(this_dims["pitch"]) != std::get<Real>(other_dims["pitch"]))
    return false;
  return true;
}

} // namespace CSG
