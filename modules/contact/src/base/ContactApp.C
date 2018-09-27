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
  ContactApp::registerAll(_factory, _action_factory, _syntax);
}

ContactApp::~ContactApp() {}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
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

void
ContactApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"ContactApp"});
  Registry::registerActionsTo(af, {"ContactApp"});
  associateSyntaxInner(s, af);
}

void
ContactApp::registerApps()
{
  registerApp(ContactApp);
}

void
ContactApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"ContactApp"});
}

void
ContactApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"ContactApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
ContactApp::registerExecFlags(Factory & /*factory*/)
{
}

extern "C" void
ContactApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ContactApp::registerAll(f, af, s);
}
extern "C" void
ContactApp__registerApps()
{
  ContactApp::registerApps();
}
