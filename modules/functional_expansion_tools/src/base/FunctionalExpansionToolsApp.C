//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionalExpansionToolsApp.h"

#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
FunctionalExpansionToolsApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;

  return params;
}

registerKnownLabel("FunctionalExpansionToolsApp");

FunctionalExpansionToolsApp::FunctionalExpansionToolsApp(const InputParameters & parameters)
  : MooseApp(parameters)
{
  FunctionalExpansionToolsApp::registerAll(_factory, _action_factory, _syntax);
}

FunctionalExpansionToolsApp::~FunctionalExpansionToolsApp() {}

void
FunctionalExpansionToolsApp::registerApps()
{
  registerApp(FunctionalExpansionToolsApp);
}

void
FunctionalExpansionToolsApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  Registry::registerObjectsTo(f, {"FunctionalExpansionToolsApp"});
  Registry::registerActionsTo(af, {"FunctionalExpansionToolsApp"});
}

extern "C" void
FunctionalExpansionToolsApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FunctionalExpansionToolsApp::registerAll(f, af, s);
}
extern "C" void
FunctionalExpansionToolsApp__registerApps()
{
  FunctionalExpansionToolsApp::registerApps();
}
