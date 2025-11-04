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
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes)
  : CSGLattice(name, MooseUtils::prettyCppType<CSGHexagonalLattice>()), _pitch(pitch)
{
  setUniverses(universes); // this will set _nrow
  if (_pitch < 0)
    mooseError("Lattice " + getName() + " must have pitch greater than 0.");
}

CSGHexagonalLattice::CSGHexagonalLattice(const std::string & name, Real pitch)
  : CSGLattice(name, MooseUtils::prettyCppType<CSGHexagonalLattice>()), _pitch(pitch), _nrow(0)
{
  if (_pitch < 0)
    mooseError("Lattice " + getName() + " must have pitch greater than 0.");
}

bool
CSGHexagonalLattice::isValidUniverseMap(
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes) const
{
  if (universes.size() < 1) // need at least one row
    return false;

  if (universes.size() % 2 == 0) // must be odd number of rows
    return false;

  // each row differs in how many elements are required depending on size of lattice
  int num_row = universes.size();
  int center_row = (num_row - 1) / 2;
  for (int row_i : index_range(universes))
  {
    int n_ele = num_row - std::abs(static_cast<int>(row_i - center_row));
    if (universes[row_i].size() != static_cast<size_t>(n_ele))
      return false;
  }

  return true;
}

void
CSGHexagonalLattice::setUniverses(
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes)
{
  // check for valid map arrangment
  if (!isValidUniverseMap(universes))
    mooseError("Cannot set lattice " + getName() +
               " with universes. Does not have valid dimensions for lattice type " + getType());
  // set dimensions attributes based on universe map (in case it differs from original dimensions)
  _nrow = universes.size();
  _universe_map = universes;
}
void
CSGHexagonalLattice::setPitch(const Real pitch)
{
  if (pitch < 0)
    mooseError("Lattice " + getName() + " must have pitch greater than 0.");
  _pitch = pitch;
}

std::unordered_map<std::string, std::any>
CSGHexagonalLattice::getDimensions() const
{
  return {{"nrow", _nrow}, {"nring", nRowToRing(_nrow)}, {"pitch", _pitch}};
}

bool
CSGHexagonalLattice::isValidIndex(const std::pair<int, int> index) const
{
  int row = index.first;  // row index
  int ele = index.second; // element index within the row

  // Check if row is valid (0 <= row < _nrow)
  if (row < 0 || row >= _nrow)
    return false;

  // Calculate maximum number of elements in this specific row
  int center_row = (_nrow - 1) / 2; // center row index
  int max_ele = _nrow - std::abs(row - center_row);

  // Check if element index is valid for this row
  if (ele < 0 || ele >= max_ele)
    return false;

  return true;
}

bool
CSGHexagonalLattice::compareDimensions(const CSGLattice & other) const
{
  if (other.getType() != this->getType())
    return false;

  auto this_dims = this->getDimensions();
  auto other_dims = other.getDimensions();

  if (std::any_cast<int>(this_dims["nrow"]) != std::any_cast<int>(other_dims["nrow"]))
    return false;
  if (std::any_cast<Real>(this_dims["pitch"]) != std::any_cast<Real>(other_dims["pitch"]))
    return false;
  return true;
}

int
CSGHexagonalLattice::getNRings() const
{
  return nRowToRing(_nrow);
}

/// convenience functions for converting between number of rows and number of rings

int
nRowToRing(int nrow)
{
  if (nrow == 0) // special case
    return 0;
  std::string base_msg = "Cannot convert number of rows " + std::to_string(nrow) +
                         " to number of rings in hexagonal lattice. ";
  if (nrow % 2 == 0)
    mooseError(base_msg + "Number of rows must be odd.");
  if (nrow < 0)
    mooseError(base_msg + "Number of rows must be >= 0.");
  return (nrow + 1) / 2;
}

int
nRingToRow(int nring)
{
  if (nring == 0) // special case
    return 0;
  if (nring < 0)
    mooseError("Cannot convert number of rings " + std::to_string(nring) +
               " to number of rows in hexagonal lattice. Number of rings must be >= 0.");
  return 2 * nring - 1;
}

} // namespace CSG