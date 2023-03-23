//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshGeneratorMesh.h"

#include "libmesh/face_quad4.h"
#include "libmesh/face_tri3.h"

registerMooseObject("MooseApp", MeshGeneratorMesh);

InputParameters
MeshGeneratorMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.set<bool>("_mesh_generator_mesh") = true;

  params.addParam<std::string>("final_generator",
                               "The name of the mesh generator output to use for the final Mesh");
  params.addClassDescription("Mesh generated using mesh generators");
  return params;
}

MeshGeneratorMesh::MeshGeneratorMesh(const InputParameters & parameters) : MooseMesh(parameters) {}

std::unique_ptr<MooseMesh>
MeshGeneratorMesh::safeClone() const
{
  return std::make_unique<MeshGeneratorMesh>(*this);
}

void
MeshGeneratorMesh::buildMesh()
{
  if (!hasMeshBase())
    mooseError("The mesh base has not been set");
}
