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

std::unordered_map<std::string, std::any>
CSGHexagonalLattice::getDimensions() const
{
  return {{"nrow", _nrow}, {"pitch", _pitch}};
}

void
CSGHexagonalLattice::updateDimension(const std::string & dim_name, std::any dim_value)
{
  if (dim_name == "nrow")
  {
    // number of lattice elements can only be changed if _universe_map is not already set
    if (_universe_map.size() > 0)
      mooseError("Cannot update the dimension " + dim_name + " of the lattice " + getName() +
                 ". Universe map is already defined.");
    else
    {
      auto v = std::any_cast<int>(dim_value);
      if (v < 1)
        mooseError("Updated number of rows nrow for lattice " + getName() + " must be >= 1.");
      if (v % 2 == 0)
        mooseError("Updated number of rows nrow for lattice " + getName() +
                   " must be an odd number.");
      _nrow = v;
    }
  }
  else if (dim_name == "pitch")
  {
    auto v = std::any_cast<Real>(dim_value);
    if (v <= 0.0)
      mooseError("Updated pitch value for lattice " + getName() + " must be > 0.");
    _pitch = v;
  }
  else
    mooseError("Dimension " + dim_name + " is not an allowable dimension for lattice type " +
               getType() + ". Allowable dimension types are: nrow, pitch.");
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

std::pair<int, int>
CSGHexagonalLattice::getRingIndexFromRowIndex(const std::pair<int, int> row_index) const
{
  int row = row_index.first;
  int ele = row_index.second;

  // Validate the input index
  if (!isValidIndex(row_index))
    mooseError("Invalid row index (" + std::to_string(row) + ", " + std::to_string(ele) +
               ") for hexagonal lattice " + getName());

  int center_row = (_nrow - 1) / 2;
  int num_rings = nRowToRing(_nrow);

  // Special case: center element
  if (row == center_row && ele == center_row)
    return std::make_pair(num_rings - 1, 0); // center corresponds to total number of rings

  // Calculate ring distance from center
  int ring_dist = std::max(std::abs(row - center_row), std::abs(ele - center_row));
  int ring = num_rings - 1 - ring_dist; // convert to outermost ring=0 numbering scheme

  // Calculate element index within the ring
  int ring_index;

  if (ring_dist > 0) // not the center
  {
    // Convert to relative coordinates from center
    int rel_row = row - center_row;
    int rel_ele = ele - center_row;

    // Determine which side and position within that side
    if (rel_ele == ring_dist) // right side
    {
      ring_index = ring_dist + rel_row; // moving up from bottom-right corner
    }
    else if (rel_row == -ring_dist) // top side
    {
      ring_index = 3 * ring_dist - rel_ele; // moving left from top-right corner
    }
    else if (rel_ele == -ring_dist) // left side
    {
      ring_index = 5 * ring_dist - rel_row; // moving down from top-left corner
    }
    else if (rel_row == ring_dist) // bottom side
    {
      ring_index = 4 * ring_dist + rel_ele; // moving right from bottom-left corner
    }
    else if (rel_row + rel_ele == ring_dist) // bottom-right diagonal
    {
      ring_index = rel_ele; // position along bottom-right edge
    }
    else if (rel_row + rel_ele == -ring_dist) // top-left diagonal
    {
      ring_index = 2 * ring_dist - rel_ele; // position along top-left edge
    }
  }

  return std::make_pair(ring, ring_index);
}

std::pair<int, int>
CSGHexagonalLattice::getRowIndexFromRingIndex(const std::pair<int, int> ring_index) const
{
  int ring = ring_index.first;
  int index = ring_index.second;
  int num_rings = nRowToRing(_nrow);

  // Validate ring number
  if (ring < 0 || ring >= num_rings)
    mooseError("Invalid ring " + std::to_string(ring) + " for hexagonal lattice " + getName() +
               ". Must be 0 <= ring < " + std::to_string(num_rings));

  int center_row = (_nrow - 1) / 2;

  // Special case: center cell (innermost ring)
  if (ring == num_rings - 1)
  {
    if (index != 0)
      mooseError("Center ring can only have index 0, got " + std::to_string(index));
    return std::make_pair(center_row, center_row);
  }

  // Calculate ring distance from center
  int ring_dist = num_rings - 1 - ring;

  // Validate index for this ring
  int max_index = 6 * ring_dist;
  if (index < 0 || index >= max_index)
    mooseError("Invalid index " + std::to_string(index) + " for ring " + std::to_string(ring) +
               ". Must be 0 <= index < " + std::to_string(max_index));

  int rel_row, rel_ele;

  // Determine position based on which side of hexagon
  if (index < ring_dist) // bottom-right edge (indices 0 to ring_dist-1)
  {
    rel_row = ring_dist - index;
    rel_ele = index;
  }
  else if (index < 2 * ring_dist) // right side (indices ring_dist to 2*ring_dist-1)
  {
    rel_row = index - ring_dist;
    rel_ele = ring_dist;
  }
  else if (index < 3 * ring_dist) // top-right edge (indices 2*ring_dist to 3*ring_dist-1)
  {
    rel_row = -ring_dist;
    rel_ele = 3 * ring_dist - index;
  }
  else if (index < 4 * ring_dist) // top-left edge (indices 3*ring_dist to 4*ring_dist-1)
  {
    rel_row = index - 3 * ring_dist - ring_dist;
    rel_ele = -ring_dist;
  }
  else if (index < 5 * ring_dist) // left side (indices 4*ring_dist to 5*ring_dist-1)
  {
    rel_row = ring_dist;
    rel_ele = index - 4 * ring_dist - ring_dist;
  }
  else // bottom-left edge (indices 5*ring_dist to 6*ring_dist-1)
  {
    rel_row = 5 * ring_dist - index;
    rel_ele = index - 5 * ring_dist;
  }

  // Convert relative coordinates back to absolute row/element
  int row = center_row + rel_row;
  int ele = center_row + rel_ele;

  return std::make_pair(row, ele);
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