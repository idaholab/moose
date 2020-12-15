//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestMeshDataGenerator.h"
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", TestMeshDataDeclareGenerator);
registerMooseObject("MooseTestApp", TestMeshDataGetGenerator);

InputParameters
TestMeshDataDeclareGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<Real>("declare_mesh_data_value",
                                "Some data to store with declareMeshProperty.");
  return params;
}

TestMeshDataDeclareGenerator::TestMeshDataDeclareGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _mesh_data_value(declareMeshProperty<Real>("data"))
{
}

std::unique_ptr<MeshBase>
TestMeshDataDeclareGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  _mesh_data_value = getParam<Real>("declare_mesh_data_value");
  return mesh;
}

InputParameters
TestMeshDataGetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<Real>("get_mesh_data_value",
                                "Expected data value to get with getMeshProperty.");
  params.addRequiredParam<std::string>("prefix", "Name of object ('prefix') in declare object");
  return params;
}

TestMeshDataGetGenerator::TestMeshDataGetGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _mesh_data_value(getMeshProperty<Real>("data", getParam<std::string>("prefix")))
{
}

std::unique_ptr<MeshBase>
TestMeshDataGetGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  if (_mesh_data_value != getParam<Real>("get_mesh_data_value"))
    mooseError("Failed to get correct value from getMeshProperty");
  return mesh;
}
