//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidPropertiesApp.h"
#include "HeatTransferApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
SolidPropertiesApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  return params;
}

registerKnownLabel("SolidPropertiesApp");

SolidPropertiesApp::SolidPropertiesApp(const InputParameters & parameters) : MooseApp(parameters)
{
  SolidPropertiesApp::registerAll(_factory, _action_factory, _syntax);
}

void
SolidPropertiesApp::registerApps()
{
  registerApp(SolidPropertiesApp);

  HeatTransferApp::registerApps();
}

void
SolidPropertiesApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  HeatTransferApp::registerAll(f, af, syntax);
  Registry::registerObjectsTo(f, {"SolidPropertiesApp"});
  Registry::registerActionsTo(af, {"SolidPropertiesApp"});

  registerSyntaxTask(
      "AddSolidPropertiesDeprecatedAction", "Modules/SolidProperties/*", "add_solid_properties");
  registerSyntaxTask("AddSolidPropertiesAction", "SolidProperties/*", "add_solid_properties");
  registerMooseObjectTask("add_solid_properties", SolidProperties, false);

  syntax.addDependency("add_solid_properties", "add_function");
  syntax.addDependency("add_user_object", "add_solid_properties");
}

extern "C" void
SolidPropertiesApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  SolidPropertiesApp::registerAll(f, af, s);
}
extern "C" void
SolidPropertiesApp__registerApps()
{
  SolidPropertiesApp::registerApps();
}
