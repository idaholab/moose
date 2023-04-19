//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Positions.h"

InputParameters
Positions::validParams()
{
  InputParameters params = GeneralReporter::validParams();

  params.addParam<ReporterName>("initial_positions",
                                "Positions at the beginning of the simulation");
  // No need to refresh unless the mesh moved
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  params.registerBase("Positions");
  return params;
}

Positions::Positions(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _initial_positions(isParamValid("initial_positions")
                           ? &getReporterValue<std::vector<Point>>("initialPositions")
                           : nullptr),
    _positions(declareValueByName<std::vector<Point>, ReporterVectorContext<Point>>(
        "positions_1d", REPORTER_MODE_REPLICATED))
{
}

const Point &
Positions::getPosition(unsigned int index, bool initial) const
{
  mooseAssert(!initial || (_initial_positions && (*_initial_positions).size() < index),
              "Initial positions is not sized or initialized appropriately");
  mooseAssert(_positions.size() > index, "Positions retrieved with an out-of-bound index");
  if (initial)
    return (*_initial_positions)[index];
  if (_positions.size())
    return _positions[index];
  else
    mooseError("Positions vector has not been initialized.");
}

const std::vector<Point> &
Positions::getPositions(bool initial) const
{
  if (initial)
    return *_initial_positions;
  if (_positions.size())
    return _positions;
  else
    mooseError("Positions vector has not been initialized.");
}

const std::vector<std::vector<Point>> &
Positions::getPositionsVector2D() const
{
  if (_positions_2d.size())
    return _positions_2d;
  else
    mooseError("2D positions vectors have not been initialized.");
}

const std::vector<std::vector<std::vector<Point>>> &
Positions::getPositionsVector3D() const
{
  if (_positions_3d.size())
    return _positions_3d;
  else
    mooseError("3D positions vectors have not been initialized.");
}

const std::vector<std::vector<std::vector<std::vector<Point>>>> &
Positions::getPositionsVector4D() const
{
  if (_positions_4d.size())
    return _positions_4d;
  else
    mooseError("4D positions vectors have not been initialized.");
}

void
Positions::clearPositions()
{
  _positions.clear();
  _positions_2d.clear();
  _positions_3d.clear();
  _positions_4d.clear();
}

void
Positions::unrollMultiDPositions()
{
  // Unroll in every dimension available
  std::vector<Point> temp_2d_unrolled;
  std::vector<Point> temp_3d_unrolled;
  std::vector<Point> temp_4d_unrolled;
  for (auto vec : _positions_2d)
    temp_2d_unrolled.insert(temp_2d_unrolled.end(), vec.begin(), vec.end());
  for (auto vec_vec : _positions_3d)
    for (auto vec : vec_vec)
      temp_3d_unrolled.insert(temp_3d_unrolled.end(), vec.begin(), vec.end());
  for (auto vec_vec_vec : _positions_4d)
    for (auto vec_vec : vec_vec_vec)
      for (auto vec : vec_vec)
        temp_4d_unrolled.insert(temp_4d_unrolled.end(), vec.begin(), vec.end());

  // for now we wont even tolerate a different ordering
  // 2D & 1D: check for conflicts
  if (temp_2d_unrolled.size() && _positions.size() && temp_2d_unrolled != _positions)
    mooseError("Inconsistency between the 2D and 1D position vectors detected. "
               "The 2D positions must unroll into the 1D positions");

  // 3D vs 2D & 1D
  if (temp_3d_unrolled.size() && temp_2d_unrolled.size() && temp_3d_unrolled != temp_3d_unrolled)
    mooseError("Inconsistency between the 3D and 2D position vectors detected. "
               "The 3D positions must unroll the same way as the 2D positions");
  if (temp_3d_unrolled.size() && _positions.size() && temp_3d_unrolled != _positions)
    mooseError("Inconsistency between the 3D and 1D position vectors detected. "
               "The 3D positions must unroll into the 1D positions");

  // 4D vs all lower dimensions
  if (temp_4d_unrolled.size() && temp_3d_unrolled.size() && temp_4d_unrolled != temp_3d_unrolled)
    mooseError("Inconsistency between the 4D and 3D position vectors detected. "
               "The 4D positions must unroll the same way as the 3D positions");
  if (temp_4d_unrolled.size() && temp_2d_unrolled.size() && temp_4d_unrolled != temp_2d_unrolled)
    mooseError("Inconsistency between the 4D and 2D position vectors detected. "
               "The 4D positions must unroll the same way as the 2D positions");
  if (temp_4d_unrolled.size() && _positions.size() && temp_4d_unrolled != _positions)
    mooseError("Inconsistency between the 4D and 1D position vectors detected. "
               "The 4D positions must unroll into the 1D positions");

  // Use any of the higher D to set the one D vector
  if (!_positions.size())
  {
    if (temp_2d_unrolled.size())
      _positions = temp_2d_unrolled;
    else if (temp_3d_unrolled.size())
      _positions = temp_3d_unrolled;
    else if (temp_4d_unrolled.size())
      _positions = temp_4d_unrolled;
    else
      mooseError("Positions::unrollMultiDPositions() may only be called if there is at least one "
                 "non-empty positions vector.");
  }
}