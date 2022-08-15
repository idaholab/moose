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
#include "EigenProblem.h"

#include "MooseTestApp.h"
#include "MooseRevision.h"

InputParameters
MooseTestApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Flag for testing MooseApp::getRestartableDataMap error message
  params.addCommandLineParam<bool>("test_getRestartableDataMap_error",
                                   "--test_getRestartableDataMap_error",
                                   false,
                                   "Call getRestartableDataMap with a bad name.");

  // Flag for turning how EigenProblem output eigenvalues
  params.addCommandLineParam<bool>("output_inverse_eigenvalue",
                                   "--output-inverse-eigenvalue",
                                   false,
                                   "True to let EigenProblem output inverse eigenvalue.");

  /* MooseTestApp is special because it will have its own
   * binary and we want the default to allow test objects.
   */
  params.suppressParameter<bool>("allow_test_objects");
  params.addCommandLineParam<bool>("disallow_test_objects",
                                   "--disallow-test-objects",
                                   false,
                                   "Don't register test objects and syntax");

  params.addCommandLineParam<bool>(
      "test_check_legacy_params",
      "--test-check-legacy-params",
      false,
      "True to test checking for legacy parameter construction with CheckLegacyParamsAction");

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

MooseTestApp::MooseTestApp(const InputParameters & parameters) : MooseApp(parameters)
{
  MooseTestApp::registerAll(
      _factory, _action_factory, _syntax, !getParam<bool>("disallow_test_objects"));

  if (getParam<bool>("test_getRestartableDataMap_error"))
    getRestartableDataMap("slaughter");
}

MooseTestApp::~MooseTestApp() {}

void
MooseTestApp::executeExecutioner()
{
#ifdef LIBMESH_HAVE_SLEPC
  if (getParam<bool>("output_inverse_eigenvalue"))
  {
    auto eigen_problem = dynamic_cast<EigenProblem *>(&(_executioner->feProblem()));
    if (eigen_problem)
      eigen_problem->outputInverseEigenvalue(true);
  }
#endif

  MooseApp::executeExecutioner();
}

std::string
MooseTestApp::getInstallableInputs() const
{
  return MOOSE_INSTALLABLE_DIRS;
}

void
MooseTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  Registry::registerObjectsTo(f, {"MooseTestApp"});
  Registry::registerActionsTo(af, {"MooseTestApp"});

  if (use_test_objs)
  {
    auto & syntax = s; // for resiterSyntax macros

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
    registerSyntax("AddDGDiffusion", "DGDiffusionAction");
    registerSyntax("MeshMetaDataDependenceAction", "AutoLineSamplerTest");
    registerSyntax("AppendMeshGeneratorAction", "ModifyMesh/*");
    registerSyntax("CheckMeshMetaDataAction", "CheckMeshMetaData");
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
