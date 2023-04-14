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
 * But they are limited in to vectors or multi-dimensional vectors of points.
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
  const std::vector<Point> & getPositions(bool initial = false) const;
  const std::vector<std::vector<Point>> & getPositionsVector2D() const;
  const std::vector<std::vector<std::vector<Point>>> & getPositionsVector3D() const;
  const std::vector<std::vector<std::vector<std::vector<Point>>>> & getPositionsVector4D() const;
  ///}

protected:
  /// In charge of computing / loading the positions.
  virtual void initialize() override = 0;

  /// By default, we wont execute often but "executing" will mean loading the positions
  void execute() override { initialize(); }
  void finalize() override {}

  /// By default, Positions will call initial setup on mesh changed
  void meshChanged() override { initialSetup(); };

  /// By default, Positions will not be modified very regularly
  void timestepSetup() override {}
  void residualSetup() override {}
  void jacobianSetup() override {}

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
};

void dataStore(std::ostream & stream, const std::vector<Point> & positions, void * context);
void dataLoad(std::istream & stream, const std::vector<Point> & positions, void * context);
