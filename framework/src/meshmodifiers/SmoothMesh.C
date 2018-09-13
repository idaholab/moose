//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmoothMesh.h"

// MOOSE includes
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/mesh_smoother_laplace.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("MooseApp", SmoothMesh);

template <>
InputParameters
validParams<SmoothMesh>()
{
  InputParameters params = validParams<MeshModifier>();

  params.addClassDescription("Utilizes a simple Laplacian based smoother to attempt to improve "
                             "mesh quality.  Will not move boundary nodes or nodes along "
                             "block/subdomain boundaries");

  params.addParam<unsigned int>("iterations", 1, "The number of smoothing iterations to do.");

  return params;
}

SmoothMesh::SmoothMesh(const InputParameters & parameters)
  : MeshModifier(parameters), _iterations(getParam<unsigned int>("iterations"))
{
}

void
SmoothMesh::modify()
{
  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling SmoothMesh::modify()");

  LaplaceMeshSmoother lms(static_cast<UnstructuredMesh &>(_mesh_ptr->getMesh()));

  lms.smooth(_iterations);
}
