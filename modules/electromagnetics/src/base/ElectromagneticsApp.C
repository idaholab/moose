//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElectromagneticsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ElectromagneticsApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

registerKnownLabel("ElectromagneticsApp");

ElectromagneticsApp::ElectromagneticsApp(InputParameters parameters) : MooseApp(parameters)
{
  ElectromagneticsApp::registerAll(_factory, _action_factory, _syntax);
}

ElectromagneticsApp::~ElectromagneticsApp() {}

void
ElectromagneticsApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  Registry::registerObjectsTo(f, {"ElectromagneticsApp"});
  Registry::registerActionsTo(af, {"ElectromagneticsApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
ElectromagneticsApp::registerApps()
{
  registerApp(ElectromagneticsApp);
}

void
ElectromagneticsApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"ElectromagneticsApp"});
}

void
ElectromagneticsApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"ElectromagneticsApp"});
}

void
ElectromagneticsApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
ElectromagneticsApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ElectromagneticsApp::registerAll(f, af, s);
}
extern "C" void
ElectromagneticsApp__registerApps()
{
  ElectromagneticsApp::registerApps();
}
