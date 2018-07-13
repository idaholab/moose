//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmoothMesh.h"

// libMesh includes
#include "libmesh/mesh_smoother_laplace.h"

registerMooseObject("MooseApp", SmoothMesh);

template <>
InputParameters
validParams<SmoothMesh>()
{
  InputParameters params = validParams<MeshModifier>();
  return params;
}

SmoothMesh::SmoothMesh(const InputParameters & parameters) : MeshModifier(parameters) {}

void
SmoothMesh::modify()
{
  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling SmoothMesh::modify()");

  LaplaceMeshSmoother lms(static_cast<UnstructuredMesh &>(_mesh_ptr->getMesh()));

  lms.smooth(5);
}
