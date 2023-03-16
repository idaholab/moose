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

class TestMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  TestMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

private:
  std::unique_ptr<MeshBase> * _return_mesh;
};
