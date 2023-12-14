//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalHydraulicsTestApp.h"
#include "ThermalHydraulicsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ThermalHydraulicsTestApp::validParams()
{
  InputParameters params = ThermalHydraulicsApp::validParams();
  return params;
}

registerKnownLabel("ThermalHydraulicsTestApp");

ThermalHydraulicsTestApp::ThermalHydraulicsTestApp(InputParameters parameters)
  : ThermalHydraulicsApp(parameters)
{
  ThermalHydraulicsTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

ThermalHydraulicsTestApp::~ThermalHydraulicsTestApp() {}

void
ThermalHydraulicsTestApp::registerAll(Factory & f,
                                      ActionFactory & af,
                                      Syntax & s,
                                      bool use_test_objs)
{
  ThermalHydraulicsApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"ThermalHydraulicsTestApp"});
    Registry::registerActionsTo(af, {"ThermalHydraulicsTestApp"});

    s.registerActionSyntax("JacobianTest1PhaseAction", "JacobianTest1Phase");
    s.registerActionSyntax("JacobianTestGeneralAction", "JacobianTestGeneral");
    s.registerActionSyntax("JacobianTest1PhaseRDGAction", "JacobianTest1PhaseRDG");

    s.registerActionSyntax("ClosureTest1PhaseAction", "ClosureTest1Phase");
  }
}

void
ThermalHydraulicsTestApp::registerApps()
{
  registerApp(ThermalHydraulicsApp);
  registerApp(ThermalHydraulicsTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
ThermalHydraulicsTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ThermalHydraulicsTestApp::registerAll(f, af, s);
}

extern "C" void
ThermalHydraulicsTestApp__registerApps()
{
  ThermalHydraulicsTestApp::registerApps();
}
