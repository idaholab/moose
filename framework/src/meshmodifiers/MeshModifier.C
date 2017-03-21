/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
