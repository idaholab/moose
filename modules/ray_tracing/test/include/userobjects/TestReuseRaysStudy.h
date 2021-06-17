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
 * Tests the re-use of banked rays.
 *
 * This will take the banked rays after execution
 * and store them so that they can be re-traced
 * in the opposite direction.
 */
class TestReuseRaysStudy : public RayTracingStudy
{
public:
  TestReuseRaysStudy(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual void generateRays() override;
  virtual void postExecuteStudy() override;

private:
  bool & _executed_once;
  std::vector<std::shared_ptr<Ray>> & _banked_rays;
};
