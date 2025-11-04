//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactApp.h"
#include "SolidMechanicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ContactApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;

  return params;
}

registerKnownLabel("ContactApp");

ContactApp::ContactApp(const InputParameters & parameters) : MooseApp(parameters)
{
  ContactApp::registerAll(_factory, _action_factory, _syntax);
}

ContactApp::~ContactApp() {}

void
ContactApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  Registry::registerObjectsTo(f, {"ContactApp"});
  Registry::registerActionsTo(af, {"ContactApp"});

  registerSyntax("ContactAction", "Contact/*");
  registerSyntax("ExplicitDynamicsContactAction", "ExplicitDynamicsContact/*");

  registerTask("output_penetration_info_vars", false);
  registerTask("add_contact_aux_variable", false);
  syntax.addDependency("output_penetration_info_vars", "add_output");
  syntax.addDependency("add_postprocessor", "output_penetration_info_vars");
  syntax.addDependency("add_contact_aux_variable", "add_variable");
  syntax.addDependency("setup_variable_complete", "add_contact_aux_variable");

  SolidMechanicsApp::registerAll(f, af, syntax);
}

void
ContactApp::registerApps()
{
  registerApp(ContactApp);

  SolidMechanicsApp::registerApps();
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
