//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCutUserObjectBase.h"

#include "MooseError.h"
#include "MooseMesh.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/mesh_tools.h"

InputParameters
MeshCutUserObjectBase::validParams()
{
  InputParameters params = GeometricCutUserObject::validParams();
  params.addRequiredParam<MeshFileName>(
      "mesh_file",
      "Mesh file for the XFEM geometric cut; currently only the Exodus type is supported");
  params.addClassDescription("Creates a UserObject base class for a mesh cutter in 2D problems");
  return params;
}

MeshCutUserObjectBase::MeshCutUserObjectBase(const InputParameters & parameters)
  : GeometricCutUserObject(parameters, true)
{
  // only the Exodus type is currently supported
  MeshFileName cutterMeshFileName = getParam<MeshFileName>("mesh_file");
  _cutter_mesh = std::make_unique<ReplicatedMesh>(_communicator);
  _cutter_mesh->read(cutterMeshFileName);
}

MeshBase &
MeshCutUserObjectBase::getCutterMesh() const
{
  mooseAssert(_cutter_mesh, "MeshCutUserObjectBase::getCutterMesh _cutter_mesh is nullptr");
  return *_cutter_mesh;
}
