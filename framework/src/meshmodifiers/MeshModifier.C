//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshModifier.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<MeshModifier>()
{
  InputParameters params = validParams<MooseObject>();
  params.addParam<std::vector<std::string>>(
      "depends_on",
      "The MeshModifiers that this modifier relies upon (i.e. must execute before this one)");
  params.addParam<bool>("force_prepare",
                        false,
                        "Normally all MeshModifiers run before the mesh is prepared for use. This "
                        "flag can be set on an individual modifier "
                        "to force preperation between modifiers where they might be needed.");

  params.registerBase("MeshModifier");

  return params;
}

MeshModifier::MeshModifier(const InputParameters & parameters)
  : MooseObject(parameters),
    Restartable(parameters, "MeshModifiers"),
    _mesh_ptr(NULL),
    _depends_on(getParam<std::vector<std::string>>("depends_on")),
    _force_prepare(getParam<bool>("force_prepare"))
{
}

void
MeshModifier::modifyMesh(MooseMesh * mesh, MooseMesh * displaced_mesh)
{
  // Initialize or reinitialize any mesh related structures.
  initialize();

  modifyMeshHelper(mesh);

  // Now do the same thing for the displaced mesh if it exists
  if (displaced_mesh)
    modifyMeshHelper(displaced_mesh);
}

void
MeshModifier::modifyMeshHelper(MooseMesh * mesh)
{
  // Set pointer to the mesh so that derived classes may use them
  _mesh_ptr = mesh;

  // Modify the mesh!
  modify();

  // Prepare the mesh if requested
  if (_force_prepare)
    mesh->prepare();
}
