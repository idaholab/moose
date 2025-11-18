//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "ShiftedBoundaryMethodTestApp.h"
#include "ShiftedBoundaryMethodApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

InputParameters
ShiftedBoundaryMethodTestApp::validParams()
{
  InputParameters params = ShiftedBoundaryMethodApp::validParams();
  params.set<bool>("use_legacy_material_output") = false;
  return params;
}

ShiftedBoundaryMethodTestApp::ShiftedBoundaryMethodTestApp(const InputParameters & parameters) : MooseApp(parameters)
{
  ShiftedBoundaryMethodTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

ShiftedBoundaryMethodTestApp::~ShiftedBoundaryMethodTestApp() {}

void
ShiftedBoundaryMethodTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  ShiftedBoundaryMethodApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"ShiftedBoundaryMethodTestApp"});
    Registry::registerActionsTo(af, {"ShiftedBoundaryMethodTestApp"});
  }
}

void
ShiftedBoundaryMethodTestApp::registerApps()
{
  registerApp(ShiftedBoundaryMethodApp);
  registerApp(ShiftedBoundaryMethodTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
ShiftedBoundaryMethodTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ShiftedBoundaryMethodTestApp::registerAll(f, af, s);
}
extern "C" void
ShiftedBoundaryMethodTestApp__registerApps()
{
  ShiftedBoundaryMethodTestApp::registerApps();
}
