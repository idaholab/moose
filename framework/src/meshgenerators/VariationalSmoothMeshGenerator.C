//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariationalSmoothMeshGenerator.h"

// libMesh includes
#include "libmesh/mesh_smoother_vsmoother.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/replicated_mesh.h"

#include "CastUniquePointer.h"

registerMooseObject("MooseApp", VariationalSmoothMeshGenerator);

InputParameters
VariationalSmoothMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to smooth.");
  params.addClassDescription("Utilizes variational mesh smoother to improve mesh quality.");

  params.addParam<unsigned int>("iterations", 1, "The number of smoothing iterations to do.");

  return params;
}

VariationalSmoothMeshGenerator::VariationalSmoothMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _iterations(getParam<unsigned int>("iterations"))
{
}

std::unique_ptr<MeshBase>
VariationalSmoothMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> old_mesh = std::move(_input);
  if (!old_mesh->is_replicated())
    mooseError("VariationalSmoothMeshGenerator is not implemented for distributed meshes");

  auto mesh = dynamic_pointer_cast<ReplicatedMesh>(old_mesh);

  VariationalMeshSmoother vms(static_cast<UnstructuredMesh &>(*mesh));

  vms.smooth(_iterations);

  return dynamic_pointer_cast<MeshBase>(mesh);
}
