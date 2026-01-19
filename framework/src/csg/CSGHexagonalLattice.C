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
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes,
    const std::optional<OuterVariant> & outer)
  : CSGLattice(name, MooseUtils::prettyCppType<CSGHexagonalLattice>(), outer), _pitch(pitch)
{
  setUniverses(universes); // this will set _nrow
  if (_pitch < 0)
    mooseError("Lattice " + getName() + " must have pitch greater than 0.");
}

CSGHexagonalLattice::CSGHexagonalLattice(const std::string & name,
                                         Real pitch,
                                         const std::optional<OuterVariant> & outer)
  : CSGLattice(name, MooseUtils::prettyCppType<CSGHexagonalLattice>(), outer),
    _pitch(pitch),
    _nrow(0),
    _nring(0)
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
  unsigned int num_row = universes.size();
  unsigned int center_row = (num_row - 1) / 2;
  for (unsigned int row_i : index_range(universes))
  {
    unsigned int n_ele =
        num_row - ((row_i > center_row) ? (row_i - center_row) : (center_row - row_i));
    if (universes[row_i].size() != n_ele)
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
  // set attributes based on universe map (in case they differ from original values)
  _nrow = universes.size();
  _nring = nRowToRing(_nrow);
  _universe_map = universes;
  buildIndexMap();
}
void
CSGHexagonalLattice::setPitch(const Real pitch)
{
  if (pitch < 0)
    mooseError("Lattice " + getName() + " must have pitch greater than 0.");
  _pitch = pitch;
}

bool
CSGHexagonalLattice::isValidIndex(const std::pair<int, int> index) const
{
  auto row = index.first;  // row index
  auto ele = index.second; // column index within the row

  // Check if row is valid (0 <= row < _nrow)
  if (row < 0 || row >= (int)_nrow)
    return false;

  // Calculate maximum number of elements in this specific row
  auto center_row = (_nrow - 1) / 2; // center row index
  int max_ele = _nrow - std::abs((int)(row - center_row));

  // Check if column index is valid for this row
  return !(ele < 0 || ele >= max_ele);
}

bool
CSGHexagonalLattice::compareAttributes(const CSGLattice & other) const
{
  if (other.getType() != this->getType())
    return false;

  auto this_dims = this->getAttributes();
  auto other_dims = other.getAttributes();
  if (std::get<unsigned int>(this_dims["nrow"]) != std::get<unsigned int>(other_dims["nrow"]))
    return false;
  if (std::get<unsigned int>(this_dims["nring"]) != std::get<unsigned int>(other_dims["nring"]))
    return false;
  if (std::get<Real>(this_dims["pitch"]) != std::get<Real>(other_dims["pitch"]))
    return false;
  return true;
}

void
CSGHexagonalLattice::buildIndexMap()
{
  for (const auto ring : make_range(_nring))
  {
    unsigned int num_elements = (ring == _nring - 1) ? 1 : 6 * (_nring - 1 - ring);
    for (const auto element make_range(num_elements))
    {
      std::pair<unsigned int, unsigned int> ring_index = std::make_pair(ring, element);
      std::pair<unsigned int, unsigned int> row_index = getRowIndexFromRingIndex(ring_index);
      _row_to_ring_map[row_index] = ring_index;
    }
  }
}

std::pair<int, int>
CSGHexagonalLattice::getRowIndexFromRingIndex(const std::pair<int, int> & ring_ele_index) const
{
  auto og_ring = ring_ele_index.first; // ring corresponds to the outermost ring as ring 0
  int ring = _nring - og_ring - 1;     // convert to internal indexing (0 as innermost ring)
  auto element = ring_ele_index.second;

  if (og_ring < 0 || og_ring >= (int)_nring)
    mooseError("Ring " + std::to_string(og_ring) + " is not valid for hexagonal lattice " +
               getName());
  if (element < 0 || element >= (ring == 0 ? 1 : 6 * ring))
    mooseError("Position " + std::to_string(element) + " is not valid for ring " +
               std::to_string(og_ring) + " in hexagonal lattice " + getName());

  // Calculate the center row and column indices
  int center_row = (_nrow - 1) / 2;
  int center_col = center_row;

  // Special case for the center element
  if (ring == 0)
    return {center_row, center_col};

  // Calculate the side length of the hexagon for the given ring
  int side_length = ring;

  // Determine which side of the hexagon the element is on and get row/col from this
  int side = element / side_length;
  int offset = element % side_length; // position within the side moving counter-clockwise
  int row, col;

  // lamba to calculate the number of columns in any given row
  auto calc_num_cols_in_row = [&](int r) { return _nrow - std::abs(center_row - r); };

  // diagram of side numbers:
  //            4
  //         _______
  //       /         \*
  //    3 /           \ 5
  //     /             \*
  //     \             /
  //    2 \           / 0
  //       \ _______ /
  //            1
  switch (side)
  {
    case 0: // bottom right (contains starting position of the ring)
      row = center_row + offset;
      col = calc_num_cols_in_row(row) - og_ring - 1;
      break;
    case 1: // bottom
      row = center_row + ring;
      col = calc_num_cols_in_row(row) - og_ring - 1 - offset;
      break;
    case 2: // bottom left
      row = center_row + (side_length - offset);
      col = center_col - ring;
      break;
    case 3: // top left
      row = center_row - offset;
      col = center_col - ring;
      break;
    case 4: // top
      row = center_row - ring;
      col = center_col - ring + offset;
      break;
    case 5: // top right
      row = center_row - (side_length - offset);
      col = calc_num_cols_in_row(row) - og_ring - 1;
      break;
    default:
      mooseError("Invalid side ID calculation in hexagonal lattice " + getName());
  }

  mooseAssert(isValidIndex(std::make_pair(row, col)),
              "Calculated index (" + std::to_string(row) + ", " + std::to_string(col) +
                  ") is not valid for hexagonal lattice " + getName());

  return {row, col};
}

std::pair<int, int>
CSGHexagonalLattice::getRingIndexFromRowIndex(const std::pair<int, int> & row_col_index) const
{
  if (!isValidIndex(row_col_index))
    mooseError("Index (" + std::to_string(row_col_index.first) + ", " +
               std::to_string(row_col_index.second) +
               ") is not a valid index for hexagonal "
               "lattice " +
               getName());

  return _row_to_ring_map.at(row_col_index);
}

/// convenience functions for converting between number of rows and number of rings

unsigned int
nRowToRing(int nrow)
{
  if (nrow == 0) // special case
    return 0;
  std::string base_msg = "Cannot convert number of rows " + std::to_string(nrow) +
                         " to number of rings in hexagonal lattice. ";
  if (nrow < 0)
    mooseError(base_msg + "Number of rows must be >= 0.");
  if (nrow % 2 == 0)
    mooseError(base_msg + "Number of rows must be odd.");
  return (nrow + 1) / 2;
}

unsigned int
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
