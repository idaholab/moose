//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestMeshGenerator.h"

registerMooseObject("MooseTestApp", TestMeshGenerator);

InputParameters
TestMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addParam<MeshGeneratorName>("input", "Input mesh for testing");
  params.addParam<std::vector<MeshGeneratorName>>("inputs", "Input meshes for testing");
  params.addParam<MeshGeneratorName>("invalid_input", "Input that isn't set");

  params.addParam<bool>("dummy", false, "Dummy parameter");

  params.addParam<bool>("get_mesh_invalid", false, "Tests the getMesh invalid parameter error");
  params.addParam<bool>("get_mesh_wrong_type", false, "Tests the getMesh wrong type error");
  params.addParam<bool>(
      "get_meshes_by_name_invalid", false, "Tests the getMeshesByName invalid parameter error");
  params.addParam<bool>(
      "get_meshes_by_name_wrong_type", false, "Tests the getMeshesByName wrong type error");
  params.addParam<bool>(
      "get_mesh_outside_construct", false, "Tests getting a mesh outside of construction");
  params.addParam<bool>(
      "declare_mesh_prop_outside_construct", false, "Declare a mesh property out of construction");
  params.addParam<bool>(
      "sub_outside_construct", false, "Tests building a subgenerator outside of construction");
  params.addParam<std::string>("null_mesh_name", "Declares a null mesh with this name");
  params.addParam<bool>(
      "mesh_prop_double_declare", false, "Tests declaring the same mesh property twice");
  params.addParam<bool>("sub_no_declare_input",
                        false,
                        "Tests getting an input from a sub generator but not declaring it");
  params.addParam<bool>("input_not_moved", false, "Tests when an input mesh is not moved");
  params.addParam<bool>("request_input", false, "Request the mesh in 'input'");
  params.addParam<bool>("request_inputs", false, "Request the meshes in 'input'");
  params.addParam<bool>("request_input_by_name", false, "Request the mesh in 'input' by name");
  params.addParam<bool>("request_inputs_by_name", false, "Request the meshes in 'inputs' by name");
  params.addParam<bool>("request_sub_input", false, "Request the mesh in 'input' for a sub");
  params.addParam<bool>("missing_get", false, "Tests getting a missing meshgenerator from the app");
  params.addParam<std::string>("add_sub_input", "Adds this input as an input to a sub generator");

  return params;
}

TestMeshGenerator::TestMeshGenerator(const InputParameters & parameters) : MeshGenerator(parameters)
{
  if (getParam<bool>("get_mesh_invalid"))
    static_cast<void>(getMesh("invalid"));
  if (getParam<bool>("get_mesh_wrong_type"))
    static_cast<void>(getMesh("dummy"));
  if (getParam<bool>("get_meshes_by_name_invalid"))
    static_cast<void>(getMeshes("invalid"));
  if (getParam<bool>("get_meshes_by_name_wrong_type"))
    static_cast<void>(getMeshes("dummy"));
  if (getParam<bool>("request_input"))
    static_cast<void>(getMesh("input"));
  if (getParam<bool>("request_inputs"))
    static_cast<void>(getMeshes("inputs"));
  if (getParam<bool>("request_input_by_name"))
    static_cast<void>(getMeshByName(getParam<MeshGeneratorName>("input")));
  if (getParam<bool>("request_inputs_by_name"))
    static_cast<void>(getMeshesByName(getParam<std::vector<MeshGeneratorName>>("inputs")));
  if (getParam<bool>("request_sub_input"))
    declareMeshForSub("input");
  if (getParam<bool>("mesh_prop_double_declare"))
  {
    static_cast<void>(declareMeshProperty<bool>("foo"));
    static_cast<void>(declareMeshProperty<bool>("foo"));
  }
  if (getParam<bool>("missing_get"))
    _app.getMeshGenerator("foo");

  if (isParamValid("null_mesh_name"))
  {
    const auto & null_mesh_name = getParam<std::string>("null_mesh_name");

    // this will get us a ref to _null_mesh in MeshGenerator
    const auto & null_mesh_ptr = getMesh("invalid_input", true);

    declareNullMeshName(null_mesh_name);

    const auto & should_be_null_by_param = getMesh("input");
    if (&should_be_null_by_param != &null_mesh_ptr)
      mooseError("Should be null by param failed");

    const auto & should_be_null_by_name = getMeshByName(null_mesh_name);
    if (&should_be_null_by_name != &null_mesh_ptr)
      mooseError("Should be null by name failed");

    const auto & should_be_null_by_vector_param = getMeshes("inputs");
    if (&*should_be_null_by_vector_param[0] != &null_mesh_ptr)
      mooseError("Should be null by vector param failed");

    declareMeshForSubByName(null_mesh_name);
    declareMeshesForSubByName({null_mesh_name});
  }

  if (getParam<bool>("sub_no_declare_input") || isParamValid("add_sub_input"))
  {
    auto params = _app.getFactory().getValidParams("RenameBlockGenerator");
    if (getParam<bool>("sub_no_declare_input"))
      params.set<MeshGeneratorName>("input") = getParam<MeshGeneratorName>("input");
    else
      params.set<MeshGeneratorName>("input") = getParam<std::string>("add_sub_input");
    params.set<std::vector<SubdomainName>>("old_block") = {"0"};
    params.set<std::vector<SubdomainName>>("new_block") = {"1"};
    addMeshSubgenerator("RenameBlockGenerator", name() + "_rbg", params);
  }

  // So that we have something to return
  {
    auto params = _app.getFactory().getValidParams("GeneratedMeshGenerator");
    params.set<MooseEnum>("dim") = "1";
    addMeshSubgenerator("GeneratedMeshGenerator", name() + "_gmg", params);
    _return_mesh = &getMeshByName(name() + "_gmg");
  }
}

std::unique_ptr<MeshBase>
TestMeshGenerator::generate()
{
  if (getParam<bool>("get_mesh_outside_construct"))
    static_cast<void>(getMeshByName("foo"));
  if (getParam<bool>("sub_outside_construct"))
  {
    auto params = _app.getFactory().getValidParams("GeneratedMeshGenerator");
    addMeshSubgenerator("GeneratedMeshGenerator", "foobar", params);
  }
  if (getParam<bool>("declare_mesh_prop_outside_construct"))
    static_cast<void>(declareMeshProperty<bool>("foo"));
  return std::move(*_return_mesh);
}
