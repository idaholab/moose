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

class TestMeshDataDeclareGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  TestMeshDataDeclareGenerator(const InputParameters & parameters);
  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;
  Real & _mesh_data_value;
};

class TestMeshDataGetGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  TestMeshDataGetGenerator(const InputParameters & parameters);
  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;
  const Real & _mesh_data_value;
};
