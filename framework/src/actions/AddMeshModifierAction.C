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
