//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MiscApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
MiscApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

registerKnownLabel("MiscApp");

MiscApp::MiscApp(const InputParameters & parameters) : MooseApp(parameters)
{
  MiscApp::registerAll(_factory, _action_factory, _syntax);
}

MiscApp::~MiscApp() {}

void
MiscApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  Registry::registerObjectsTo(f, {"MiscApp"});
  Registry::registerActionsTo(af, {"MiscApp"});
}

void
MiscApp::registerApps()
{
  registerApp(MiscApp);
}

void
MiscApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"MiscApp"});
}

void
MiscApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"MiscApp"});
}

void
MiscApp::registerExecFlags(Factory & /*factory*/)
{
}

extern "C" void
MiscApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  MiscApp::registerAll(f, af, s);
}

extern "C" void
MiscApp__registerApps()
{
  MiscApp::registerApps();
}
