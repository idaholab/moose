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

class TestOneToManyDependencyCombinerGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  TestOneToManyDependencyCombinerGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  void generateData() override {};

  std::unique_ptr<CSG::CSGBase> generateCSG() override;

protected:
  /// Pointer to the input mesh
  std::vector<std::unique_ptr<MeshBase> *> _mesh_ptr;
  /// List of pointers to all CSG bases created by input mesh generators
  std::vector<std::unique_ptr<CSG::CSGBase> *> _build_csg_bases;
};
