//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StitchedSubgenerators.h"

#include "CastUniquePointer.h"
#include "MooseUtils.h"
#include "FileMeshGenerator.h"

#include "libmesh/replicated_mesh.h"

registerMooseObject("MooseApp", StitchedSubgenerators);

defineLegacyParams(StitchedSubgenerators);

InputParameters
StitchedSubgenerators::validParams()
{
  InputParameters params = StitchedMeshGenerator::validParams();

  params.makeParamNotRequired<std::vector<MeshGeneratorName>>("inputs");

  params.addRequiredParam<std::vector<std::string>>("input_files", "The input mesh filenames");
  return params;
}

StitchedSubgenerators::StitchedSubgenerators(const InputParameters & parameters)
  : StitchedMeshGenerator(parameters),
    _input_filenames(getParam<std::vector<std::string>>("input_files"))
{
  const std::string sg_name_base = name() + "_sub_";

  // Create and add MeshGenerators for the input meshes
  _mesh_ptrs.reserve(_input_filenames.size());
  int sg_num = 0;
  for (auto & input_filename : _input_filenames)
    _mesh_ptrs.push_back(
      &this->addMeshSubgenerator("FileMeshGenerator",
                                 sg_name_base + std::to_string(sg_num++),
                                 "file", MeshFileName(input_filename)));
}
