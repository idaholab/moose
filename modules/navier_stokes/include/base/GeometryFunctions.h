//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/**
 * Compute the radial coordinate
 * @param p      point
 * @param center center coordinate of the bed
 * @param axis   vertical axis of the bed
 */
Real
computeRadialCoordinate(const Point & p, const Point & center, const int & axis);

/**
 * Get the coordinate along a particular axis
 * @param  p    point
 * @param  axis axis
 * @return point along axis
 */
Real
getCoordinate(const Point & p, const int & axis);

/**
 * Compute displacement above a point along a particular axis
 * @param p          point
 * @param axis       axis
 * @param bed_bottom point from which to compute distance
 * @return displacement from point along axis relative to bed_bottom
 */
Real
computeAxialDistanceAbove(const Point & p, int axis, Real bed_bottom);

/**
 * Compute displacement below a point along a particular axis
 * @param p          point
 * @param axis       axis
 * @param bed_top    point from which to compute distance
 * @return displacement from point along axis relative to bed_top
 */
Real
computeAxialDistanceBelow(const Point & p, int axis, Real bed_top);

Real
computeMinRadialDistance(const Point & point, const Point & center,
  const int & axis, const Real & inner_radius, const Real & outer_radius);

/**
 * Compute minimum distance to an axial boundary
 * @param p          point
 * @param axis       axis
 * @param bed_bottom lower bound of bed along axis
 * @param bed_top    upper bound of bed along axis
 * @return minimum distance to an axial boundary
 */
Real
computeMinAxialDistance(const Point & p, int axis, const Real & bed_bottom, const Real & bed_top);

Real
elementSize(const Elem * e);
