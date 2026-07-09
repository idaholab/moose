//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComponentMeshGenerationHelper.h"

InputParameters
ComponentMeshGenerationHelper::validParams()
{
  auto params = ActionComponent::validParams();
  params.addParam<bool>("show_mesh_generation_info",
                        false,
                        "Whether to show the current status of the mesh in the console after each "
                        "mesh generation step");
  params.addParam<bool>(
      "output_intermediate_meshes",
      false,
      "Whether to show the output the mesh to exodus after each mesh generation step");
  params.addParam<bool>("number_mesh_generator_output",
                        false,
                        "Whether to introduce a number in the mesh generator name to facilitate "
                        "sorting the output files");

  params.addParamNamesToGroup("show_mesh_generation_info output_intermediate_meshes",
                              "Verbosity and debug");
  return params;
}

ComponentMeshGenerationHelper::ComponentMeshGenerationHelper(const InputParameters & params)
  : ActionComponent(params),
    _show_mesh_generation_info(getParam<bool>("show_mesh_generation_info")),
    _output_intermediate_meshes(getParam<bool>("output_intermediate_meshes")),
    _number_mesh_generator_output(getParam<bool>("number_mesh_generator_output"))
{
}

void
ComponentMeshGenerationHelper::addMeshGenerator(const std::string & type,
                                                const std::string & suffix,
                                                InputParameters & params)
{
  params.set<bool>("show_info") = _show_mesh_generation_info;
  params.set<bool>("output") = _output_intermediate_meshes;
  const auto mg_name =
      name() + "_" + (_number_mesh_generator_output ? std::to_string(_mg_names.size()) + "_" : "") +
      suffix;
  _app.getMeshGeneratorSystem().addMeshGenerator(type, mg_name, params);
  _mg_names.push_back(mg_name);
}
