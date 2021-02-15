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

#include "Ray.h"

/**
 * A RayTracingStudy in which the user defines a set of Rays that can be traced repeatedly.
 */
class RepeatableRayStudy : public RepeatableRayStudyBase
{
public:
  RepeatableRayStudy(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual void defineRays() override;

  /// The Ray names
  const std::vector<std::string> _names;
  /// The points to start the Rays from
  const std::vector<Point> _start_points;
  /// The Ray end points (if defined)
  const std::vector<Point> * const _end_points;
  /// The Ray directions (if defined)
  const std::vector<Point> * const _directions;
  /// The Ray max distances (if defined)
  const std::vector<Real> * const _max_distances;
  /// The Ray data indices (if defined)
  const std::vector<RayDataIndex> _ray_data_indices;
  /// The initial Ray data to set (if defined)
  const std::vector<std::vector<Real>> * const _initial_ray_data;
  /// The Ray aux data indices (if defined)
  const std::vector<RayDataIndex> _ray_aux_data_indices;
  /// The initial Ray data to set (if defined)
  const std::vector<std::vector<Real>> * const _initial_ray_aux_data;
};
