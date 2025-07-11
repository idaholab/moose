//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Moose.h"
#include "LevelSetApp.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "LevelSetTypes.h"

InputParameters
LevelSetApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.addClassDescription(
      "Application containing object necessary to solve the level set equation.");

  params.set<bool>("automatic_automatic_scaling") = false;

  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;

  return params;
}

registerKnownLabel("LevelSetApp");

LevelSetApp::LevelSetApp(const InputParameters & parameters) : MooseApp(parameters)
{
  srand(processor_id());
  LevelSetApp::registerAll(_factory, _action_factory, _syntax);
}

void
LevelSetApp::registerApps()
{
  registerApp(LevelSetApp);
}

void
LevelSetApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  Registry::registerObjectsTo(f, {"LevelSetApp"});
  Registry::registerActionsTo(af, {"LevelSetApp"});
}

// Dynamic Library Entry Points - DO NOT MODIFY
extern "C" void
LevelSetApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  LevelSetApp::registerAll(f, af, s);
}
extern "C" void
LevelSetApp__registerApps()
{
  LevelSetApp::registerApps();
}
