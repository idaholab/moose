//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReactorApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ReactorApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  return params;
}

ReactorApp::ReactorApp(const InputParameters & parameters) : MooseApp(parameters)
{
  ReactorApp::registerAll(_factory, _action_factory, _syntax);
}

ReactorApp::~ReactorApp() {}

void
ReactorApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  /* ModulesApp::registerAll(f, af, s); */
  Registry::registerObjectsTo(f, {"ReactorApp"});
  Registry::registerActionsTo(af, {"ReactorApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
ReactorApp::registerApps()
{
  registerApp(ReactorApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
ReactorApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ReactorApp::registerAll(f, af, s);
}
extern "C" void
ReactorApp__registerApps()
{
  ReactorApp::registerApps();
}
