//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<ContactApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("ContactApp");

ContactApp::ContactApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ContactApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ContactApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  ContactApp::registerExecFlags(_factory);
}

ContactApp::~ContactApp() {}

// External entry point for dynamic application loading
extern "C" void
ContactApp__registerApps()
{
  ContactApp::registerApps();
}
void
ContactApp::registerApps()
{
  registerApp(ContactApp);
}

// External entry point for dynamic object registration
extern "C" void
ContactApp__registerObjects(Factory & factory)
{
  ContactApp::registerObjects(factory);
}
void
ContactApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"ContactApp"});
}

// External entry point for dynamic syntax association
extern "C" void
ContactApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  ContactApp::associateSyntax(syntax, action_factory);
}
void
ContactApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"ContactApp"});

  registerSyntax("ContactAction", "Contact/*");

  registerSyntax("ContactPenetrationAuxAction", "Contact/*");
  registerSyntax("ContactPenetrationVarAction", "Contact/*");

  registerSyntax("ContactPressureAuxAction", "Contact/*");
  registerSyntax("ContactPressureVarAction", "Contact/*");

  registerSyntax("NodalAreaAction", "Contact/*");
  registerSyntax("NodalAreaVarAction", "Contact/*");

  registerTask("output_penetration_info_vars", false);
  syntax.addDependency("output_penetration_info_vars", "add_output");
}

// External entry point for dynamic execute flag registration
extern "C" void
ContactApp__registerExecFlags(Factory & factory)
{
  ContactApp::registerExecFlags(factory);
}
void
ContactApp::registerExecFlags(Factory & /*factory*/)
{
}
