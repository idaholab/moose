/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CombinedTestApp.h"
#include "CombinedApp.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "ChemicalReactionsTestApp.h"
#include "ContactTestApp.h"
#include "FluidPropertiesTestApp.h"
#include "HeatConductionTestApp.h"
#include "MiscTestApp.h"
#include "NavierStokesTestApp.h"
#include "PhaseFieldTestApp.h"
#include "RichardsTestApp.h"
#include "SolidMechanicsTestApp.h"
#include "StochasticToolsTestApp.h"
#include "TensorMechanicsTestApp.h"
#include "WaterSteamEOSTestApp.h"
#include "XFEMTestApp.h"
#include "PorousFlowTestApp.h"
#include "RdgTestApp.h"
#include "LevelSetTestApp.h"

template <>
InputParameters
validParams<CombinedTestApp>()
{
  InputParameters params = validParams<CombinedApp>();
  return params;
}

CombinedTestApp::CombinedTestApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  CombinedApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  CombinedApp::associateSyntax(_syntax, _action_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    CombinedTestApp::registerObjects(_factory);
    CombinedTestApp::associateSyntax(_syntax, _action_factory);
  }
}

CombinedTestApp::~CombinedTestApp() {}

// External entry point for dynamic application loading
extern "C" void
CombinedTestApp__registerApps()
{
  CombinedTestApp::registerApps();
}
void
CombinedTestApp::registerApps()
{
  registerApp(CombinedApp);
  registerApp(CombinedTestApp);
}

// External entry point for dynamic object registration
extern "C" void
CombinedTestApp__registerObjects(Factory & factory)
{
  CombinedTestApp::registerObjects(factory);
}
void
CombinedTestApp::registerObjects(Factory & factory)
{
  ChemicalReactionsTestApp::registerObjects(factory);
  ContactTestApp::registerObjects(factory);
  FluidPropertiesTestApp::registerObjects(factory);
  HeatConductionTestApp::registerObjects(factory);
  MiscTestApp::registerObjects(factory);
  NavierStokesTestApp::registerObjects(factory);
  PhaseFieldTestApp::registerObjects(factory);
  RichardsTestApp::registerObjects(factory);
  SolidMechanicsTestApp::registerObjects(factory);
  StochasticToolsTestApp::registerObjects(factory);
  TensorMechanicsTestApp::registerObjects(factory);
  WaterSteamEOSTestApp::registerObjects(factory);
  XFEMTestApp::registerObjects(factory);
  PorousFlowTestApp::registerObjects(factory);
  RdgTestApp::registerObjects(factory);
  LevelSetTestApp::registerObjects(factory);
}

// External entry point for dynamic syntax association
extern "C" void
CombinedTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  CombinedTestApp::associateSyntax(syntax, action_factory);
}
void
CombinedTestApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  ChemicalReactionsTestApp::associateSyntax(syntax, action_factory);
  ContactTestApp::associateSyntax(syntax, action_factory);
  FluidPropertiesTestApp::associateSyntax(syntax, action_factory);
  HeatConductionTestApp::associateSyntax(syntax, action_factory);
  MiscTestApp::associateSyntax(syntax, action_factory);
  NavierStokesTestApp::associateSyntax(syntax, action_factory);
  PhaseFieldTestApp::associateSyntax(syntax, action_factory);
  RichardsTestApp::associateSyntax(syntax, action_factory);
  SolidMechanicsTestApp::associateSyntax(syntax, action_factory);
  StochasticToolsTestApp::associateSyntax(syntax, action_factory);
  TensorMechanicsTestApp::associateSyntax(syntax, action_factory);
  WaterSteamEOSTestApp::associateSyntax(syntax, action_factory);
  XFEMTestApp::associateSyntax(syntax, action_factory);
  PorousFlowTestApp::associateSyntax(syntax, action_factory);
  RdgTestApp::associateSyntax(syntax, action_factory);
  LevelSetTestApp::associateSyntax(syntax, action_factory);
}
