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
  MooseTestApp::registerAll(
      _factory, _action_factory, _syntax, !getParam<bool>("disallow_test_objects"));
}

MooseTestApp::~MooseTestApp() {}

void
MooseTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  Registry::registerObjectsTo(f, {"MooseTestApp"});
  Registry::registerActionsTo(af, {"MooseTestApp"});

  if (use_test_objs)
  {
    auto & syntax = s;  // for resiterSyntax macros
    auto & factory = f; // for resiterSyntax macros

    registerExecFlag(EXEC_JUST_GO);

    registerSyntax("ConvDiffMetaAction", "ConvectionDiffusion");
    registerSyntaxTask("AddAuxVariableAction", "MoreAuxVariables/*", "add_aux_variable");
    registerSyntaxTask("AddLotsOfAuxVariablesAction", "LotsOfAuxVariables/*", "add_variable");
    registerSyntax("ApplyCoupledVariablesTestAction", "ApplyInputParametersTest");
    registerSyntax("AddLotsOfDiffusion", "Testing/LotsOfDiffusion/*");
    registerSyntax("TestGetActionsAction", "TestGetActions");
    registerSyntax("BadAddKernelAction", "BadKernels/*");
    registerSyntax("AddMatAndKernel", "AddMatAndKernel");
    registerSyntax("MetaNodalNormalsAction", "MetaNodalNormals");
    // For testing the ability to create problems in user defined Actions
    registerSyntax("CreateSpecialProblemAction", "TestProblem");
  }
}

void
MooseTestApp::registerApps()
{
  registerApp(MooseTestApp);
}

extern "C" void
MooseTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  MooseTestApp::registerAll(f, af, s);
}

// External entry point for dynamic application loading
extern "C" void
MooseTestApp__registerApps()
{
  MooseTestApp::registerApps();
}
