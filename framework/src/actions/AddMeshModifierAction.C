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

#include "AddMeshModifierAction.h"
#include "MooseMesh.h"
#include "MeshModifier.h"
#include "Factory.h"
#include "MooseApp.h"

template<>
InputParameters validParams<AddMeshModifierAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

AddMeshModifierAction::AddMeshModifierAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddMeshModifierAction::act()
{
  // Don't do mesh modifiers when recovering!
  if (_app.isRecovering())
    return;

  // Add a pointer to the mesh, this is required for this MeshModifier to inheret from the BlockRestrictable,
  // as is the case for SideSetAroundSubdomain
  _moose_object_pars.set<MooseMesh *>("_mesh") = _mesh;

  // Create the modifier object and run it
  MeshModifier *mesh_modifier = static_cast<MeshModifier *>(_factory.create(_type, getShortName(), _moose_object_pars));

  mooseAssert(_mesh != NULL, "Mesh hasn't been created");
  mooseAssert(mesh_modifier != NULL, "Mesh Modifier hasn't been created");

  // Run the modifier on the normal mesh
  mesh_modifier->setMeshPointer(_mesh);
  mesh_modifier->modify();

  // We'll need to prepare again!
  _mesh->prepared(false);

  // Run the modifier on the displaced mesh
  if (_displaced_mesh)
  {
    mesh_modifier->setMeshPointer(_displaced_mesh);
    mesh_modifier->modify();
    _displaced_mesh->prepared(false);
  }

  delete mesh_modifier;
}
