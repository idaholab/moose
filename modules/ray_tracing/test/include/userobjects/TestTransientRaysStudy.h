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
 * Tests transient rays.
 *
 * This will start rays on a boundary initially and then let them
 * trace to a maximum distance, set by a function. On subsequent
 * calls to execute, the same rays are used and the maximum distance
 * function is resampled.
 */
class TestTransientRaysStudy : public RayTracingStudy
{
public:
  TestTransientRaysStudy(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual void generateRays() override;
  virtual void postExecuteStudy() override;

private:
  const Function & _distance_function;
  const BoundaryID _boundary_id;
  bool & _generated_rays;
  std::vector<std::shared_ptr<Ray>> & _banked_rays;
};
