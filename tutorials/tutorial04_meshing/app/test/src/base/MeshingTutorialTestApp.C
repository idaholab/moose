//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "MeshingTutorialTestApp.h"
#include "MeshingTutorialApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

InputParameters
MeshingTutorialTestApp::validParams()
{
  InputParameters params = MeshingTutorialApp::validParams();
  return params;
}

MeshingTutorialTestApp::MeshingTutorialTestApp(InputParameters parameters) : MooseApp(parameters)
{
  MeshingTutorialTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

MeshingTutorialTestApp::~MeshingTutorialTestApp() {}

void
MeshingTutorialTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  MeshingTutorialApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"MeshingTutorialTestApp"});
    Registry::registerActionsTo(af, {"MeshingTutorialTestApp"});
  }
}

void
MeshingTutorialTestApp::registerApps()
{
  registerApp(MeshingTutorialApp);
  registerApp(MeshingTutorialTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
MeshingTutorialTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  MeshingTutorialTestApp::registerAll(f, af, s);
}
extern "C" void
MeshingTutorialTestApp__registerApps()
{
  MeshingTutorialTestApp::registerApps();
}
