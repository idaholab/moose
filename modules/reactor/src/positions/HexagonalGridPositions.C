//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HexagonalGridPositions.h"
#include "HexagonalLatticeUtils.h"

registerMooseObject("ReactorApp", HexagonalGridPositions);

InputParameters
HexagonalGridPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params.addClassDescription(
      "Create positions along a hexagonal grid. Numbering of positions increases first "
      "counterclockwise, then expanding outwards from the inner ring. "
      "Inner-numbering is within a radial ring.");

  params.addRequiredParam<Point>("center", "Center of the hexagonal grid");
  params.addRequiredRangeCheckedParam<Real>(
      "lattice_flat_to_flat",
      "lattice_flat_to_flat>0",
      "Distance between two (inner) opposite sides of a lattice. Also known as bundle pitch or "
      "inner flat-to-flat distance");
  params.addRequiredRangeCheckedParam<Real>("pin_pitch", "pin_pitch>0", "Distance between pins");
  params.addRequiredRangeCheckedParam<unsigned int>("nr", "nr>0", "Number of hexagonal rings");
  params.addRangeCheckedParam<std::vector<std::vector<unsigned int>>>(
      "pattern",
      {},
      "pattern>=0",
      "A double-indexed hexagonal-shaped array starting with the upper-left corner. '-1's are not "
      "selected as positions.");
  params.addParam<std::vector<unsigned int>>(
      "include_in_pattern", {}, "A vector of the numbers in the pattern to include");
  params.addParam<std::vector<std::vector<int>>>(
      "positions_pattern",
      {},
      "A double-indexed hexagonal-shaped array with the index of other Positions objects starting "
      "with the upper-left corner. These positions objects will be distributed within the "
      "hexagonal grid. The indexing is given in 'positions_pattern_indexing'. Use '-1' to discard "
      "a position in the lattice.");
  params.addParam<std::map<std::string, unsigned int>>(
      "positions_pattern_indexing",
      {},
      "A map from the name of positions objects to their index in the 'positions_pattern'");

  // Use user-provided ordering
  params.set<bool>("auto_sort") = false;
  // All functors defined on all processes for now
  params.set<bool>("auto_broadcast") = false;

  return params;
}

HexagonalGridPositions::HexagonalGridPositions(const InputParameters & parameters)
  : Positions(parameters),
    _center(getParam<Point>("center")),
    _lattice_flat_to_flat(getParam<Real>("lattice_flat_to_flat")),
    _pin_pitch(getParam<Real>("pin_pitch")),
    _z_axis_index(MooseEnum("X Y Z", "Z")),
    _nr(getParam<unsigned int>("nr")),
    _pattern(getParam<std::vector<std::vector<unsigned int>>>("pattern")),
    _include_in_pattern(
        std::set<unsigned int>(getParam<std::vector<unsigned int>>("include_in_pattern").begin(),
                               getParam<std::vector<unsigned int>>("include_in_pattern").end())),
    _positions_pattern(getParam<std::vector<std::vector<int>>>("positions_pattern")),
    _positions_pattern_indexing(
        getParam<std::map<std::string, unsigned int>>("positions_pattern_indexing"))
{
  // Check dimensions
  if (_nr == 1)
  {
    if (MooseUtils::absoluteFuzzyGreaterThan(_pin_pitch, _lattice_flat_to_flat))
      paramError("lattice_flat_to_flat",
                 "For one ring, the lattice flat to flat must be at least the pin pitch");
  }
  else
  {
    if (MooseUtils::absoluteFuzzyGreaterThan((3 * _nr - 1) * _pin_pitch / sqrt(3),
                                             _lattice_flat_to_flat))
      paramError("lattice_flat_to_flat",
                 "Lattice flat to flat distance is less than the minimum (3 * nr - 1) * pin_pitch "
                 "/ sqrt(3) given nr rings with a pitch of pin_pitch");
  }

  // Check pattern and include_in_pattern
  if ((_include_in_pattern.empty() && _pattern.size()) ||
      (_include_in_pattern.size() && _pattern.empty()))
    paramError(
        "include_in_pattern",
        "The 'pattern' parameter and the 'include_in_pattern' must be both specified or both not "
        "specified by the user.");
  for (const auto include : _include_in_pattern)
  {
    bool found = false;
    for (const auto & row : _pattern)
      if (std::find(row.begin(), row.end(), include) != row.end())
        found = true;
    if (!found)
      paramError("include_in_pattern",
                 "Pattern item '" + std::to_string(include) +
                     "' to include is not present in the pattern");
  }

  // Check positions_pattern and positions_pattern_indexing
  if ((_positions_pattern.empty() && !_positions_pattern_indexing.empty()) ||
      (!_positions_pattern.empty() && _positions_pattern_indexing.empty()))
    paramError("positions_pattern_indexing",
               "The 'positions_pattern' parameter and the 'positions_pattern_indexing' must be "
               "both specified or both not specified by the user.");
  for (const auto & [pos_name, index] : _positions_pattern_indexing)
  {
    bool found = false;
    for (const auto & row : _positions_pattern)
      if (std::find(row.begin(), row.end(), index) != row.end())
        found = true;
    if (!found)
      paramError("include_in_pattern",
                 "Pattern item '" + pos_name + "' with index '" + std::to_string(index) +
                     "' to include is not present in the pattern");
  }

  // Check incompatible parameters
  if (((_positions_pattern.size() && _positions_pattern[0].size()) ||
       _positions_pattern_indexing.size()) &&
      ((_pattern.size() && _pattern[0].size()) || _include_in_pattern.size()))
    paramError("positions_pattern",
               "'pattern'/'include_in_pattern' are not supported in combination with "
               "'positions_pattern/positions_pattern_indexing'. Only one pattern is supported");

  // Obtain the positions by unrolling the patterns
  initialize();
  // Sort if needed (user-specified)
  finalize();
}

void
HexagonalGridPositions::initialize()
{
  clearPositions();

  // We make very large pins so they cover the entire position
  _hex_latt = std::make_unique<HexagonalLatticeUtils>(
      _lattice_flat_to_flat, _pin_pitch, _pin_pitch, 0., 1., _nr, _z_axis_index);

  if (!isParamSetByUser("positions_pattern"))
  {
    // Unroll pattern
    std::vector<int> pattern_unrolled;

    if (_pattern.size())
    {
      // Check number of pins in pattern
      std::size_t pattern_size = 0;
      for (const auto & row : _pattern)
        pattern_size += row.size();
      if (_pattern.size() != cast_int<std::size_t>(2 * _nr - 1))
        mooseError("Number of rows in pattern (",
                   _pattern.size(),
                   ") should be equal to twice the number of hexagonal rings minus one");
      if (pattern_size != _hex_latt->totalPins(_nr))
        mooseError("Pattern size (",
                   pattern_size,
                   ") does not match the number of pins with ",
                   _nr,
                   " rings: ",
                   _hex_latt->totalPins(_nr));

      pattern_unrolled.resize(_hex_latt->totalPins(_nr));
      unsigned int i = 0;
      for (const auto r_i : make_range(_nr))
        for (const auto a_i : make_range(_hex_latt->pins(r_i + 1)))
        {
          libmesh_ignore(a_i);
          unsigned int row_i, within_row_i;
          _hex_latt->get2DInputPatternIndex(i, row_i, within_row_i);
          pattern_unrolled[i++] = _pattern[row_i][within_row_i];
        }
    }
    // just needs to be the right size
    else
      pattern_unrolled.resize(_hex_latt->totalPins(_nr), 0);

    // Count the number of positions we do not need to include
    unsigned int n_exclusions = 0;
    if (_include_in_pattern.size())
      for (const auto patt : pattern_unrolled)
        if (_include_in_pattern.count(patt) == 0)
          n_exclusions++;

    // Size array, remove the '-1' / not included positions
    const auto n_positions = cast_int<std::size_t>(_hex_latt->totalPins(_nr) - n_exclusions);
    _positions.resize(n_positions);

    // Fill the positions by retrieving the pin centers at indices included in the pattern (if
    // specified)
    unsigned int pos_i = 0;
    for (const auto patt_i : index_range(pattern_unrolled))
      if (!_pattern.size() || !_include_in_pattern.size() ||
          _include_in_pattern.count(pattern_unrolled[patt_i]))
        _positions[pos_i++] = _hex_latt->pinCenters()[patt_i];
  }
  else
  {
    // Unroll pattern into positions array
    const bool initial = _fe_problem.getCurrentExecuteOnFlag() == EXEC_INITIAL;

    // Check number of positions in pattern of nested positions
    unsigned pattern_size = 0;
    for (const auto & row : _positions_pattern)
      pattern_size += row.size();
    if (_positions_pattern.size() != cast_int<std::size_t>(2 * _nr - 1))
      mooseError("Number of rows in 'positions_pattern' (",
                 _pattern.size(),
                 ") should be equal to twice the number of hexagonal rings minus one");
    if (pattern_size != _hex_latt->totalPins(_nr))
      mooseError("Pattern size ",
                 pattern_size,
                 " does not match the number of pins with ",
                 _nr,
                 " rings: ",
                 _hex_latt->totalPins(_nr));

    // Check that all the positions names are valid
    unsigned num_pos = 0;
    for (const auto & [pos_name, index] : _positions_pattern_indexing)
      if (_fe_problem.hasUserObject(pos_name))
        num_pos += _fe_problem.getPositionsObject(pos_name).getNumPositions(initial);
    _positions.reserve(num_pos);

    // Invert map from positions to indices
    std::map<unsigned int, PositionsName> index_to_pos;
    for (const auto & [pos_name, index] : _positions_pattern_indexing)
      index_to_pos[index] = pos_name;

    // Unroll pattern : the positions vector is 1D and should be
    std::vector<PositionsName> pattern_unrolled;
    pattern_unrolled.resize(_hex_latt->totalPins(_nr));
    unsigned int i = 0;
    for (const auto r_i : make_range(_nr))
      for (const auto a_i : make_range(_hex_latt->pins(r_i + 1)))
      {
        libmesh_ignore(a_i);
        unsigned int row_i, within_row_i;
        _hex_latt->get2DInputPatternIndex(i, row_i, within_row_i);
        const auto pos_index = _positions_pattern[row_i][within_row_i];
        if (auto it = index_to_pos.find(cast_int<unsigned int>(pos_index));
            it != index_to_pos.end())
          pattern_unrolled[i++] = it->second;
        else if (pos_index != -1)
          paramError("positions_pattern",
                     "Index '" + std::to_string(pos_index) +
                         "' in pattern is not found in 'positions_pattern_indexing'");
        else
          pattern_unrolled[i++] = "-1";
      }

    // Now place the positions in the _positions array with the offset from the parent lattice index
    for (const auto patt_i : index_range(pattern_unrolled))
    {
      const auto & pos_name = pattern_unrolled[patt_i];
      if (pos_name != "-1")
        for (const auto & pos : _fe_problem.getPositionsObject(pos_name).getPositions(initial))
          _positions.push_back(_hex_latt->pinCenters()[patt_i] + pos);
    }
  }

  _initialized = true;
}
