//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RayTracingStudy.h"

/**
 * Test study for generating rays for a basic particle-in-cell capability,
 * where Rays propagate a bit each time step
 */
class TestPICRayStudy : public RayTracingStudy
{
public:
  TestPICRayStudy(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void generateRays() override;

protected:
  virtual void postExecuteStudy() override;

private:
  /**
   * @return The max distance the given ray should travel at the current time
   *
   * Uses the current ray position to sample the velocity field in \p velocity_function
   */
  Real maxDistance(const Ray & ray) const;

  /// The starting points
  const std::vector<Point> & _start_points;

  /// The starting directions
  const std::vector<Point> & _start_directions;

  /// The function that represents the velocity field
  const Function & _velocity_function;

  /// Whether or not we've generated rays yet (restartable)
  bool & _has_generated;

  /// The banked rays to be used on the next timestep (restartable)
  std::vector<std::shared_ptr<Ray>> & _banked_rays;
};
