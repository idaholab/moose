//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "FluidStructureInteractionTestApp.h"
#include "FluidStructureInteractionApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
FluidStructureInteractionTestApp::validParams()
{
  InputParameters params = FluidStructureInteractionApp::validParams();
  return params;
}

FluidStructureInteractionTestApp::FluidStructureInteractionTestApp(InputParameters parameters)
  : MooseApp(parameters)
{
  FluidStructureInteractionTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

FluidStructureInteractionTestApp::~FluidStructureInteractionTestApp() {}

void
FluidStructureInteractionTestApp::registerAll(Factory & f,
                                              ActionFactory & af,
                                              Syntax & s,
                                              bool use_test_objs)
{
  FluidStructureInteractionApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"FluidStructureInteractionTestApp"});
    Registry::registerActionsTo(af, {"FluidStructureInteractionTestApp"});
  }
}

void
FluidStructureInteractionTestApp::registerApps()
{
  registerApp(FluidStructureInteractionApp);
  registerApp(FluidStructureInteractionTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
FluidStructureInteractionTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FluidStructureInteractionTestApp::registerAll(f, af, s);
}
extern "C" void
FluidStructureInteractionTestApp__registerApps()
{
  FluidStructureInteractionTestApp::registerApps();
}
