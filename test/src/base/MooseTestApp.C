//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

  // MooseTestApp is special because it will have its own
  // binary and we want the default to allow test objects.
  params.suppressParameter<bool>("allow_test_objects");
  params.addCommandLineParam<bool>(
      "disallow_test_objects", "--disallow-test-objects", "Don't register test objects and syntax");
  params.addParamNamesToGroup("disallow_test_objects", "Testing");

  // Flag for testing MooseApp::getRestartableDataMap error message
  params.addParam<bool>(
      "test_getrestartabledatamap_error", false, "Call getRestartableDataMap with a bad name.");

  // Flag for turning how EigenProblem output eigenvalues
  params.addParam<bool>(
      "output_inverse_eigenvalue", false, "True to let EigenProblem output inverse eigenvalue.");

  params.addParam<std::string>("append_header", "", "String to print at top of console output");

  params.addRangeCheckedParam<Real>(
      "output_wall_time_interval",
      "output_wall_time_interval>0",
      "The target wall time interval at which to write to output; for testing");

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  params.set<bool>(MeshGeneratorSystem::allow_data_driven_param) = true;

  return params;
}

MooseTestApp::MooseTestApp(const InputParameters & parameters) : MooseApp(parameters)
{
  MooseTestApp::registerAll(
      _factory, _action_factory, _syntax, !getParam<bool>("disallow_test_objects"));

  if (getParam<bool>("test_getrestartabledatamap_error"))
    getRestartableDataMap("slaughter");
  if (getParam<bool>("disallow_test_objects"))
    const_cast<InputParameters &>(_pars).set<bool>(MeshGeneratorSystem::allow_data_driven_param) =
        false;
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
    // For testing Physics & ActionComponents
    registerSyntax("TestPhysicsComponentInterfaceErrors",
                   "Physics/Test/ComponentInterfaceErrors/*");
  }
}

void
MooseTestApp::registerApps()
{
  registerApp(MooseTestApp);
}

std::string
MooseTestApp::header() const
{
  return getParam<std::string>("append_header");
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
