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
#include "FluidPropertiesTestApp.h"
#include "HeatConductionTestApp.h"
#include "MiscTestApp.h"
#include "NavierStokesTestApp.h"
#include "PhaseFieldTestApp.h"
#include "RichardsTestApp.h"
#include "SolidMechanicsTestApp.h"
#include "StochasticToolsTestApp.h"
#include "TensorMechanicsTestApp.h"
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
    FluidPropertiesTestApp::registerAll(f, af, s, use_test_objs);
    HeatConductionTestApp::registerAll(f, af, s, use_test_objs);
    MiscTestApp::registerAll(f, af, s, use_test_objs);
    NavierStokesTestApp::registerAll(f, af, s, use_test_objs);
    PhaseFieldTestApp::registerAll(f, af, s, use_test_objs);
    RichardsTestApp::registerAll(f, af, s, use_test_objs);
    SolidMechanicsTestApp::registerAll(f, af, s, use_test_objs);
    StochasticToolsTestApp::registerAll(f, af, s, use_test_objs);
    TensorMechanicsTestApp::registerAll(f, af, s, use_test_objs);
    XFEMTestApp::registerAll(f, af, s, use_test_objs);
    PorousFlowTestApp::registerAll(f, af, s, use_test_objs);
    RdgTestApp::registerAll(f, af, s, use_test_objs);
    LevelSetTestApp::registerAll(f, af, s, use_test_objs);
  }
}

void
CombinedTestApp::registerApps()
{
  registerApp(CombinedApp);
  registerApp(CombinedTestApp);
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
  XFEMTestApp::registerObjects(factory);
  PorousFlowTestApp::registerObjects(factory);
  RdgTestApp::registerObjects(factory);
  LevelSetTestApp::registerObjects(factory);
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
  XFEMTestApp::associateSyntax(syntax, action_factory);
  PorousFlowTestApp::associateSyntax(syntax, action_factory);
  RdgTestApp::associateSyntax(syntax, action_factory);
  LevelSetTestApp::associateSyntax(syntax, action_factory);
}

void
CombinedTestApp::registerExecFlags(Factory & factory)
{
  ChemicalReactionsTestApp::registerExecFlags(factory);
  ContactTestApp::registerExecFlags(factory);
  FluidPropertiesTestApp::registerExecFlags(factory);
  HeatConductionTestApp::registerExecFlags(factory);
  MiscTestApp::registerExecFlags(factory);
  NavierStokesTestApp::registerExecFlags(factory);
  PhaseFieldTestApp::registerExecFlags(factory);
  RichardsTestApp::registerExecFlags(factory);
  SolidMechanicsTestApp::registerExecFlags(factory);
  StochasticToolsTestApp::registerExecFlags(factory);
  TensorMechanicsTestApp::registerExecFlags(factory);
  XFEMTestApp::registerExecFlags(factory);
  PorousFlowTestApp::registerExecFlags(factory);
  RdgTestApp::registerExecFlags(factory);
  LevelSetTestApp::registerExecFlags(factory);
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
