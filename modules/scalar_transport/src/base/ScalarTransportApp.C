//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarTransportApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "ChemicalReactionsApp.h"
#include "NavierStokesApp.h"
#include "ThermalHydraulicsApp.h"
#include "FluidPropertiesApp.h"
#include "HeatTransferApp.h"
#include "RdgApp.h"
#include "RayTracingApp.h"
#include "SolidPropertiesApp.h"
#include "MiscApp.h"

InputParameters
ScalarTransportApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  params.set<bool>("automatic_automatic_scaling") = false;
  return params;
}

ScalarTransportApp::ScalarTransportApp(InputParameters parameters) : MooseApp(parameters)
{
  ScalarTransportApp::registerAll(_factory, _action_factory, _syntax);
}

ScalarTransportApp::~ScalarTransportApp() {}

void
ScalarTransportApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"ScalarTransportApp"});
  Registry::registerActionsTo(af, {"ScalarTransportApp"});

  ChemicalReactionsApp::registerAll(f, af, s);
  NavierStokesApp::registerAll(f, af, s);
  ThermalHydraulicsApp::registerAll(f, af, s);
  FluidPropertiesApp::registerAll(f, af, s);
  HeatTransferApp::registerAll(f, af, s);
  RdgApp::registerAll(f, af, s);
  RayTracingApp::registerAll(f, af, s);
  SolidPropertiesApp::registerAll(f, af, s);
  MiscApp::registerAll(f, af, s);

  /* register custom execute flags, action syntax, etc. here */
  auto & syntax = s;
  registerSyntax("MultiSpeciesDiffusionCG", "Physics/MultiSpeciesDiffusion/ContinuousGalerkin/*");
}

void
ScalarTransportApp::registerApps()
{
  registerApp(ScalarTransportApp);

  ChemicalReactionsApp::registerApps();
  NavierStokesApp::registerApps();
  ThermalHydraulicsApp::registerApps();
  FluidPropertiesApp::registerApps();
  HeatTransferApp::registerApps();
  RdgApp::registerApps();
  RayTracingApp::registerApps();
  SolidPropertiesApp::registerApps();
  MiscApp::registerApps();
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
ScalarTransportApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ScalarTransportApp::registerAll(f, af, s);
}
extern "C" void
ScalarTransportApp__registerApps()
{
  ScalarTransportApp::registerApps();
}
