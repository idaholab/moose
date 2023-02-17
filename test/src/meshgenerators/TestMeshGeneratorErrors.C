//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestMeshGeneratorErrors.h"

registerMooseObject("MooseTestApp", TestMeshGeneratorErrors);

InputParameters
TestMeshGeneratorErrors::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addParam<bool>("dummy", false, "Dummy parameter");

  params.addParam<bool>("get_mesh_invalid", false, "Tests the getMesh invalid parameter error");
  params.addParam<bool>("get_mesh_wrong_type", false, "Tests the getMesh wrong type error");
  params.addParam<bool>(
      "get_meshes_by_name_invalid", false, "Tests the getMeshesByName invalid parameter error");
  params.addParam<bool>(
      "get_meshes_by_name_wrong_type", false, "Tests the getMeshesByName wrong type error");

  return params;
}

TestMeshGeneratorErrors::TestMeshGeneratorErrors(const InputParameters & parameters)
  : MeshGenerator(parameters)
{
  // static casts here are needed because getMesh is [[nodiscard]]
  if (getParam<bool>("get_mesh_invalid"))
    static_cast<void>(getMesh("invalid"));
  if (getParam<bool>("get_mesh_wrong_type"))
    static_cast<void>(getMesh("dummy"));
  if (getParam<bool>("get_meshes_by_name_invalid"))
    static_cast<void>(getMeshes("invalid"));
  if (getParam<bool>("get_meshes_by_name_wrong_type"))
    static_cast<void>(getMeshes("dummy"));
}
