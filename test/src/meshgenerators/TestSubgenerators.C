//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestSubgenerators.h"

#include "CastUniquePointer.h"
#include "MooseUtils.h"
#include "FileMeshGenerator.h"

#include "libmesh/replicated_mesh.h"

registerMooseObject("MooseTestApp", TestSubgenerators);

InputParameters
TestSubgenerators::validParams()
{
  InputParameters params = StitchedMeshGenerator::validParams();

  params.makeParamNotRequired<std::vector<MeshGeneratorName>>("inputs");
  params.set<std::vector<MeshGeneratorName>>("inputs") = {};

  params.addRequiredParam<std::vector<std::string>>("input_files", "The input mesh filenames");

  return params;
}

TestSubgenerators::TestSubgenerators(const InputParameters & parameters)
  : StitchedMeshGenerator(parameters),
    _input_filenames(getParam<std::vector<std::string>>("input_files"))
{
  const std::string sg_name_base = name() + "_sub_";

  // Create and add MeshGenerators for the input meshes
  _mesh_ptrs.reserve(_input_filenames.size());
  int sg_num = 0;
  for (auto & input_filename : _input_filenames)
    // Test the variadic API for half of our subgenerators
    if (sg_num % 2)
    {
      const auto name = sg_name_base + std::to_string(sg_num++);
      addMeshSubgenerator("FileMeshGenerator", name, "file", MeshFileName(input_filename));
      _mesh_ptrs.push_back(&getMeshByName(name));
    }
    // Test the InputParameters API for the other half
    else
    {
      // Deliberately avoid getting params from Factory, to test
      // that this overload is adding any missing ones itself
      InputParameters subgenerator_params = FileMeshGenerator::validParams();

      subgenerator_params.set<MeshFileName>("file") = input_filename;
      const auto name = sg_name_base + std::to_string(sg_num++);
      addMeshSubgenerator("FileMeshGenerator", name, subgenerator_params);
      _mesh_ptrs.push_back(&getMeshByName(name));
    }
}
