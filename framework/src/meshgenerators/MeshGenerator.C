//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshGenerator.h"
#include "MooseMesh.h"
#include "MooseApp.h"

template <>
InputParameters
validParams<MeshGenerator>()
{
  InputParameters params = validParams<MooseObject>();

  params.registerBase("MeshGenerator");
  return params;
}

MeshGenerator::MeshGenerator(const InputParameters & parameters)
  : MooseObject(parameters),
    Restartable(this, "MeshGenerators"),
    _mesh(_app.actionWarehouse().mesh())
{
}

std::unique_ptr<MeshBase> &
MeshGenerator::getMesh(const std::string & input_mesh_generator_parameter_name)
{
  if (isParamValid(input_mesh_generator_parameter_name))
  {
    auto name = getParam<MeshGeneratorName>(input_mesh_generator_parameter_name);

    _depends_on.push_back(name);

    return _app.getMeshGeneratorOutput(name);
  }
  else
    return _null_mesh;
}

std::unique_ptr<MeshBase> &
MeshGenerator::getMeshByName(const MeshGeneratorName & input_mesh_generator)
{
  _depends_on.push_back(input_mesh_generator);
  return _app.getMeshGeneratorOutput(input_mesh_generator);
}
