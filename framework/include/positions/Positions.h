//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "GeneralReporter.h"

/**
 * Positions objects are under the hood Reporters.
 * But they are limited to vectors or multi-dimensional vectors of points.
 */
class Positions : public GeneralReporter
{
public:
  static InputParameters validParams();
  Positions(const InputParameters & parameters);
  virtual ~Positions() = default;

  ///{
  /// Getters for the positions vector for the desired dimension
  /// 1D will be the only one guaranteed to succeed
  const std::vector<Point> & getPositions(bool initial) const;
  const std::vector<std::vector<Point>> & getPositionsVector2D() const;
  const std::vector<std::vector<std::vector<Point>>> & getPositionsVector3D() const;
  const std::vector<std::vector<std::vector<std::vector<Point>>>> & getPositionsVector4D() const;
  ///}

  /// Getter for a single position at a known index
  const Point & getPosition(unsigned int index, bool initial) const;

  /// Find the nearest Position for a given point
  const Point & getNearestPosition(const Point & target, bool initial) const;

protected:
  /// In charge of computing / loading the positions.
  virtual void initialize() override = 0;

  /// By default, we wont execute often but "executing" will mean loading the positions
  virtual void execute() override { initialize(); }

  /// In charge of reduction across all ranks & sorting for consistent output
  virtual void finalize() override;

  /// By default, Positions will call initial setup on mesh changed
  virtual void meshChanged() override { initialSetup(); }

  /// By default, Positions will not be modified very regularly
  virtual void timestepSetup() override {}
  virtual void residualSetup() override {}
  virtual void jacobianSetup() override {}

  /// Clear all positions vectors
  void clearPositions();

  /// Unrolls the multi-dimensional position vectors
  void unrollMultiDPositions();

  /// For initialization of the positions, another position reporter may be used
  const std::vector<Point> * const _initial_positions;

  /// For now, only the 1D vector will be shared across all ranks. All the others only exist
  /// locally.
  /// The 4 dimensions could be used for xyzt, but could also be axial/assembly/pin/ring
  /// 1D storage for all the positions
  std::vector<Point> & _positions;

  /// 2D storage for all the positions
  std::vector<std::vector<Point>> _positions_2d;

  /// 3D storage for all the positions
  std::vector<std::vector<std::vector<Point>>> _positions_3d;

  /// 4D storage for all the positions : space & time
  std::vector<std::vector<std::vector<std::vector<Point>>>> _positions_4d;

  /// Whether generation of positions is distributed or not (and therefore needs a broadcast)
  bool _need_broadcast;

  /// Whether positions should be sorted. Be careful with sorting! For example if initial positions
  /// are not sorted, then we switch to sorted positions, the mapping might be odd
  /// User may have also sorted their positions file, their input parameter etc
  const bool _need_sort;
};
