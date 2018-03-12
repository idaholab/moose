//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseTestApp.h"
#include "MooseTestAppTypes.h"
#include "Moose.h"
#include "Factory.h"
#include "MooseSyntax.h"

#include "ActionFactory.h"
#include "AppFactory.h"

#include "MooseTestApp.h"

template <>
InputParameters
validParams<MooseTestApp>()
{
  InputParameters params = validParams<MooseApp>();
  /* MooseTestApp is special because it will have its own
   * binary and we want the default to allow test objects.
   */
  params.suppressParameter<bool>("allow_test_objects");
  params.addCommandLineParam<bool>("disallow_test_objects",
                                   "--disallow-test-objects",
                                   false,
                                   "Don't register test objects and syntax");
  return params;
}

MooseTestApp::MooseTestApp(const InputParameters & parameters) : MooseApp(parameters)
{
  bool use_test_objs = !getParam<bool>("disallow_test_objects");

  Moose::registerObjects(_factory);
  Moose::associateSyntax(_syntax, _action_factory);
  Moose::registerExecFlags(_factory);
  if (use_test_objs)
  {
    MooseTestApp::registerObjects(_factory);
    MooseTestApp::associateSyntax(_syntax, _action_factory);
    MooseTestApp::registerExecFlags(_factory);
  }
}

MooseTestApp::~MooseTestApp() {}

// External entry point for dynamic application loading
extern "C" void
MooseTestApp__registerApps()
{
  MooseTestApp::registerApps();
}
void
MooseTestApp::registerApps()
{
  registerApp(MooseTestApp);
}

// External entry point for dynamic object registration
extern "C" void
MooseTestApp__registerObjects(Factory & factory)
{
  MooseTestApp::registerObjects(factory);
}
void
MooseTestApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"MooseTestApp"});
}

// External entry point for dynamic syntax association
extern "C" void
MooseTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  MooseTestApp::associateSyntax(syntax, action_factory);
}

void
MooseTestApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"MooseTestApp"});

  // and add more
  registerSyntax("ConvDiffMetaAction", "ConvectionDiffusion");
  registerSyntaxTask("AddAuxVariableAction", "MoreAuxVariables/*", "add_aux_variable");
  registerSyntaxTask("AddLotsOfAuxVariablesAction", "LotsOfAuxVariables/*", "add_variable");

  registerSyntax("ApplyCoupledVariablesTestAction", "ApplyInputParametersTest");
  registerSyntax("AddLotsOfDiffusion", "Testing/LotsOfDiffusion/*");
  registerSyntax("TestGetActionsAction", "TestGetActions");
  registerSyntax("BadAddKernelAction", "BadKernels/*");

  registerSyntax("AddMatAndKernel", "AddMatAndKernel");

  registerSyntax("MetaNodalNormalsAction", "MetaNodalNormals");
}

// External entry point for dynamic execute flag registration
extern "C" void
MooseTestApp__registerExecFlags(Factory & factory)
{
  MooseTestApp::registerExecFlags(factory);
}
void
MooseTestApp::registerExecFlags(Factory & factory)
{
  registerExecFlag(EXEC_JUST_GO);
}
