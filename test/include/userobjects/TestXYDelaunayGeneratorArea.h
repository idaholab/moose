//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

/**
 * Test that the areas of TRI elements built with XYDelaunayGenerator
 * are restricted by a given area function. That is, the volumes of
 * each TRI are less than a value evaluated by a function at each
 * of their centroids.
 */
class TestXYDelaunayGeneratorArea : public ElementUserObject
{
public:
  static InputParameters validParams();
  TestXYDelaunayGeneratorArea(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;

protected:
  /// The function evaluated at each TRI centroid to restrict area
  const Function & _area_func;
  /// The failures (element centroid, area, required area)
  std::vector<std::tuple<Point, float, float>> _failures;
};
