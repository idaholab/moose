//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CombinedTestApp.h"
#include "CombinedApp.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "ChemicalReactionsTestApp.h"
#include "ContactTestApp.h"
#include "ElectromagneticsTestApp.h"
#include "ExternalPetscSolverTestApp.h"
#include "FluidPropertiesTestApp.h"
#include "FsiTestApp.h"
#include "FunctionalExpansionToolsTestApp.h"
#include "GeochemistryTestApp.h"
#include "HeatConductionTestApp.h"
#include "LevelSetTestApp.h"
#include "MiscTestApp.h"
#include "NavierStokesTestApp.h"
#include "OptimizationTestApp.h"
#include "PeridynamicsTestApp.h"
#include "PhaseFieldTestApp.h"
#include "PorousFlowTestApp.h"
#include "PorousFlowTestApp.h"
#include "RayTracingTestApp.h"
#include "RdgTestApp.h"
#include "ReactorTestApp.h"
#include "RichardsTestApp.h"
#include "StochasticToolsTestApp.h"
#include "TensorMechanicsTestApp.h"
#include "ThermalHydraulicsTestApp.h"
#include "XFEMTestApp.h"

InputParameters
CombinedTestApp::validParams()
{
  InputParameters params = CombinedApp::validParams();
  return params;
}

registerKnownLabel("CombinedTestApp");

CombinedTestApp::CombinedTestApp(const InputParameters & parameters) : MooseApp(parameters)
{
  CombinedTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

CombinedTestApp::~CombinedTestApp() {}

void
CombinedTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  CombinedApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"CombinedTestApp"});
    Registry::registerActionsTo(af, {"CombinedTestApp"});

    ChemicalReactionsTestApp::registerAll(f, af, s, use_test_objs);
    ContactTestApp::registerAll(f, af, s, use_test_objs);
    ElectromagneticsTestApp::registerAll(f, af, s, use_test_objs);
    ExternalPetscSolverTestApp::registerAll(f, af, s, use_test_objs);
    FluidPropertiesTestApp::registerAll(f, af, s, use_test_objs);
    FsiTestApp::registerAll(f, af, s, use_test_objs);
    FunctionalExpansionToolsTestApp::registerAll(f, af, s, use_test_objs);
    GeochemistryTestApp::registerAll(f, af, s, use_test_objs);
    HeatConductionTestApp::registerAll(f, af, s, use_test_objs);
    LevelSetTestApp::registerAll(f, af, s, use_test_objs);
    MiscTestApp::registerAll(f, af, s, use_test_objs);
    NavierStokesTestApp::registerAll(f, af, s, use_test_objs);
    OptimizationTestApp::registerAll(f, af, s, use_test_objs);
    PeridynamicsTestApp::registerAll(f, af, s, use_test_objs);
    PhaseFieldTestApp::registerAll(f, af, s, use_test_objs);
    PorousFlowTestApp::registerAll(f, af, s, use_test_objs);
    RayTracingTestApp::registerAll(f, af, s, use_test_objs);
    RdgTestApp::registerAll(f, af, s, use_test_objs);
    ReactorTestApp::registerAll(f, af, s, use_test_objs);
    RichardsTestApp::registerAll(f, af, s, use_test_objs);
    StochasticToolsTestApp::registerAll(f, af, s, use_test_objs);
    TensorMechanicsTestApp::registerAll(f, af, s, use_test_objs);
    ThermalHydraulicsTestApp::registerAll(f, af, s, use_test_objs);
    XFEMTestApp::registerAll(f, af, s, use_test_objs);
  }
}

void
CombinedTestApp::registerApps()
{
  registerApp(CombinedApp);
  registerApp(CombinedTestApp);

  // Terrible Hack:
  // Right now we aren't automatically registering dependent apps to build. We
  // need a way to do this so that Multiapp types work automatically. We have a
  // few regression tests in THM that create ThermalHydraulicsApp that fail to work with the
  // combined module. For now, I'm going to manually register ThermalHydraulicsApp. We'll
  // need to design the API so that all registered apps and modules also get
  // immediate access to the buildable apps for use in Multiapps.
  registerApp(ThermalHydraulicsApp);
}

void
CombinedTestApp::registerObjects(Factory & /*factory*/)
{
  mooseError("registerObjects is deprecated, fix the calling application");
}

void
CombinedTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
  mooseError("associateSyntax is deprecated, fix the calling application");
}

void
CombinedTestApp::registerExecFlags(Factory & /*factory*/)
{
  mooseError("registerExecFlags is deprecated, fix the calling application");
}

extern "C" void
CombinedTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  CombinedTestApp::registerAll(f, af, s);
}
extern "C" void
CombinedTestApp__registerApps()
{
  CombinedTestApp::registerApps();
}
