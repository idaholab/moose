//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMeshGenerator.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/face_quad4.h"

registerMooseObject("MooseApp", FileMeshGenerator);

template <>
InputParameters
validParams<FileMeshGenerator>()
{
  InputParameters params = validParams<MeshGenerator>();

  params.addRequiredParam<MeshFileName>("file", "The filename to read.");

  return params;
}

FileMeshGenerator::FileMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _file_name(getParam<MeshFileName>("file"))
{
}

std::unique_ptr<MeshBase>
FileMeshGenerator::generate()
{
  auto mesh = libmesh_make_unique<ReplicatedMesh>(comm());

  mesh->read(_file_name);

  return mesh;
}
