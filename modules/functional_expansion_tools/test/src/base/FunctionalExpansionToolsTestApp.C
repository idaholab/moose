//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "FunctionalExpansionToolsTestApp.h"
#include "FunctionalExpansionToolsApp.h"

template <>
InputParameters
validParams<FunctionalExpansionToolsTestApp>()
{
  InputParameters params = validParams<FunctionalExpansionToolsApp>();
  return params;
}

registerKnownLabel("FunctionalExpansionToolsTestApp");

FunctionalExpansionToolsTestApp::FunctionalExpansionToolsTestApp(InputParameters parameters)
  : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  FunctionalExpansionToolsApp::registerObjectDepends(_factory);
  FunctionalExpansionToolsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  FunctionalExpansionToolsApp::associateSyntaxDepends(_syntax, _action_factory);
  FunctionalExpansionToolsApp::associateSyntax(_syntax, _action_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    FunctionalExpansionToolsTestApp::registerObjects(_factory);
    FunctionalExpansionToolsTestApp::associateSyntax(_syntax, _action_factory);
  }
}

FunctionalExpansionToolsTestApp::~FunctionalExpansionToolsTestApp() {}

void
FunctionalExpansionToolsTestApp::registerApps()
{
  registerApp(FunctionalExpansionToolsApp);
  registerApp(FunctionalExpansionToolsTestApp);
}

void
FunctionalExpansionToolsTestApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"FunctionalExpansionToolsTestApp"});
}

void
FunctionalExpansionToolsTestApp::associateSyntax(Syntax & /*syntax*/,
                                                 ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"FunctionalExpansionToolsTestApp"});
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
FunctionalExpansionToolsTestApp__registerApps()
{
  FunctionalExpansionToolsTestApp::registerApps();
}

// External entry point for dynamic object registration
extern "C" void
FunctionalExpansionToolsTestApp__registerObjects(Factory & factory)
{
  FunctionalExpansionToolsTestApp::registerObjects(factory);
}

// External entry point for dynamic syntax association
extern "C" void
FunctionalExpansionToolsTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  FunctionalExpansionToolsTestApp::associateSyntax(syntax, action_factory);
}
