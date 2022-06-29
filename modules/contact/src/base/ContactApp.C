//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactApp.h"
#include "TensorMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ContactApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;

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

  registerTask("output_penetration_info_vars", false);
  registerTask("add_contact_aux_variable", false);
  syntax.addDependency("output_penetration_info_vars", "add_output");
  syntax.addDependency("add_postprocessor", "output_penetration_info_vars");
  syntax.addDependency("add_contact_aux_variable", "add_variable");
  syntax.addDependency("setup_variable_complete", "add_contact_aux_variable");
}

void
ContactApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"ContactApp"});
  Registry::registerActionsTo(af, {"ContactApp"});
  associateSyntaxInner(s, af);

  TensorMechanicsApp::registerAll(f, af, s);
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
ContactApp::registerObjectDepends(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjectsDepends");
  TensorMechanicsApp::registerObjects(factory);
}

void
ContactApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of registerObjectsDepends");
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
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
