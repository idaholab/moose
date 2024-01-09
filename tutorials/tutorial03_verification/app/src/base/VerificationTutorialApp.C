//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VerificationTutorialApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

InputParameters
VerificationTutorialApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy material output, i.e., output properties on INITIAL as well as TIMESTEP_END
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

VerificationTutorialApp::VerificationTutorialApp(InputParameters parameters) : MooseApp(parameters)
{
  VerificationTutorialApp::registerAll(_factory, _action_factory, _syntax);
}

VerificationTutorialApp::~VerificationTutorialApp() {}

void
VerificationTutorialApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
  ModulesApp::registerAllObjects<VerificationTutorialApp>(f, af, syntax);
  Registry::registerObjectsTo(f, {"VerificationTutorialApp"});
  Registry::registerActionsTo(af, {"VerificationTutorialApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
VerificationTutorialApp::registerApps()
{
  registerApp(VerificationTutorialApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
VerificationTutorialApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  VerificationTutorialApp::registerAll(f, af, s);
}
extern "C" void
VerificationTutorialApp__registerApps()
{
  VerificationTutorialApp::registerApps();
}
