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
#include "MooseUtils.h"
#include "MooseMesh.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/mesh_tools.h"

InputParameters
MeshCutUserObjectBase::validParams()
{
  InputParameters params = GeometricCutUserObject::validParams();
  params.addDeprecatedParam<MeshFileName>(
      "mesh_file",
      "Mesh file for the XFEM geometric cut.",
      "This parameter is deprecated in favor of reading in the cuttermesh from the mesh system "
      "using 'mesh_generator_name'.");
  params.addParam<std::string>("mesh_generator_name",
                               "Mesh generator for the XFEM geometric cutter.");
  params.addClassDescription("Base class for mesh-based cutters used with XFEM.");
  return params;
}

MeshCutUserObjectBase::MeshCutUserObjectBase(const InputParameters & parameters)
  : GeometricCutUserObject(parameters, true)
{
  if (isParamValid("mesh_generator_name"))
  {
    std::string cutterMeshName = getParam<std::string>("mesh_generator_name");
    auto & mesh_generator_system = _app.getMeshGeneratorSystem();
    _cutter_mesh = mesh_generator_system.getSavedMesh(cutterMeshName);
  }
  else if (isParamValid("mesh_file"))
  {
    MeshFileName cutterMeshName = getParam<MeshFileName>("mesh_file");
    _cutter_mesh = std::make_unique<ReplicatedMesh>(_communicator);
    _cutter_mesh->read(cutterMeshName);
  }
  else
  {
    mooseError("Must specify 'mesh_generator_name' or 'mesh_file'. ");
  }

  if (!_cutter_mesh)
    mooseError("Not able to read in a cutter mesh.");
}

MeshBase &
MeshCutUserObjectBase::getCutterMesh() const
{
  mooseAssert(_cutter_mesh, "MeshCutUserObjectBase::getCutterMesh _cutter_mesh is nullptr");
  return *_cutter_mesh;
}
