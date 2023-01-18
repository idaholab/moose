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

class TestMeshMetaData : public MeshGenerator
{
public:
  static InputParameters validParams();

  TestMeshMetaData(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  const MeshGeneratorName _input;

  std::unique_ptr<MeshBase> & _input_mesh;

  template <class T>
  void checkMeshMetaData(const std::string mesh_data_name, const T ref_data);
};
