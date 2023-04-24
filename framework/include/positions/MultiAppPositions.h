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
#include "Positions.h"

/**
 * Positions from all the MultiApps
 */
class MultiAppPositions : public Positions
{
public:
  static InputParameters validParams();
  MultiAppPositions(const InputParameters & parameters);
  virtual ~MultiAppPositions() = default;

  // MultiApp positions are not known at construction
  void initialize() override;
  using Positions::_positions;

  /// Whether to use the subapp mesh centroids to compute the positions, in addition to the positions
  const bool _use_apps_centroid;
};
