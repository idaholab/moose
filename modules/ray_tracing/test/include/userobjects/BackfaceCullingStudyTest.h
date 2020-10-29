//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LotsOfRaysRayStudy.h"

/**
 * A test study that tests the use of backface culling in TraceRay.
 *
 * This functionality is for advanced use where large numbers of rays
 * need to be traced. It is disabled by default because it requires a fast
 * caching method for the normals for all elements that has not been
 * implemented in the ray tracing module yet.
 */
class BackfaceCullingStudyTest : public LotsOfRaysRayStudy
{
public:
  BackfaceCullingStudyTest(const InputParameters & parameters);

  static InputParameters validParams();

  void initialSetup() override;
  void meshChanged() override;

  const Point * getElemNormals(const Elem * elem, const THREAD_ID tid) override;

private:
  virtual void generateNormals();

  std::unordered_map<const Elem *, std::vector<Point>> _normals;
};
