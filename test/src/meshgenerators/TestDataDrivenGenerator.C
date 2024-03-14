//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestDataDrivenGenerator.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/mesh_modification.h"

#include "GeneratedMeshGenerator.h"
#include "TransformGenerator.h"

registerMooseObject("MooseTestApp", TestDataDrivenGenerator);

InputParameters
TestDataDrivenGenerator::validParams()
{
  auto params = MeshGenerator::validParams();

  params.addParam<unsigned int>("nx", "The metadata value of nx to set, if any");
  params.addParam<unsigned int>("ny", "The metadata value of ny to set, if any");

  params.addParam<MeshGeneratorName>("nx_generator",
                                     "The generator to get nx from (if generating a mesh)");
  params.addParam<MeshGeneratorName>("ny_generator",
                                     "The generator to get ny from (if generating a mesh)");

  params.addParam<MeshGeneratorName>("subgenerator_no_data_only_from_generator", "abcd");
  params.addParam<std::string>("subgenerator_no_data_only_scale_metadata", "abcd");

  MeshGenerator::setHasGenerateData(params);

  return params;
}

TestDataDrivenGenerator::TestDataDrivenGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _subgenerator_no_data_only_mesh(isParamValid("subgenerator_no_data_only_from_generator")
                                        ? &getMesh("subgenerator_no_data_only_from_generator")
                                        : nullptr)
{
  if (isParamValid("nx"))
    declareMeshProperty<unsigned int>("nx");
  if (isParamValid("ny"))
    declareMeshProperty<unsigned int>("ny");
  if (isParamValid("nx_generator"))
    (void)getMesh("nx_generator");
  if (isParamValid("ny_generator"))
    (void)getMesh("ny_generator");

  if (_subgenerator_no_data_only_mesh)
  {
    {
      auto params = GeneratedMeshGenerator::validParams();
      params.set<MooseEnum>("dim") = 1;
      addMeshSubgenerator("GeneratedMeshGenerator", "generated", params);
    }

    {
      auto params = TransformGenerator::validParams();
      params.set<MeshGeneratorName>("input") = "generated";
      params.set<MooseEnum>("transform") = "translate";
      params.set<RealVectorValue>("vector_value") = {10, 0, 0};
      addMeshSubgenerator("TransformGenerator", "transform", params);
    }

    _subgenerator_no_data_only_submesh = &getMeshByName("transform");
  }
}

void
TestDataDrivenGenerator::generateData()
{
  if (isParamValid("nx"))
    setMeshProperty<unsigned int>("nx") = getParam<unsigned int>("nx");
  if (isParamValid("ny"))
    setMeshProperty<unsigned int>("ny") = getParam<unsigned int>("ny");
}

std::unique_ptr<MeshBase>
TestDataDrivenGenerator::generate()
{
  if (isParamValid("nx_generator") && isParamValid("ny_generator"))
  {
    const auto nx =
        getMeshProperty<unsigned int>("nx", getParam<MeshGeneratorName>("nx_generator"));
    const auto ny =
        getMeshProperty<unsigned int>("ny", getParam<MeshGeneratorName>("ny_generator"));

    auto mesh = buildMeshBaseObject();
    MeshTools::Generation::build_square(static_cast<UnstructuredMesh &>(*mesh), nx, ny);
    return mesh;
  }
  else if (_subgenerator_no_data_only_mesh && _subgenerator_no_data_only_submesh)
  {
    mooseAssert(!*_subgenerator_no_data_only_mesh, "Should be data only");
    _subgenerator_no_data_only_mesh->reset();
    const auto scale = getMeshProperty<Real>(
        getParam<std::string>("subgenerator_no_data_only_scale_metadata"),
        getParam<MeshGeneratorName>("subgenerator_no_data_only_from_generator"));
    MeshTools::Modification::scale(**_subgenerator_no_data_only_submesh, scale, 0, 0);
    return std::move(*_subgenerator_no_data_only_submesh);
  }

  mooseError("Should not call");
}
