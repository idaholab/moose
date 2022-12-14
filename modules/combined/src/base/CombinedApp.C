//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CombinedApp.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "ChemicalReactionsApp.h"
#include "ContactApp.h"
#include "ElectromagneticsApp.h"
#include "ExternalPetscSolverApp.h"
#include "FluidPropertiesApp.h"
#include "FsiApp.h"
#include "FunctionalExpansionToolsApp.h"
#include "GeochemistryApp.h"
#include "HeatConductionApp.h"
#include "LevelSetApp.h"
#include "MiscApp.h"
#include "NavierStokesApp.h"
#include "OptimizationApp.h"
#include "PeridynamicsApp.h"
#include "PhaseFieldApp.h"
#include "PorousFlowApp.h"
#include "RayTracingApp.h"
#include "RdgApp.h"
#include "ReactorApp.h"
#include "RichardsApp.h"
#include "ScalarTransportApp.h"
#include "SolidPropertiesApp.h"
#include "StochasticToolsApp.h"
#include "TensorMechanicsApp.h"
#include "ThermalHydraulicsApp.h"
#include "XFEMApp.h"

InputParameters
CombinedApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

registerKnownLabel("CombinedApp");

CombinedApp::CombinedApp(const InputParameters & parameters) : MooseApp(parameters)
{
  CombinedApp::registerAll(_factory, _action_factory, _syntax);
}

CombinedApp::~CombinedApp() {}

void
CombinedApp::registerApps()
{
  registerApp(CombinedApp);
}

void
CombinedApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"CombinedApp"});
  Registry::registerActionsTo(af, {"CombinedApp"});

  ChemicalReactionsApp::registerAll(f, af, s);
  ContactApp::registerAll(f, af, s);
  ElectromagneticsApp::registerAll(f, af, s);
  ExternalPetscSolverApp::registerAll(f, af, s);
  FluidPropertiesApp::registerAll(f, af, s);
  FsiApp::registerAll(f, af, s);
  FunctionalExpansionToolsApp::registerAll(f, af, s);
  GeochemistryApp::registerAll(f, af, s);
  HeatConductionApp::registerAll(f, af, s);
  LevelSetApp::registerAll(f, af, s);
  MiscApp::registerAll(f, af, s);
  NavierStokesApp::registerAll(f, af, s);
  OptimizationApp::registerAll(f, af, s);
  PeridynamicsApp::registerAll(f, af, s);
  PhaseFieldApp::registerAll(f, af, s);
  PorousFlowApp::registerAll(f, af, s);
  RayTracingApp::registerAll(f, af, s);
  RdgApp::registerAll(f, af, s);
  ReactorApp::registerAll(f, af, s);
  RichardsApp::registerAll(f, af, s);
  ScalarTransportApp::registerAll(f, af, s);
  SolidPropertiesApp::registerAll(f, af, s);
  StochasticToolsApp::registerAll(f, af, s);
  TensorMechanicsApp::registerAll(f, af, s);
  ThermalHydraulicsApp::registerAll(f, af, s);
  XFEMApp::registerAll(f, af, s);
}

void
CombinedApp::registerObjects(Factory & /*factory*/)
{
  mooseError("registerObjects is deprecated, fix the calling application");
}

void
CombinedApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
  mooseError("associateSyntax is deprecated, fix the calling application");
}

void
CombinedApp::registerExecFlags(Factory & /*factory*/)
{
  mooseError("registerExecFlags is deprecated, fix the calling application");
}

extern "C" void
CombinedApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  CombinedApp::registerAll(f, af, s);
}
extern "C" void
CombinedApp__registerApps()
{
  CombinedApp::registerApps();
}
