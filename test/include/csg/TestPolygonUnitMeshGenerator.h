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

class TestPolygonUnitMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  TestPolygonUnitMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  void generateData() override {};

  std::unique_ptr<CSG::CSGBase> generateCSG() override;

protected:
  /// the apothem length of the polygon
  const Real _apothem;
  /// the number of sides
  const int _num_sides;
  /// whether or not to expand the final polygon unit into regular surfaces
  bool _expand;
};
