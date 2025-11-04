//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;

  return params;
}

registerKnownLabel("ElectromagneticsApp");

ElectromagneticsApp::ElectromagneticsApp(const InputParameters & parameters) : MooseApp(parameters)
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
