//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PolygonMeshGeneratorBase.h"

/**
 * Generate a polyline mesh that is based on an input 2D-XY mesh. The 2D-XY mesh needs to be a
 * connected mesh with only one outer boundary manifold. The polyline mesh generated along with the
 * boundary of the input mesh form a gap with a specified thickness.
 */
class GapMeshGenerator : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  GapMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Input mesh defining the boundary
  std::unique_ptr<MeshBase> & _input;

  /// The thickness of the gap to be created
  const Real _thickness;

  /**
   * Check if three points are collinear.
   * @param p1 First point
   * @param p2 Second point
   * @param p3 Third point
   * @return true if the three points are collinear, false otherwise
   */
  bool isPointsColinear(const Point & p1, const Point & p2, const Point & p3) const;

  /**
   * Check if the line segment p1-p2 intersects with line segment p3-p4
   * @param p1 First point of first line segment
   * @param p2 Second point of first line segment
   * @param p3 First point of second line segment
   * @param p4 Second point of second line segment
   * @return true if the two line segments intersect, false otherwise
   */
  bool
  fourPointOverlap(const Point & p1, const Point & p2, const Point & p3, const Point & p4) const;
};
