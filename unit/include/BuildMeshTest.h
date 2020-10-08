//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest_include.h"

#include "MeshGenerator.h"

class MooseApp;
class Factory;
class MooseMesh;
class InputParameters;

class BuildMeshTest : public ::testing::Test
{
protected:
  void SetUp() override;

  std::shared_ptr<MooseApp> _app;
  std::shared_ptr<MooseMesh> _moose_mesh;
  std::shared_ptr<MeshGenerator> _mesh_gen;
  Factory * _factory;
};

class BuildMeshBaseTypesGenerator : public MeshGenerator
{
public:
  static InputParameters validParams() { return MeshGenerator::validParams(); }

  BuildMeshBaseTypesGenerator(const InputParameters & params) : MeshGenerator(params) {}

  std::unique_ptr<MeshBase> generate() override final;
};
