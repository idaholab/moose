//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

class TestCSGCylindersMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  TestCSGCylindersMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  void generateData() override {};

  std::unique_ptr<CSG::CSGBase> generateCSG() override;

protected:
  /// radii of cylinders
  const std::vector<Real> _radii;
  /// height of cylinder
  const Real _h;
  /// First coordinate of center of cylinder
  const Real _x0;
  /// Second coordinate of center of cylinder
  const Real _x1;
  /// axis: x, y, or z
  const std::string _axis;
};
