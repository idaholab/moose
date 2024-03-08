//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CartesianGridPositions.h"

registerMooseObject("ReactorApp", CartesianGridPositions);

InputParameters
CartesianGridPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params.addClassDescription("Create positions along a Cartesian grid.");

  params.addRequiredParam<Point>("center", "Center of the Cartesian grid");
  params.addRequiredRangeCheckedParam<Real>("dx", "dx>0", "Lattice extent in the X direction");
  params.addRequiredRangeCheckedParam<Real>("dy", "dy>0", "Lattice extent in the Y direction");
  params.addRequiredRangeCheckedParam<Real>("dz", "dz>0", "Lattice extent in the Z direction");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "nx", "nx>0", "Number of points in the X direction");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "ny", "ny>0", "Number of points in the Y direction");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "nz", "nz>0", "Number of points in the Z direction");

  // Pattern for selecting some lattice positions
  params.addRangeCheckedParam<std::vector<std::vector<std::vector<unsigned int>>>>(
      "pattern",
      {},
      "pattern>=0",
      "A double-indexed Cartesian-shaped array starting with the upper-left corner.");
  params.addParam<std::vector<unsigned int>>(
      "include_in_pattern", {}, "A vector of the numbers in the pattern to include");

  // Use user-provided ordering
  params.set<bool>("auto_sort") = false;
  // All functors defined on all processes for now
  params.set<bool>("auto_broadcast") = false;

  return params;
}

CartesianGridPositions::CartesianGridPositions(const InputParameters & parameters)
  : Positions(parameters),
    _center(getParam<Point>("center")),
    _dx(getParam<Real>("dx")),
    _dy(getParam<Real>("dy")),
    _dz(getParam<Real>("dz")),
    _nx(getParam<unsigned int>("nx")),
    _ny(getParam<unsigned int>("ny")),
    _nz(getParam<unsigned int>("nz")),
    _pattern(getParam<std::vector<std::vector<std::vector<unsigned int>>>>("pattern")),
    _include_in_pattern(
        std::set<unsigned int>(getParam<std::vector<unsigned int>>("include_in_pattern").begin(),
                               getParam<std::vector<unsigned int>>("include_in_pattern").end()))
{
  if ((_include_in_pattern.empty() && _pattern.size()) ||
      (_include_in_pattern.size() && _pattern.empty()))
    paramError(
        "include_in_pattern",
        "The 'pattern' parameter and the include_in_pattern must be both specified or both not "
        "specified by the user.");
  for (const auto include : _include_in_pattern)
  {
    bool found = false;
    for (const auto iz : index_range(_pattern))
      for (const auto iy : index_range(_pattern[iz]))
      {
        const auto row = _pattern[iz][iy];

        if (std::find(row.begin(), row.end(), include) != row.end())
          found = true;
      }
    if (!found)
      paramError("include_in_pattern",
                 "Pattern item " + std::to_string(include) +
                     " to include is not present in the pattern");
  }
  // Check size of pattern
  if (_pattern.size())
  {
    if (_pattern.size() != _nz)
      mooseError("Wrong pattern size in Z direction: ", _pattern.size());
    for (const auto & column : _pattern)
      if (column.size() != _ny)
        mooseError("Wrong pattern size in Y direction: ", column.size());
    for (const auto & column : _pattern)
      for (const auto & row : column)
        if (row.size() != _nx)
          mooseError("Wrong pattern size in X direction: ", row.size());
  }

  // Obtain the positions by evaluating the functors
  initialize();
  // Sort if needed (user-specified)
  finalize();
}

void
CartesianGridPositions::initialize()
{
  clearPositions();

  // Count the number of positions we do not need to include
  unsigned int n_exclusions = 0;
  if (_include_in_pattern.size())
    for (const auto iz : index_range(_pattern))
      for (const auto iy : index_range(_pattern[iz]))
        for (const auto ix : index_range(_pattern[iz][iy]))
          if (!_include_in_pattern.count(_pattern[iz][iy][ix]))
            n_exclusions++;
  const auto n_positions = _nx * _ny * _nz - n_exclusions;
  _positions.resize(n_positions);

  // Fill the positions by retrieving the pin centers at indices included in the pattern (if
  // specified)
  unsigned int pos_i = 0;
  for (const auto iz : make_range(_nz))
    for (const auto iy : make_range(_ny))
      for (const auto ix : make_range(_nx))
      {
        if (!_pattern.size() || !_include_in_pattern.size() ||
            _include_in_pattern.count(_pattern[iz][iy][ix]))
          _positions[pos_i++] = _center + Point(_dx / 2 * ((2. * ix + 1) / _nx - 1),
                                                _dy / 2 * ((2. * iy + 1) / _ny - 1),
                                                _dz / 2 * ((2. * iz + 1) / _nz - 1));
      }
  _initialized = true;
}
