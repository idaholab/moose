//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarTransportTestApp.h"
#include "ScalarTransportApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "ChemicalReactionsTestApp.h"
#include "NavierStokesTestApp.h"
#include "ThermalHydraulicsTestApp.h"
#include "FluidPropertiesTestApp.h"
#include "HeatTransferTestApp.h"
#include "RdgTestApp.h"
#include "RayTracingTestApp.h"
#include "SolidPropertiesTestApp.h"
#include "MiscTestApp.h"

InputParameters
ScalarTransportTestApp::validParams()
{
  InputParameters params = ScalarTransportApp::validParams();
  return params;
}

ScalarTransportTestApp::ScalarTransportTestApp(InputParameters parameters)
  : ScalarTransportApp(parameters)
{
  ScalarTransportTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

ScalarTransportTestApp::~ScalarTransportTestApp() {}

void
ScalarTransportTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  ScalarTransportApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"ScalarTransportTestApp"});
    Registry::registerActionsTo(af, {"ScalarTransportTestApp"});
  }
}

void
ScalarTransportTestApp::registerApps()
{
  ScalarTransportApp::registerApps();
  registerApp(ScalarTransportTestApp);

  ChemicalReactionsTestApp::registerApps();
  NavierStokesTestApp::registerApps();
  ThermalHydraulicsTestApp::registerApps();
  FluidPropertiesTestApp::registerApps();
  HeatTransferTestApp::registerApps();
  RdgTestApp::registerApps();
  RayTracingTestApp::registerApps();
  SolidPropertiesTestApp::registerApps();
  MiscTestApp::registerApps();
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
ScalarTransportTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ScalarTransportTestApp::registerAll(f, af, s);
}
extern "C" void
ScalarTransportTestApp__registerApps()
{
  ScalarTransportTestApp::registerApps();
}
