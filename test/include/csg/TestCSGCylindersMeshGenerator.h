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

  void generateData() override{};

  std::unique_ptr<CSG::CSGBase> generateCSG() override;

protected:
  /// radius of cylinder
  const Real _radius;
  /// center of cylinder (x, y) for z aligned, (y, z) for x aligned, (x, z) for y aligned
  const Real _x0;
  const Real _x1;
  /// axis: x, y, or z
  const std::string _axis;
};
