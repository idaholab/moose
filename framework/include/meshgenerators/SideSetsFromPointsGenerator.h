//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideSetsGeneratorBase.h"

/**
 * A mesh generator to generate new sidesets from all faces connected to points
 * with the same normal as the face the point is part of
 */
class SideSetsFromPointsGenerator : public SideSetsGeneratorBase
{
public:
  static InputParameters validParams();

  SideSetsFromPointsGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// holds the points used to generate sidesets
  std::vector<Point> _points;
};
