//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddMeshModifierAction.h"
#include "MooseMesh.h"
#include "MeshModifier.h"
#include "Factory.h"
#include "MooseApp.h"

template <>
InputParameters
validParams<AddMeshModifierAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

AddMeshModifierAction::AddMeshModifierAction(InputParameters params) : MooseObjectAction(params) {}

void
AddMeshModifierAction::act()
{
  // Don't do mesh modifiers when recovering!
  if (_app.isRecovering())
    return;

  // Add a pointer to the mesh, this is required for this MeshModifier to inherit from the
  // BlockRestrictable,
  // as is the case for SideSetAroundSubdomain
  _moose_object_pars.set<MooseMesh *>("_mesh") = _mesh.get();

  _app.addMeshModifier(_type, _name, _moose_object_pars);
}
