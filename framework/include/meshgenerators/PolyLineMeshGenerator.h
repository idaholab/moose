//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

/**
 * Generates a polyline (open ended or looped) of Edge elements
 * through a series of nodal locations and other input parameters.
 */
class PolyLineMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  PolyLineMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// The points defining the polyline, in order
  const std::vector<Point> _points;

  /// Whether edges should form a closed loop
  const bool _loop;

  /// Boundary names to assign to (non-looped) polyline start and end
  const BoundaryName _start_boundary, _end_boundary;

  /// How many Edge elements to build between each point pair
  const unsigned int _num_edges_between_points;
};
