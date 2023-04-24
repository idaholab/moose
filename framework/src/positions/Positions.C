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

  // leverage reporter interface to keep track of consumers
  params.addParam<ReporterName>("initial_positions",
                                "Positions at the beginning of the simulation");

  // This parameter should be set by each derived class depending on whether the generation of
  // positions is replicated or distributed. We want positions to be replicated across all ranks
  params.addRequiredParam<bool>("auto_broadcast",
                                "Wether Positions should be broadcasted across all ranks");
  // This parameter should be set by each derived class depending on whether positions should
  // be sorted after being communicated. Sorting is usually undesirable, as the user input order
  // should be respected. It may be only desirable for testing reproducibility
  params.addRequiredParam<bool>("auto_sort", "Whether Positions should be sorted by coordinates");

  // In general, no need to refresh Positions unless the mesh moved
  params.set<ExecFlagEnum>("execute_on") = EXEC_NONE;

  params.addParamNamesToGroup("auto_broadcast auto_sort", "Advanced");
  params.registerBase("Positions");
  return params;
}

Positions::Positions(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _initial_positions(isParamValid("initial_positions")
                           ? &getReporterValue<std::vector<Point>>("initial_positions")
                           : nullptr),
    // Positions will be replicated on every rank so transfers may query positions from all ranks
    _positions(declareValueByName<std::vector<Point>, ReporterVectorContext<Point>>(
        "positions_1d", REPORTER_MODE_REPLICATED)),
    _need_broadcast(getParam<bool>("auto_broadcast")),
    _need_sort(getParam<bool>("auto_sort"))
{
}

const Point &
Positions::getPosition(unsigned int index, bool initial) const
{
  // Check sizes of inital positions
  if (initial && _initial_positions && (*_initial_positions).size() < index)
    mooseError("Initial positions is not sized or initialized appropriately");

  // Check sizes of regular positions
  if (!initial)
  {
    if (_initial_positions && _positions.size() != (*_initial_positions).size())
      mooseError("Initial positions and current positions array length do not match");
    else if (_positions.size() < index)
      mooseError("Positions retrieved with an out-of-bound index");
  }

  if (initial && _initial_positions)
    return (*_initial_positions)[index];
  if (_positions.size())
    return _positions[index];
  else
    mooseError("Positions vector has not been initialized.");
}

const Point &
Positions::getNearestPosition(const Point & target) const
{
  // TODO Use faster & fancier machinery such as a KNN-partition
  unsigned int nearest_index = 0;
  Real nearest_distance_sq = std::numeric_limits<float>::max();
  for (const auto & pt : _positions)
    if ((pt - target).norm_sq() < nearest_distance_sq)
    {
      nearest_index = &pt - &_positions[0];
      nearest_distance_sq = (pt - target).norm_sq();
    }

#ifndef NDEBUG
  // Check that no two positions are equidistant to the target
  for (const auto & pt : _positions)
    if (MooseUtils::absoluteFuzzyEqual((pt - target).norm_sq(), nearest_distance_sq) &&
        (nearest_index != (&pt - &_positions[0])))
      mooseWarning("Search for nearest position found several matches: " +
                       Moose::stringify(_positions[&pt - &_positions[0]]) + " and " +
                       Moose::stringify(_positions[nearest_index]),
                   " at a distance of " + std::to_string(std::sqrt(nearest_distance_sq)));
#endif

  return _positions[nearest_index];
}

const std::vector<Point> &
Positions::getPositions(bool initial) const
{
  if (initial && _initial_positions)
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

void
Positions::finalize()
{
  // Gather up the positions vector on all ranks

  if (_need_broadcast)
  {
    // TODO: be able to do this
    // _communicator.allgather(_positions, /* identical buffer lengths = */ false);

    // Other notes to explain why we do this:
    // - The vector broadcasting code is for VPPs, so it's *currently* not accessible as we have
    //   vectors of points, not reals
    // - The consumer/producer reporter interface can keep track of whether a reduction is needed
    //   (for example if a consumer needs replicated data, but the producer is distributed) however,
    //   we have currently made the decision that positions should ALWAYS be replicated

    // Gather all components of all positions
    std::vector<Real> x(_positions.size()), y(_positions.size()), z(_positions.size());
    for (const auto & pt : _positions)
    {
      x[&pt - &_positions[0]] = pt(0);
      y[&pt - &_positions[0]] = pt(1);
      z[&pt - &_positions[0]] = pt(2);
    }

    _communicator.allgather(x, /* identical buffer lengths = */ false);
    _communicator.allgather(y, /* identical buffer lengths = */ false);
    _communicator.allgather(z, /* identical buffer lengths = */ false);

    _positions.resize(x.size());
    for (auto & pt : _positions)
      pt = Point(x[&pt - &_positions[0]], y[&pt - &_positions[0]], z[&pt - &_positions[0]]);
  }

  // Sort positions by X then Y then Z
  if (_need_sort)
    std::sort(_positions.begin(), _positions.end());
}
