//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChemicalReactionsTestApp.h"
#include "ChemicalReactionsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ChemicalReactionsTestApp::validParams()
{
  InputParameters params = ChemicalReactionsApp::validParams();
  return params;
}

registerKnownLabel("ChemicalReactionsTestApp");

ChemicalReactionsTestApp::ChemicalReactionsTestApp(InputParameters parameters)
  : MooseApp(parameters)
{
  ChemicalReactionsTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

ChemicalReactionsTestApp::~ChemicalReactionsTestApp() {}

void
ChemicalReactionsTestApp::registerAll(Factory & f,
                                      ActionFactory & af,
                                      Syntax & s,
                                      bool use_test_objects)
{
  ChemicalReactionsApp::registerAll(f, af, s);
  if (use_test_objects)
  {
    Registry::registerObjectsTo(f, {"ChemicalReactionsTestApp"});
    Registry::registerActionsTo(af, {"ChemicalReactionsTestApp"});
  }
}

void
ChemicalReactionsTestApp::registerApps()
{
  registerApp(ChemicalReactionsApp);
  registerApp(ChemicalReactionsTestApp);
}

void
ChemicalReactionsTestApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"ChemicalReactionsTestApp"});
}
void
ChemicalReactionsTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"ChemicalReactionsTestApp"});
}
void
ChemicalReactionsTestApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

extern "C" void
ChemicalReactionsTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ChemicalReactionsTestApp::registerAll(f, af, s);
}

// External entry point for dynamic application loading
extern "C" void
ChemicalReactionsTestApp__registerApps()
{
  ChemicalReactionsTestApp::registerApps();
}
