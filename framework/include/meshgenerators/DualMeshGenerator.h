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
 * MeshGenerator for creating dual mesh
 */
class DualMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  DualMeshGenerator(const InputParameters & parameters);

  Point circumcenter(const Elem * elem);

  std::vector<Point> clipPolygonToElem(const std::vector<Point> & poly, const Elem * elem);

  std::vector<Point>
  clipPolygonToPhysicalBoundary(const std::vector<Point> & poly,
                                const std::vector<std::pair<Point, Point>> & boundary_segments);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;

  /// Vertex tolerance for determining colinearity of adjacent sides
  Real _boundary_node_angular_tol;

  // Vertex tolerance for determining if vertices are inside/outside of a boundary
  Real _boundary_edge_outside_tol;
};