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

registerMooseObject("MooseApp", SmoothMeshGenerator);

template <>
InputParameters
validParams<SmoothMeshGenerator>()
{
  InputParameters params = validParams<MeshGenerator>();

  params.addParam<MeshGeneratorName>("input", "Optional input mesh to add the elements to");
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
  //std::unique_ptr<MeshBase> mesh = dynamic_cast<std::unique_ptr<ReplicatedMesh>&>(*std::move(_input))->clone();
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // If there was no input mesh then let's just make a new one
  // Is it pertinent ? No input would be useless...
  if (!mesh)
    mesh = libmesh_make_unique<ReplicatedMesh>(comm(), 2);

  LaplaceMeshSmoother lms(static_cast<UnstructuredMesh &>(*mesh));

  lms.smooth(_iterations);

  return mesh;
}
