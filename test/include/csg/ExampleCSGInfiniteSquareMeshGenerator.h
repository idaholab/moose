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

class ExampleCSGInfiniteSquareMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  ExampleCSGInfiniteSquareMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  void generateData() override {};

  std::unique_ptr<CSG::CSGBase> generateCSG() override;

protected:
  /// the side length of the infinite square
  const Real _side_length;
};
