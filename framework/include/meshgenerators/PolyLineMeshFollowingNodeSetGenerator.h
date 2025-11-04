//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * by marching along a nodeeset and trying to be as close as possible to
 * the nodes of the nodeset
 */
class PolyLineMeshFollowingNodeSetGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  PolyLineMeshFollowingNodeSetGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  // Input mesh with the nodeset
  std::unique_ptr<MeshBase> & _input;

  /// Starting point of the polyline
  const Point _starting_point;
  /// Starting direction for the polyline
  const Point _starting_direction;
  /// Whether edges should form a closed loop. Will error if the nodeset does not loop back on itself
  const bool _loop;

  /// Subdomain name to assign to the polyline edge elements
  const SubdomainName _line_subdomain;
  /// Boundary names to assign to (non-looped) polyline start and end
  const BoundaryName _start_boundary, _end_boundary;

  /// Approximate spacing between nodes
  const Real _dx;
  /// How many Edge elements to build between each point pair
  const unsigned int _num_edges_between_points;
  /// Whether to output to console the mesh generation process
  const bool _verbose;
};
