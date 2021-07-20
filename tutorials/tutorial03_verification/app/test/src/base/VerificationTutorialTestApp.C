//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "VerificationTutorialTestApp.h"
#include "VerificationTutorialApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

InputParameters
VerificationTutorialTestApp::validParams()
{
  InputParameters params = VerificationTutorialApp::validParams();
  return params;
}

VerificationTutorialTestApp::VerificationTutorialTestApp(InputParameters parameters)
  : MooseApp(parameters)
{
  VerificationTutorialTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

VerificationTutorialTestApp::~VerificationTutorialTestApp() {}

void
VerificationTutorialTestApp::registerAll(Factory & f,
                                         ActionFactory & af,
                                         Syntax & s,
                                         bool use_test_objs)
{
  VerificationTutorialApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"VerificationTutorialTestApp"});
    Registry::registerActionsTo(af, {"VerificationTutorialTestApp"});
  }
}

void
VerificationTutorialTestApp::registerApps()
{
  registerApp(VerificationTutorialApp);
  registerApp(VerificationTutorialTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
VerificationTutorialTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  VerificationTutorialTestApp::registerAll(f, af, s);
}
extern "C" void
VerificationTutorialTestApp__registerApps()
{
  VerificationTutorialTestApp::registerApps();
}
