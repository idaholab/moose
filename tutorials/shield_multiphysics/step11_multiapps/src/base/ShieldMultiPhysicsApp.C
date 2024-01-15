//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Tutorial Includes
#include "ShieldMultiPhysicsApp.h"

// MOOSE Includes
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

InputParameters
ShieldMultiPhysicsApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

ShieldMultiPhysicsApp::ShieldMultiPhysicsApp(InputParameters parameters) : MooseApp(parameters)
{
  ShieldMultiPhysicsApp::registerAll(_factory, _action_factory, _syntax);
}

void
ShieldMultiPhysicsApp::registerApps()
{
  registerApp(ShieldMultiPhysicsApp);
}

void
ShieldMultiPhysicsApp::registerAll(Factory & factory,
                                   ActionFactory & action_factory,
                                   Syntax & syntax)
{
  Registry::registerObjectsTo(factory, {"ShieldMultiPhysicsApp"});
  Registry::registerActionsTo(action_factory, {"ShieldMultiPhysicsApp"});
  ModulesApp::registerAllObjects<ShieldMultiPhysicsApp>(factory, action_factory, syntax);
}
