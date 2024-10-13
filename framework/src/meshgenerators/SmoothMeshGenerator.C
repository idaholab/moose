//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmoothMeshGenerator.h"

// libMesh includes
#include "libmesh/mesh_smoother_laplace.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/replicated_mesh.h"

#include "CastUniquePointer.h"

registerMooseObject("MooseApp", SmoothMeshGenerator);

InputParameters
SmoothMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to smooth.");
  params.addClassDescription("Utilizes a simple Laplacian based smoother to attempt to improve "
                             "mesh quality.  Will not move boundary nodes or nodes along "
                             "block/subdomain boundaries");

  params.addParam<unsigned int>("iterations", 1, "The number of smoothing iterations to do.");

  return params;
}

SmoothMeshGenerator::SmoothMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _iterations(getParam<unsigned int>("iterations"))
{
}

std::unique_ptr<MeshBase>
SmoothMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> old_mesh = std::move(_input);
  if (!old_mesh->is_replicated())
    mooseError("SmoothMeshGenerator is not implemented for distributed meshes");

  auto mesh = dynamic_pointer_cast<ReplicatedMesh>(old_mesh);

  libMesh::LaplaceMeshSmoother lms(static_cast<UnstructuredMesh &>(*mesh));

  lms.smooth(_iterations);

  return dynamic_pointer_cast<MeshBase>(mesh);
}
