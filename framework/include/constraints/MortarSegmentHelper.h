//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// TODO: fix tolerancing issue better
class MortarSegmentHelper
{
public:
  MortarSegmentHelper(const Elem * secondary_elem_ptr, Point & center, Point & normal);

  Point
  getIntersection(Point & p1, Point & p2, Point & q1, Point & q2, Real & s);

  // Check that a point is indeed inside (or on) the secondary polygon
  bool
  isInsideSecondary(Point & pt);


  bool
  isDisjoint(std::vector<Point> & poly);


  void
  clipPoly(const Elem * primary_elem_ptr, std::vector<Point> & clipped_poly);


  void
  plotPoly(std::vector<Point> & poly);


  void
  plotTriangulation(std::vector<Point> & nodes, std::vector<std::array<int, 3>> & elem_to_nodes);


  void
  triangulatePoly(std::vector<Point> & nodes, std::vector<std::array<int, 3>> & tri_map);


  void
  getMortarSegments(const Elem * primary_elem_ptr, std::vector<Point> & nodes, std::vector<std::array<int, 3>> & elem_to_nodes);

private:
  /**
   * Pointer to secondary element this object will operate on
   */
  const Elem * _secondary_elem_ptr;

  /**
   * Geometric center of secondary element
   */
  Point _center;

  /**
   * Normal at geometric center of secondary element
   */
  Point _normal;

  /**
   * Vectors orthogonal to normal that define the plane projection will be performed on.
   * These vectors are used to project the polygon clipping problem on a 2D plane,
   * they are defined so the nodes of the projected polygon are listed with positive orientation
   */
  Point _u, _v;
  bool _debug;

  /**
   * List of projected points on the linearized secondary element
   */
  std::vector<Point> _secondary_poly;
};
