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

class NestedLatticeCellUniverseMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  NestedLatticeCellUniverseMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  void generateData() override {};

  std::unique_ptr<CSG::CSGBase> generateCSG() override;

protected:
  /// radius of inner sphere surface
  const Real _inner_rad;
  /// radius of outer sphere surface
  const Real _outer_rad;
  /// radius of lattice sphere surface
  const Real _lattice_rad;
};
