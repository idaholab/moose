//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RepeatableRayStudyBase.h"

/**
 * Ray study that spawns Rays in a cone from a given set of starting points
 * for the cones and half angles for the cones.
 */
class ConeRayStudy : public RepeatableRayStudyBase
{
public:
  ConeRayStudy(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual void defineRays() override;

  /// The points to start the Rays from (the cone points)
  const std::vector<Point> _start_points;
  /// The directions that define the cones (points down the center of the cone)
  const std::vector<Point> _directions;
  /// Scaling factors for each cone's Rays (defaults to 1)
  const std::vector<Real> _scaling_factors;

  /// The half-cone angles in degrees for each cone
  const std::vector<Real> _half_cone_angles;
  /// The polar quadrature orders for each cone
  const std::vector<unsigned int> _polar_quad_orders;
  /// The azimuthal quadrature orders for each cone
  const std::vector<unsigned int> _azimuthal_quad_orders;

  /// The index into the Ray's data for storing the angular quadrature weight and scaling factor
  const RayDataIndex _ray_data_index;
};
