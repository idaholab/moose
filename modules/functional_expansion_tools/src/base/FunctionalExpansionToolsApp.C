//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

template <>
InputParameters
validParams<FunctionalExpansionToolsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("FunctionalExpansionToolsApp");

FunctionalExpansionToolsApp::FunctionalExpansionToolsApp(InputParameters parameters)
  : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  FunctionalExpansionToolsApp::registerObjectDepends(_factory);
  FunctionalExpansionToolsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  FunctionalExpansionToolsApp::associateSyntaxDepends(_syntax, _action_factory);
  FunctionalExpansionToolsApp::associateSyntax(_syntax, _action_factory);
}

FunctionalExpansionToolsApp::~FunctionalExpansionToolsApp() {}

void
FunctionalExpansionToolsApp::registerApps()
{
  registerApp(FunctionalExpansionToolsApp);
}

void
FunctionalExpansionToolsApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"FunctionalExpansionToolsApp"});
}

void
FunctionalExpansionToolsApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"FunctionalExpansionToolsApp"});
  /* Uncomment Syntax parameters and register your new objects here! */
}

void
FunctionalExpansionToolsApp::registerObjectDepends(Factory & /*factory*/)
{
}

void
FunctionalExpansionToolsApp::associateSyntaxDepends(Syntax & /*syntax*/,
                                                    ActionFactory & /*action_factory*/)
{
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
FunctionalExpansionToolsApp__registerApps()
{
  FunctionalExpansionToolsApp::registerApps();
}

extern "C" void
FunctionalExpansionToolsApp__registerObjects(Factory & factory)
{
  FunctionalExpansionToolsApp::registerObjects(factory);
}

extern "C" void
FunctionalExpansionToolsApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  FunctionalExpansionToolsApp::associateSyntax(syntax, action_factory);
}
